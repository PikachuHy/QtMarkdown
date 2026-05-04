//
// Created by PikachuHy on 2021/11/5.
//

#include "EditorInputHandler.h"
#include "Cursor.h"
#include "Document.h"
#include "QtAdapters.h"
#include "debug.h"
#include "parser/Text.h"
#include "render/Instruction.h"

#include <QFile>
#include <QColor>
#include <QSize>

using namespace md::parser;

namespace md::editor {

namespace {

class MousePressVisitor : public NodeVisitor {
 public:
  MousePressVisitor(Document &doc, Editor &editor, SizeType blockNo)
      : m_handled(false), m_doc(doc), m_editor(editor), m_blockNo(blockNo) {}
  void visit(Link *node) override {
    if (m_editor.isHoldCtrl()) {
      auto url = node->href()->toString(m_doc.parserDoc());
      m_editor.setHoldCtrl(false);
      m_editor.triggerLinkClicked(url);
      m_handled = true;
    }
  }
  void visit(CheckboxItem *node) override {
    node->setChecked(!node->isChecked());
    m_doc.renderBlock(m_blockNo);
    m_editor.triggerCheckBoxClicked();
    m_handled = true;
  }
  void visit(Image *node) override {
    auto path = node->src()->toString(m_doc.parserDoc());
#if defined (Q_OS_ANDROID) || defined (Q_OS_UNIX)
    if (!path.startsWith("/")) {
      for (const auto& resPath: m_doc.setting().resPathList) {
        QString newImgPath = resPath + "/" + path;
        if (QFile(newImgPath).exists()) {
          path = newImgPath;
        }
      }
    }
#endif
    m_editor.triggerImageClicked(path);
    m_handled = true;
  }
  void visit(CodeBlock *node) override {
    String code;
    for (const auto &child : node->children()) {
      if (child->type() == NodeType::text) {
        auto text = static_cast<Text*>(child.get());
        code += text->toString(m_doc.parserDoc());
        code += "\n";
      } else if (child->type() == NodeType::lf) {
        code += "\n";
      } else {
        DEBUG << node->type();
        ASSERT(false && "not support");
      }
    }
    m_editor.triggerCopyCodeClicked(code);
    m_handled = true;
  }
  [[nodiscard]] bool handled() const { return m_handled; }

