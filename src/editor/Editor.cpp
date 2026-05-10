//
// Created by PikachuHy on 2021/11/5.
//

#include "Editor.h"
#include "EditorRenderer.h"
#include "EditorInputHandler.h"
#include "FileManager.h"
#include "platform/qt/QtImageProvider.h"

#include <format>
#include <memory>
#include <vector>

#include "Cursor.h"
#include "debug.h"
#include "render/Instruction.h"
#include "render/Render.h"
using namespace md::parser;
namespace md::editor {
Editor::Editor() {
  m_cursor = std::make_shared<Cursor>();
  m_imageProvider = std::make_shared<QtImageProvider>();
  m_renderSetting = std::make_shared<render::RenderSetting>();
#ifdef __ANDROID__
  m_renderSetting->docMargin.left = 0;
#endif
  m_linkClickedCallback = [](String s) { DEBUG << "click link" << s; };
  m_imageClickedCallback = [](String s) { DEBUG << "click image" << s; };
  m_copyCodeBtnClickedCallback = [](String s) { DEBUG << "click copy code btn" << s; };
  m_checkBoxClickedCallback = []() { DEBUG << "click check box"; };
}
Editor::~Editor() = default;
void Editor::loadText(const String &text) {
  m_doc = std::make_shared<Document>(text, m_renderSetting, m_imageProvider.get());
  m_cursor = std::make_shared<Cursor>();
  m_renderer = std::make_unique<EditorRenderer>(*m_doc, *m_renderSetting);
  m_inputHandler = std::make_unique<EditorInputHandler>(*this, *m_doc, *m_cursor, *m_renderSetting);
  m_doc->updateCursor(*m_cursor, m_cursor->coord());
  DEBUG << "load text done";
}
std::pair<bool, String> Editor::loadFile(const String &path) {
  auto [ok, mdText] = FileManager::loadFile(path);
  if (!ok) return {false, ""};
  loadText(mdText);
  return {true, this->title()};
}

bool Editor::saveToFile(const String &path) {
  if (!m_doc) return false;
  FileManager fm(*m_doc);
  return fm.saveToFile(path);
}
void Editor::drawSelection(core::AbstractPainter& painter,
                           const core::Point& offset) {
  if (!m_renderer || !m_hasSelection) return;
  m_renderer->drawSelection(painter, offset,
                            m_selectionInstructions, *m_doc);
}
void Editor::drawDoc(core::AbstractPainter& painter,
                     const core::Point& offset) {
  if (!m_renderer) return;
  m_renderer->drawDoc(painter, offset);
#ifndef Q_OS_ANDROID
  auto coord = m_cursor->coord();
  if (coord.blockNo < 0 || coord.blockNo >= static_cast<SizeType>(m_doc->blocks().size())) return;
  // 高亮当前Block
  int h = m_renderSetting->docMargin.top;
  for (int i = 0; i < coord.blockNo; ++i) {
    h += m_renderSetting->blockSpacing;
    h += m_doc->blocks()[i].height();
  }
  const auto& block = m_doc->blocks()[coord.blockNo];
  painter.save();
  painter.setPen(core::Color(0, 255, 255));
  auto highlightPos = core::Point(m_renderSetting->docMargin.left, h);
  auto highlightSize = core::Size(block.width(), block.height() - m_renderSetting->lineSpacing);
  painter.drawRect(core::Rect(highlightPos + offset, highlightSize));
  painter.restore();
  auto& node = m_doc->root()->children()[coord.blockNo];
  auto typePos = core::Point(0, h) + offset;
  if (node->type() == parser::NodeType::paragraph) {
    painter.drawText(typePos, "P");
  } else if (node->type() == parser::NodeType::header) {
    painter.drawText(typePos, "H");
  } else if (node->type() == parser::NodeType::ol) {
    painter.drawText(typePos, "ol");
  } else if (node->type() == parser::NodeType::ul) {
    painter.drawText(typePos, "ul");
  } else if (node->type() == parser::NodeType::checkbox) {
    painter.drawText(typePos, "cb");
  } else if (node->type() == parser::NodeType::latex_block) {
    painter.drawText(typePos, "latex");
  }
#endif
}
int Editor::width() const { return m_renderSetting->maxWidth; }
int Editor::height() const {
  if (!m_renderer) return 0;
  return m_renderer->documentHeight();
}
void Editor::drawCursor(core::AbstractPainter& painter, const core::Point& offset) {
  if (!m_renderer) return;
  m_renderer->drawCursor(painter, offset, *m_cursor, m_hasSelection);
}
void Editor::keyPressEvent(const core::KeyEvent& event) {
  if (!m_inputHandler) return;
  m_inputHandler->keyPressEvent(event);
}
core::Point Editor::cursorPos() const {
  if (!m_inputHandler) return {};
  return m_inputHandler->isPreediting() ? m_inputHandler->preeditPos() : m_cursor->pos();
}
core::Rect Editor::cursorRect() const {
  auto pos = cursorPos();
  auto h = m_cursor->height();
  return core::Rect(pos, core::Size(5, h));
}
void Editor::mousePressEvent(const core::Point& offset, const core::MouseEvent& event) {
  if (!m_inputHandler) return;
  m_inputHandler->mousePressEvent(offset, event);
}
void Editor::insertText(String str) {
  if (str.isEmpty()) return;
  auto strs = str.split('\n');
  for (int i = 0; i < static_cast<int>(strs.size()) - 1; ++i) {
    m_doc->insertText(*m_cursor, strs[i]);
    m_doc->insertReturn(*m_cursor);
  }
  m_doc->insertText(*m_cursor, strs.back());
}
void Editor::reset() {
  m_cursor = std::make_shared<Cursor>();
  m_doc = std::make_shared<Document>("", m_renderSetting, m_imageProvider.get());
  m_renderer = std::make_unique<EditorRenderer>(*m_doc, *m_renderSetting);
  m_inputHandler = std::make_unique<EditorInputHandler>(*this, *m_doc, *m_cursor, *m_renderSetting);
}
String Editor::cursorCoord() const {
  String s;
  auto pos = m_cursor->pos();
  auto coord = m_cursor->coord();
  s += std::format("Cursor: ({}, {}, {})", pos.x, pos.y, m_cursor->height());
  s += "\n";
  s += std::format("BlockNo: {}/{}", coord.blockNo, m_doc->blocks().size());
  s += "\n";
  const auto &block = m_doc->blocks()[coord.blockNo];
  s += std::format("LineNo: {}/{}", coord.lineNo, block.countOfLogicalLine());
  s += "\n";
  s += std::format("Offset: {}/{}", coord.offset, block.logicalLineAt(coord.lineNo).length());
  s += "\n";
  s += "Hold Ctrl: ";
  if (m_holdCtrl)
    s += "YES";
  else
    s += "NO";
  s += "\n";
  s += "Hold Shift: ";
  if (m_holdShift)
    s += "YES";
  else
    s += "NO";
  s += "\n";
  s += "Pre Editing: ";
  if (m_inputHandler && m_inputHandler->isPreediting())
    s += "YES";
  else
    s += "NO";
  s += "\n";
  if (m_hasSelection) {
    s += "Selection Range: ";
    auto [begin, end] = m_selectionRange->range();
    s += std::format("({},{},{})", begin.coord().blockNo, begin.coord().lineNo, begin.coord().offset);
    s += "->";
    s += std::format("({},{},{})", end.coord().blockNo, end.coord().lineNo, end.coord().offset);
  }
  return s;
}

void Editor::setWidth(int w) { m_renderSetting->maxWidth = w; }
void Editor::setResPathList(StringList pathList) { m_renderSetting->resPathList = pathList; }

void Editor::renderDocument() {
  if (m_doc) {
    m_doc->renderAllBlock();
  }
}
CursorShape Editor::cursorShape(const core::Point& offset, const core::Point& pos) {
  if (!m_inputHandler) return IBeamCursor;
  return m_inputHandler->cursorShape(offset, pos);
}
void Editor::keyReleaseEvent(const core::KeyEvent& event) {
  if (!m_inputHandler) return;
  m_inputHandler->keyReleaseEvent(event);
}
void Editor::setPreedit(const String& str) {
  if (!m_inputHandler) return;
  m_inputHandler->setPreedit(str);
}
void Editor::commitString(const String& str) {
  if (!m_inputHandler) return;
  m_inputHandler->commitString(str);
}
String Editor::title() {
  if (!m_doc) return "";
  FileManager fm(*m_doc);
  return fm.title();
}
void Editor::mouseMoveEvent(const core::Point& offset, const core::MouseEvent& event) {
  if (!m_inputHandler) return;
  m_inputHandler->mouseMoveEvent(offset, event);
}
void Editor::mouseReleaseEvent(const core::Point& offset, const core::MouseEvent& event) {
  if (!m_inputHandler) return;
  m_inputHandler->mouseReleaseEvent(offset, event);
}
void Editor::setHoldCtrl(bool v) { m_holdCtrl = v; }
void Editor::setHoldShift(bool v) { m_holdShift = v; }
void Editor::triggerLinkClicked(const String& url) {
  if (m_linkClickedCallback) m_linkClickedCallback(url);
}
void Editor::triggerImageClicked(const String& path) {
  if (m_imageClickedCallback) m_imageClickedCallback(path);
}
void Editor::triggerCopyCodeClicked(const String& code) {
  if (m_copyCodeBtnClickedCallback) m_copyCodeBtnClickedCallback(code);
}
void Editor::triggerCheckBoxClicked() {
  if (m_checkBoxClickedCallback) m_checkBoxClickedCallback();
}
}  // namespace md::editor
