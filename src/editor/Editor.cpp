//
// Created by PikachuHy on 2021/11/5.
//

#include "Editor.h"
#include "EditorRenderer.h"
#include "EditorInputHandler.h"

#include <QFile>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStringList>
#include <memory>
#include <vector>

#include "Cursor.h"
#include "debug.h"
#include "parser/Text.h"
#include "render/Instruction.h"
#include "render/Render.h"
using namespace md::parser;
namespace md::editor {
class SimpleMarkdownVisitor
    : public NodeVisitor {
 public:
  explicit SimpleMarkdownVisitor(DocPtr doc) : m_doc(doc) {}
  void visit(Header *node) override {
    for (int i = 0; i < node->level(); ++i) {
      m_md += "#";
    }
    m_md += " ";
    for (auto& it : node->children()) {
      it->accept(this);
    }
    m_md += "\n";
    m_md += "\n";
  }
  void visit(Text *node) override { m_md += node->toString(m_doc); }
  void visit(ItalicText *node) override {
    m_md += "*";
    node->text()->accept(this);
    m_md += "*";
  }
  void visit(BoldText *node) override {
    m_md += "**";
    node->text()->accept(this);
    m_md += "**";
  }
  void visit(ItalicBoldText *node) override {
    m_md += "***";
    node->text()->accept(this);
    m_md += "***";
  }
  void visit(StrickoutText *node) override {
    m_md += "~~";
    node->text()->accept(this);
    m_md += "~~";
  }
  void visit(Image *node) override {
    m_md += "![";
    if (node->alt()) {
      node->alt()->accept(this);
    } else {
      qDebug() << "image alt is null";
    }
    m_md += "]";
    m_md += "(";
    if (node->src()) {
      node->src()->accept(this);
    } else {
      qDebug() << "image src is null";
    }
    m_md += ")";
  }
  void visit(Link *node) override {
    m_md += "[";
    if (node->href()) {
      node->href()->accept(this);
    } else {
      qDebug() << "link href is null";
    }
    m_md += "]";
    m_md += "(";
    if (node->content()) {
      node->content()->accept(this);
    } else {
      qDebug() << "link content is null";
    }
    m_md += ")";
  }
  void visit(CodeBlock *node) override {
    m_md += "```";
    node->name()->accept(this);
    m_md += "\n";
    for (auto& child : node->children()) {
      child->accept(this);
      m_md += "\n";
    }
    m_md += "```";
    m_md += "\n";
    m_md += "\n";
  }
  void visit(InlineCode *node) override {
    m_md += "`";
    if (auto code = node->code(); code) {
      code->accept(this);
    }
    m_md += "`";
  }
  void visit(Paragraph *node) override {
    if (node->children().empty()) return;
    for (auto& it : node->children()) {
      it->accept(this);
    }
    m_md += "\n";
    m_md += "\n";
  }
  void visit(CheckboxList *node) override {
    for (auto& child : node->children()) {
      ASSERT(child->type() == NodeType::checkbox_item);
      auto item = static_cast<CheckboxItem*>(child.get());
      m_md += "- [";
      if (item->isChecked()) {
        m_md += "x";
      } else {
        m_md += " ";
      }
      m_md += "] ";
      item->accept(this);
      m_md += "\n";
    }
  }
  void visit(CheckboxItem *node) override {
    for (auto& it : node->children()) {
      it->accept(this);
    }
  }
  void visit(UnorderedList *node) override {
    for (auto& it : node->children()) {
      m_md += "- ";
      it->accept(this);
      m_md += "\n";
    }
  }
  void visit(OrderedList *node) override {
    int i = 1;
    for (auto& it : node->children()) {
      m_md += QString("%1. ").arg(i);
      it->accept(this);
      m_md += "\n";
      i++;
    }
  }
  void visit(OrderedListItem *node) override {
    for (auto& child : node->children()) {
      child->accept(this);
    }
  }
  void visit(UnorderedListItem *node) override {
    for (auto& child : node->children()) {
      child->accept(this);
    }
  }
  void visit(Hr *node) override { m_md += "---\n"; }
  void visit(Lf *node) override { m_md += "\n"; }
  void visit(QuoteBlock *node) override {
    m_md += "> ";
    for (auto& it : node->children()) {
      it->accept(this);
      m_md += "\n";
    }
    m_md += "\n";
  }
  void visit(Table *node) override {
    auto renderRow = [this](const StringList& cells) {
      m_md += "|";
      for (const auto& cell : cells) {
        m_md += " " + cell + " |";
      }
      m_md += "\n";
    };
    if (node->header().isEmpty() && node->content().isEmpty()) return;
    renderRow(node->header());
    m_md += "|";
    for (int i = 0; i < node->header().size(); ++i) {
      m_md += " --- |";
    }
    m_md += "\n";
    for (const auto& row : node->content()) {
      renderRow(row);
    }
    m_md += "\n";
  }
  void visit(LatexBlock *node) override {
    m_md += "\n";
    m_md += "$$\n";
    for (auto& it : node->children()) {
      it->accept(this);
    }
    m_md += "$$\n";
    m_md += "\n";
  }
  void visit(InlineLatex *node) override {
    m_md += "$";
    node->code()->accept(this);
    m_md += "$";
  }
  String markdown() { return m_md; }

 private:
  String m_md;
  DocPtr m_doc;
};
Editor::Editor() {
  m_cursor = std::make_shared<Cursor>();
  m_renderSetting = std::make_shared<render::RenderSetting>();
#ifdef Q_OS_ANDROID
  m_renderSetting->docMargin.setLeft(0);
#endif
  m_linkClickedCallback = [](String s) { DEBUG << "click link" << s; };
  m_imageClickedCallback = [](String s) { DEBUG << "click image" << s; };
  m_copyCodeBtnClickedCallback = [](String s) { DEBUG << "click copy code btn" << s; };
  m_checkBoxClickedCallback = []() { DEBUG << "click check box"; };
}
Editor::~Editor() = default;
void Editor::loadText(const String &text) {
  m_doc = std::make_shared<Document>(text, m_renderSetting);
  m_cursor = std::make_shared<Cursor>();
  m_renderer = std::make_unique<EditorRenderer>(*m_doc, *m_renderSetting);
  m_inputHandler = std::make_unique<EditorInputHandler>(*this, *m_doc, *m_cursor, *m_renderSetting);
  m_doc->updateCursor(*m_cursor, m_cursor->coord());
  DEBUG << "load text done";
}
std::pair<bool, String> Editor::loadFile(const String &path) {
  DEBUG << path;
  String notePath = path;
  String prefix = "file://";
  if (path.startsWith(prefix)) {
    notePath = path.mid(prefix.size());
  }
  QFile file(notePath);
  if (!file.exists()) {
    DEBUG << "file not exist:" << notePath;
    return {false, ""};
  }
  if (!file.open(QIODevice::ReadOnly)) {
    DEBUG << "file open fail:" << notePath;
    return {false, ""};
  }
  auto mdText = file.readAll();
  loadText(mdText);
  DEBUG << "load" << path << "done!!!";
  return {true, this->title()};
}

bool Editor::saveToFile(const String &path) {
  String notePath = path;
  if (!notePath.endsWith(".md")) {
    notePath += ".md";
  }
  DEBUG << "note path" << notePath;
  QFile file(notePath);
  if (!file.open(QIODevice::WriteOnly)) {
    DEBUG << "file open fail:" << notePath;
    return false;
  }
  SimpleMarkdownVisitor visitor(m_doc->parserDoc());
  m_doc->accept(&visitor);
  auto mdText = visitor.markdown();
  file.write(mdText.toUtf8());
  file.close();
  return true;
}
void Editor::drawSelection(core::AbstractPainter& painter, QPainter* nativePainter,
                           const core::Point& offset) {
  if (!m_renderer) return;
  m_renderer->drawSelection(painter, nativePainter, offset,
                            m_selectionInstructions, *m_doc);
}
void Editor::drawDoc(core::AbstractPainter& painter, QPainter* nativePainter,
                     const core::Point& offset) {
  if (!m_renderer) return;
  m_renderer->drawDoc(painter, nativePainter, offset);
#ifndef Q_OS_ANDROID
  auto coord = m_cursor->coord();
  if (coord.blockNo < 0 || coord.blockNo >= static_cast<SizeType>(m_doc->blocks().size())) return;
  // 高亮当前Block
  int h = m_renderSetting->docMargin.top();
  for (int i = 0; i < coord.blockNo; ++i) {
    h += m_renderSetting->blockSpacing;
    h += m_doc->blocks()[i].height();
  }
  const auto& block = m_doc->blocks()[coord.blockNo];
  painter.save();
  painter.setPen(core::Color(0, 255, 255));
  auto highlightPos = core::Point(m_renderSetting->docMargin.left(), h);
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
core::Point Editor::cursorPos() const { return m_preediting ? m_preeditPos : m_cursor->pos(); }
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
  auto strs = str.split("\n");
  for (int i = 0; i < strs.size() - 1; ++i) {
    m_doc->insertText(*m_cursor, strs[i]);
    m_doc->insertReturn(*m_cursor);
  }
  m_doc->insertText(*m_cursor, strs.back());
}
void Editor::reset() {
  m_cursor = std::make_shared<Cursor>();
  m_doc = std::make_shared<Document>("", m_renderSetting);
  m_renderer = std::make_unique<EditorRenderer>(*m_doc, *m_renderSetting);
  m_inputHandler = std::make_unique<EditorInputHandler>(*this, *m_doc, *m_cursor, *m_renderSetting);
}
String Editor::cursorCoord() const {
  String s;
  auto pos = m_cursor->pos();
  auto coord = m_cursor->coord();
  s += QString("Cursor: (%1, %2, %3)").arg(pos.x).arg(pos.y).arg(m_cursor->height());
  s += "\n";
  s += QString("BlockNo: %1/%2").arg(coord.blockNo).arg(m_doc->blocks().size());
  s += "\n";
  const auto &block = m_doc->blocks()[coord.blockNo];
  s += QString("LineNo: %1/%2").arg(coord.lineNo).arg(block.countOfLogicalLine());
  s += "\n";
  s += QString("Offset: %1/%2").arg(coord.offset).arg(block.logicalLineAt(coord.lineNo).length());
  s += "\n";
  s += QString("Hold Ctrl: ");
  if (m_holdCtrl)
    s += "YES";
  else
    s += "NO";
  s += "\n";
  s += QString("Hold Shift: ");
  if (m_holdShift)
    s += "YES";
  else
    s += "NO";
  s += "\n";
  s += QString("Pre Editing: ");
  if (m_preediting)
    s += "YES";
  else
    s += "NO";
  s += "\n";
  if (m_hasSelection) {
    s += QString("Selection Range: ");
    auto [begin, end] = m_selectionRange->range();
    s += QString("(%1,%2,%3)").arg(begin.coord().blockNo).arg(begin.coord().lineNo).arg(begin.coord().offset);
    s += QString("->");
    s += QString("(%1,%2,%3)").arg(end.coord().blockNo).arg(end.coord().lineNo).arg(end.coord().offset);
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
void Editor::setPreedit(String str) {
  if (m_preediting) {
    for (int i = 0; i < m_preeditLength; ++i) {
      m_doc->removeText(*m_cursor);
    }
  } else {
    m_preeditPos = m_cursor->pos();
  }
  if (str.isEmpty()) {
    m_preediting = false;
    m_preeditLength = 0;
  } else {
    m_preediting = true;
    m_preeditLength = str.length();
    m_doc->insertText(*m_cursor, str);
  }
}
void Editor::commitString(String str) {
  if (m_preediting) {
    for (int i = 0; i < m_preeditLength; ++i) {
      m_doc->removeText(*m_cursor);
    }
  }
  m_doc->insertText(*m_cursor, str);
  m_preediting = false;
  m_preeditLength = 0;
}
String Editor::title() {
  if (m_doc->root()->empty()) return "";
  auto node = m_doc->root()->childAt(0);
  if (node->type() != NodeType::header) return "";
  auto header = static_cast<Header*>(node);
  if (header->level() != 1) return "";
  String s;
  for (auto& child : header->children()) {
    if (child->type() != NodeType::text) continue;
    auto textNode = static_cast<Text*>(child.get());
    s += textNode->toString(m_doc->parserDoc());
  }
  return s;
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