 private:
  bool m_handled;
  Document &m_doc;
  Editor &m_editor;
  SizeType m_blockNo;
};

} // anonymous namespace

EditorInputHandler::EditorInputHandler(Editor& editor, Document& doc, Cursor& cursor,
                                       const render::RenderSetting& setting)
    : m_editor(editor), m_doc(doc), m_cursor(cursor), m_setting(setting) {}

void EditorInputHandler::setPreedit(const String& str) {
    if (m_preediting) {
        for (int i = 0; i < m_preeditLength; ++i) {
            m_doc.removeText(m_cursor);
        }
    } else {
        m_preeditPos = m_cursor.pos();
    }
    if (str.isEmpty()) {
        m_preediting = false;
        m_preeditLength = 0;
    } else {
        m_preediting = true;
        m_preeditLength = str.length();
        m_doc.insertText(m_cursor, str);
    }
}

void EditorInputHandler::commitString(const String& str) {
    if (m_preediting) {
        for (int i = 0; i < m_preeditLength; ++i) {
            m_doc.removeText(m_cursor);
        }
    }
    m_doc.insertText(m_cursor, str);
    m_preediting = false;
    m_preeditLength = 0;
}

bool EditorInputHandler::isPreediting() const {
    return m_preediting;
}

core::Point EditorInputHandler::preeditPos() const {
    return m_preeditPos;
}

void EditorInputHandler::keyPressEvent(const core::KeyEvent &event) {
  int key = static_cast<int>(event.key());
  if (key == static_cast<int>(core::Key::Tab)) {
    return;
  }
  if (key == static_cast<int>(core::Key::Escape)) {
    return;
  }
  if ((event.modifiers() & core::Modifier::Ctrl) != core::Modifier::None) {
    m_editor.setHoldCtrl(true);
  }
  if ((event.modifiers() & core::Modifier::Shift) != core::Modifier::None) {
    m_editor.setHoldShift(true);
  }
  if (key == static_cast<int>(core::Key::A) && m_editor.isHoldCtrl()) {
    selectAll();
    return;
  }
  if (key == static_cast<int>(core::Key::Z) && m_editor.isHoldCtrl()) {
    DEBUG << "undo";
    m_doc.undo(m_cursor);
    return;
  }
  if (key == static_cast<int>(core::Key::Y) && m_editor.isHoldCtrl()) {
    DEBUG << "redo";
    m_doc.redo(m_cursor);
    return;
  }
  if (m_editor.isHoldCtrl()) {
    if (key == static_cast<int>(core::Key::Key_1)) {
      m_doc.upgradeToHeader(m_cursor, 1);
      m_doc.updateCursor(m_cursor, m_cursor.coord());
      return;
    }
    if (key == static_cast<int>(core::Key::Key_2)) {
      m_doc.upgradeToHeader(m_cursor, 2);
      m_doc.updateCursor(m_cursor, m_cursor.coord());
      return;
    }
    if (key == static_cast<int>(core::Key::Key_3)) {
      m_doc.upgradeToHeader(m_cursor, 3);
      m_doc.updateCursor(m_cursor, m_cursor.coord());
      return;
    }
    if (key == static_cast<int>(core::Key::Key_4)) {
      m_doc.upgradeToHeader(m_cursor, 4);
      m_doc.updateCursor(m_cursor, m_cursor.coord());
      return;
    }
    if (key == static_cast<int>(core::Key::Key_5)) {
      m_doc.upgradeToHeader(m_cursor, 5);
      m_doc.updateCursor(m_cursor, m_cursor.coord());
      return;
    }
    if (key == static_cast<int>(core::Key::Key_6)) {
      m_doc.upgradeToHeader(m_cursor, 6);
      m_doc.updateCursor(m_cursor, m_cursor.coord());
      return;
    }
  }
  if (key == static_cast<int>(core::Key::Left) || key == static_cast<int>(core::Key::Right) || key == static_cast<int>(core::Key::Up) || key == static_cast<int>(core::Key::Down)) {
    if (m_editor.m_hasSelection && !m_editor.isHoldShift()) {
      auto coord = m_editor.m_selectionRange->caret.coord();
      m_doc.updateCursor(m_cursor, coord);
      m_editor.m_hasSelection = false;
      return;
    }
    if (key == static_cast<int>(core::Key::Left)) {
      if (m_editor.isHoldShift()) {
        if (m_editor.isHoldCtrl()) {
          selectBol();
        } else {
          selectLeft();
        }
      } else {
        if (m_editor.isHoldCtrl()) {
          auto coord = m_doc.moveCursorToBol(m_cursor.coord());
          m_doc.updateCursor(m_cursor, coord, true);
        } else {
          auto coord = m_doc.moveCursorToLeft(m_cursor.coord());
          m_doc.updateCursor(m_cursor, coord);
        }
      }
      return;
    }
    if (key == static_cast<int>(core::Key::Right)) {
      if (m_editor.isHoldShift()) {
        if (m_editor.isHoldCtrl()) {
          selectEol();
        } else {
          selectRight();
        }
      } else {
        if (m_editor.isHoldCtrl()) {
          auto [coord, x] = m_doc.moveCursorToEol(m_cursor.coord());
          m_cursor.setX(x);
          m_doc.updateCursor(m_cursor, coord, false);
        } else {
          auto coord = m_doc.moveCursorToRight(m_cursor.coord());
          m_doc.updateCursor(m_cursor, coord);
        }
      }
      return;
    }
    if (key == static_cast<int>(core::Key::Up)) {
      if (m_editor.isHoldShift()) {
        selectUp();
      } else {
        CursorCoord coord;
        if (m_editor.m_hasSelection) {
          coord = m_editor.m_selectionRange->caret.coord();
          m_editor.m_hasSelection = false;
        } else {
          coord = m_doc.moveCursorToUp(m_cursor.coord(), m_cursor.pos());
        }
        m_doc.updateCursor(m_cursor, coord);
      }
      return;
    }
    if (key == static_cast<int>(core::Key::Down)) {
      if (m_editor.isHoldShift()) {
        selectDown();
      } else {
        CursorCoord coord;
        if (m_editor.m_hasSelection) {
          coord = m_editor.m_selectionRange->caret.coord();
          m_editor.m_hasSelection = false;
        } else {
          coord = m_doc.moveCursorToDown(m_cursor.coord(), m_cursor.pos());
        }
        m_doc.updateCursor(m_cursor, coord);
      }
      return;
    }
  }
  if (key == static_cast<int>(core::Key::Backspace)) {
    if (m_editor.m_hasSelection) {
      removeSelection();
    } else {
      m_doc.removeText(m_cursor);
    }
  } else if (key == static_cast<int>(core::Key::Return)) {
    if (m_editor.m_hasSelection) {
      removeSelection();
    }
    // 处理回车，要拆结点
    m_doc.insertReturn(m_cursor);
  } else {
    if (m_editor.m_hasSelection) {
      removeSelection();
    }
    auto text = QString::fromStdString(event.text());
    m_doc.insertText(m_cursor, text);
  }
}

void EditorInputHandler::keyReleaseEvent(const core::KeyEvent &event) {
  if (event.key() == core::Key::Key_Control) {
    m_editor.setHoldCtrl(false);
  }
  if (event.key() == core::Key::Key_Shift) {
    m_editor.setHoldShift(false);
  }
}

CursorShape EditorInputHandler::cursorShape(const core::Point& offset, const core::Point& pos) {
  auto qOffset = toQPoint(offset);
  auto qPos = toQPoint(pos);
  qOffset.setY(qOffset.y() + m_setting.docMargin.top());
  for (const auto &block : m_doc.blocks()) {
    auto h = block.height();
    for (const auto &element : block.elementList()) {
      if (QRect(element.pos + qOffset, element.size).contains(qPos)) {
        return PointingHandCursor;
      }
    }
    qOffset.setY(qOffset.y() + h + m_setting.blockSpacing);
  }
  return IBeamCursor;
}

void EditorInputHandler::mousePressEvent(const core::Point& offset, const core::MouseEvent &event) {
#ifdef Q_OS_ANDROID
  // 安卓有press，但是没有release
#else
  m_editor.m_mousePressing = true;
#endif
  if (m_editor.isHoldShift()) {
    if (!m_editor.m_hasSelection) {
      m_editor.m_selectionRange = std::make_shared<SelectionRange>();
      m_editor.m_selectionRange->anchor = m_cursor;
      m_editor.m_hasSelection = true;
    }
  } else {
    m_editor.m_hasSelection = false;
  }
  auto qOffset = toQPoint(offset);
  qOffset.setY(qOffset.y() + m_setting.docMargin.top());
  auto mousePos = toQPoint(event.pos());
  for (int blockNo = 0; blockNo < m_doc.blocks().size(); ++blockNo) {
    const auto &block = m_doc.blocks()[blockNo];
    auto h = block.height();
    for (const auto &element : block.elementList()) {
      if (QRect(element.pos + qOffset, element.size).contains(mousePos)) {
        MousePressVisitor visitor(m_doc, m_editor, blockNo);
        element.node->accept(&visitor);
        if (visitor.handled()) {
          return;
        }
      }
    }
    qOffset.setY(qOffset.y() + h + m_setting.blockSpacing);
  }
  auto coord = m_doc.moveCursorToPos(event.pos());
  m_doc.updateCursor(m_cursor, coord);
  if (m_editor.m_hasSelection) {
    m_doc.updateCursor(m_editor.m_selectionRange->caret, coord);
    generateSelectionInstruction();
  }
}

void EditorInputHandler::mouseMoveEvent(const core::Point& offset, const core::MouseEvent &event) {
  if (!m_editor.m_mousePressing) return;
  if (!m_editor.m_hasSelection) {
    m_editor.m_selectionRange = std::make_shared<SelectionRange>();
    m_editor.m_selectionRange->anchor = m_cursor;
    m_editor.m_hasSelection = true;
  }
  auto coord = m_doc.moveCursorToPos(event.pos() + offset);
  m_doc.updateCursor(m_editor.m_selectionRange->caret, coord);
  generateSelectionInstruction();
}

void EditorInputHandler::mouseReleaseEvent(const core::Point& offset, const core::MouseEvent &event) {
  (void)offset;
  (void)event;
  m_editor.m_mousePressing = false;
}

void EditorInputHandler::selectLeft() {
  CursorCoord coord;
  if (m_editor.m_hasSelection) {
    coord = m_editor.m_selectionRange->caret.coord();
  } else {
    m_editor.m_selectionRange = std::make_shared<SelectionRange>();
    coord = m_cursor.coord();
    m_editor.m_selectionRange->anchor = m_cursor;
    m_editor.m_hasSelection = true;
  }
  coord = m_doc.moveCursorToLeft(coord);
  m_doc.updateCursor(m_editor.m_selectionRange->caret, coord);
  generateSelectionInstruction();
}

void EditorInputHandler::selectRight() {
  CursorCoord coord;
  if (m_editor.m_hasSelection) {
    coord = m_editor.m_selectionRange->caret.coord();
  } else {
    m_editor.m_selectionRange = std::make_shared<SelectionRange>();
    coord = m_cursor.coord();
    m_editor.m_selectionRange->anchor = m_cursor;
    m_editor.m_hasSelection = true;
  }
  coord = m_doc.moveCursorToRight(coord);
  m_doc.updateCursor(m_editor.m_selectionRange->caret, coord);
  generateSelectionInstruction();
}

void EditorInputHandler::selectUp() {
  CursorCoord coord;
  core::Point pos;
  if (m_editor.m_hasSelection) {
    coord = m_editor.m_selectionRange->caret.coord();
    pos = m_editor.m_selectionRange->caret.pos();
  } else {
    m_editor.m_selectionRange = std::make_shared<SelectionRange>();
    coord = m_cursor.coord();
    pos = m_cursor.pos();
    m_editor.m_selectionRange->anchor = m_cursor;
    m_editor.m_hasSelection = true;
  }
  coord = m_doc.moveCursorToUp(coord, pos);
  m_doc.updateCursor(m_editor.m_selectionRange->caret, coord);
  generateSelectionInstruction();
}

void EditorInputHandler::selectDown() {
  CursorCoord coord;
  core::Point pos;
  if (m_editor.m_hasSelection) {
    coord = m_editor.m_selectionRange->caret.coord();
    pos = m_editor.m_selectionRange->caret.pos();
  } else {
    m_editor.m_selectionRange = std::make_shared<SelectionRange>();
    coord = m_cursor.coord();
    pos = m_cursor.pos();
    m_editor.m_selectionRange->anchor = m_cursor;
    m_editor.m_hasSelection = true;
  }
  // 如果是视觉行开头，则移动到视觉行结尾
  if (m_doc.isBol(coord)) {
    auto [_coord, x] = m_doc.moveCursorToEol(coord);
    m_editor.m_selectionRange->caret.setX(x);
    m_doc.updateCursor(m_editor.m_selectionRange->caret, _coord, false);
  } else {
    coord = m_doc.moveCursorToDown(coord, pos);
    m_doc.updateCursor(m_editor.m_selectionRange->caret, coord);
  }
  generateSelectionInstruction();
}

void EditorInputHandler::selectBol() {
  CursorCoord coord;
  if (m_editor.m_hasSelection) {
    coord = m_editor.m_selectionRange->caret.coord();
  } else {
    m_editor.m_selectionRange = std::make_shared<SelectionRange>();
    coord = m_cursor.coord();
    m_editor.m_selectionRange->anchor = m_cursor;
    m_editor.m_hasSelection = true;
  }
  coord = m_doc.moveCursorToBol(coord);
  m_doc.updateCursor(m_editor.m_selectionRange->caret, coord);
  generateSelectionInstruction();
}

void EditorInputHandler::selectEol() {
  CursorCoord coord;
  if (m_editor.m_hasSelection) {
    coord = m_editor.m_selectionRange->caret.coord();
  } else {
    m_editor.m_selectionRange = std::make_shared<SelectionRange>();
    coord = m_cursor.coord();
    m_editor.m_selectionRange->anchor = m_cursor;
    m_editor.m_hasSelection = true;
  }
  auto [_coord, x] = m_doc.moveCursorToEol(coord);
  m_editor.m_selectionRange->caret.setX(x);
  m_doc.updateCursor(m_editor.m_selectionRange->caret, _coord, false);
  generateSelectionInstruction();
}

void EditorInputHandler::selectAll() {
  CursorCoord coord;
  m_editor.m_hasSelection = true;
  m_editor.m_selectionRange = std::make_shared<SelectionRange>();
  coord = m_doc.moveCursorToBeginOfDocument();
  m_doc.updateCursor(m_editor.m_selectionRange->anchor, coord);
  coord = m_doc.moveCursorToEndOfDocument();
  m_doc.updateCursor(m_editor.m_selectionRange->caret, coord);
  generateSelectionInstruction();
}

void EditorInputHandler::removeSelection() {
  if (!m_editor.m_hasSelection) return;
  auto [begin, end] = m_editor.m_selectionRange->range();
  m_doc.removeTextRange(begin.coord(), end.coord());
  m_editor.m_hasSelection = false;
  m_doc.updateCursor(m_cursor, begin.coord());
}

void EditorInputHandler::generateSelectionInstruction() {
  m_editor.m_selectionInstructions.clear();
  auto fillRect = [this, &selIns = m_editor.m_selectionInstructions](QPoint pos, int w, int h) {
    QColor bg(187, 214, 251);
    auto instruction = std::make_unique<render::FillRectInstruction>(pos, QSize(w, h), bg);
    selIns.push_back(std::move(instruction));
  };
  auto isBefore = [this](const CursorCoord &coord, SizeType blockNo, SizeType lineNo, SizeType visualLineNo) {
    if (blockNo < coord.blockNo) return true;
    if (blockNo > coord.blockNo) return false;
    if (lineNo < coord.lineNo) return true;
    if (lineNo > coord.lineNo) return false;
    const auto &line = m_doc.blocks()[blockNo].logicalLineAt(lineNo);
    auto coordVisualLineNo = line.visualLineAt(coord.offset, m_doc.parserDoc());
    if (coordVisualLineNo > visualLineNo) return true;
    return false;
  };
  auto isSameVisualLine = [this](const CursorCoord &coord, SizeType blockNo, SizeType lineNo, SizeType visualLineNo) {
    if (blockNo < coord.blockNo) return false;
    if (blockNo > coord.blockNo) return false;
    if (lineNo < coord.lineNo) return false;
    if (lineNo > coord.lineNo) return false;
    const auto &line = m_doc.blocks()[blockNo].logicalLineAt(lineNo);
    auto coordVisualLineNo = line.visualLineAt(coord.offset, m_doc.parserDoc());
    if (coordVisualLineNo < visualLineNo) return false;
    if (coordVisualLineNo > visualLineNo) return false;
    return true;
  };
  auto selectionRange = m_editor.m_selectionRange->range();
  auto [begin, end] = selectionRange;
  auto totalH = m_setting.docMargin.top();
  for (int i = 0; i < begin.coord().blockNo; ++i) {
    const auto &block = m_doc.blocks()[i];
    totalH += block.height();
  }
  bool drawDone = false;
  for (auto blockNo = begin.coord().blockNo; blockNo <= end.coord().blockNo; ++blockNo) {
    const auto &block = m_doc.blocks()[blockNo];
    auto blockOffset = QPoint(0, totalH);
    for (int lineNo = 0; lineNo < block.countOfLogicalLine(); ++lineNo) {
      const auto &line = block.logicalLineAt(lineNo);
      for (int visualLineNo = 0; visualLineNo < line.countOfVisualLine(); ++visualLineNo) {
        const auto &visualLine = line.visualLineAt(visualLineNo);
        if (isBefore(begin.coord(), blockNo, lineNo, visualLineNo)) {
          continue;
        }
        auto fixW = [&visualLine, &selectionRange](int &w) {
          if (w == 0) {
            if (selectionRange.first.coord() == selectionRange.second.coord()) {
              // 不用管的确是0
            } else {
              // 整行都要画，因为end被调到另外一行到开头
              w = visualLine.width();
            }
          }
        };
        auto h = visualLine.height();
        if (isSameVisualLine(begin.coord(), blockNo, lineNo, visualLineNo)) {
          auto pos = QPoint(begin.x(), 0) + QPoint(0, visualLine.pos().y()) + blockOffset;
          if (isSameVisualLine(end.coord(), blockNo, lineNo, visualLineNo)) {
            // 同一个视觉行，从begin画到end
            auto w = end.pos().x - begin.pos().x;
            fixW(w);
            fillRect(pos, w, h);
            drawDone = true;
            break;
          } else {
            // 从begin画到视觉行结束
            auto w = visualLine.width() + visualLine.pos().x() - begin.pos().x;
            fixW(w);
            fillRect(pos, w, h);
            continue;
          }
        }
        if (isSameVisualLine(end.coord(), blockNo, lineNo, visualLineNo)) {
          auto pos = visualLine.pos() + blockOffset;
          auto w = end.pos().x - visualLine.pos().x();
          fixW(w);
          fillRect(pos, w, h);
          drawDone = true;
          break;
        }
        fillRect(visualLine.pos() + blockOffset, visualLine.width(), h);
      }
      if (drawDone) break;
    }
    totalH += block.height();
    if (drawDone) break;
  }
}

} // namespace md::editor
