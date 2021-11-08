//
// Created by PikachuHy on 2021/11/5.
//

#include "Editor.h"

#include <QFile>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QStringList>
#include <memory>

#include "Cursor.h"
#include "debug.h"
#include "render/Instruction.h"
#include "render/Render.h"
namespace md::editor {
Editor::Editor() { m_cursor = std::make_shared<Cursor>(); }
void Editor::loadFile(const String& path) {
  QFile file(path);
  if (!file.exists()) {
    DEBUG << "file not exist:" << path;
    return;
  }
  if (!file.open(QIODevice::ReadOnly)) {
    DEBUG << "file open fail:" << path;
    return;
  }
  auto mdText = file.readAll();
  m_doc = std::make_shared<Document>(mdText);
}
void Editor::paintEvent(QPoint offset, Painter& painter) {
  if (!m_doc) return;
  auto oldOffset = offset;
  for (const auto& instructionGroup : m_doc->m_blocks) {
    auto h = instructionGroup.height();
    for (const auto& instructionLine : instructionGroup.visualLines()) {
      for (auto instruction : instructionLine) {
        instruction->run(painter, offset, m_doc.get());
      }
    }
    offset.setY(offset.y() + h);
  }
  auto pos = m_cursor->pos();
  auto coord = m_cursor->coord();
  QStringList list;
  list << QString("Cursor: (%1, %2)").arg(pos.x()).arg(pos.y());
  list << QString("BlockNo: %1").arg(coord.blockNo);
  list << QString("LineNo: %1").arg(coord.lineNo);
  list << QString("Offset: %1").arg(coord.offset);
  int x = 500;
  int y = 400;
  for (auto msg : list) {
    painter.drawText(x, y, msg);
    y += painter.fontMetrics().height() + 4;
  }
  drawCursor(oldOffset, painter);
}
int Editor::width() const { return 800; }
int Editor::height() const {
  auto h = 0;
  for (const auto& instructionGroup : m_doc->m_blocks) {
    h += instructionGroup.height();
  }
  return h;
}
void Editor::drawCursor(QPoint offset, Painter& painter) {
  if (m_showCursor) {
    m_doc->updateCursor(*m_cursor);
    auto pos = m_cursor->pos();
    pos += offset;
    painter.drawLine(pos.x(), pos.y(), pos.x(), pos.y() + m_cursor->height());
  }
  m_showCursor = !m_showCursor;
}
void Editor::keyPressEvent(KeyEvent* event) {
  DEBUG << event;
  if (event->key() == Qt::Key_Left) {
    if (event->modifiers() & Qt::Modifier::CTRL) {
      m_doc->moveCursorToBol(*m_cursor);
    } else {
      m_doc->moveCursorToLeft(*m_cursor);
    }
  } else if (event->key() == Qt::Key_Right) {
    if (event->modifiers() & Qt::Modifier::CTRL) {
      m_doc->moveCursorToEol(*m_cursor);
    } else {
      m_doc->moveCursorToRight(*m_cursor);
    }
  } else if (event->key() == Qt::Key_Up) {
    m_doc->moveCursorToUp(*m_cursor);
  } else if (event->key() == Qt::Key_Down) {
    m_doc->moveCursorToDown(*m_cursor);
  } else if (event->modifiers() & Qt::Modifier::CTRL) {
  } else if (event->key() == Qt::Key_Backspace) {
    m_doc->removeText(*m_cursor);
  } else if (event->key() == Qt::Key_Return) {
    // 处理回车，要拆结点
    DEBUG << "Return";
    m_doc->insertReturn(*m_cursor);
  } else {
    auto text = event->text();
    DEBUG << "insert" << text;
    m_doc->insertText(*m_cursor, text);
  }
}
Point Editor::cursorPos() const { return m_cursor->pos(); }
void Editor::mousePressEvent(MouseEvent* event) {
  DEBUG << event->pos();
  m_doc->moveCursorToPos(*m_cursor, event->pos());
}
void Editor::insertText(String str) { m_doc->insertText(*m_cursor, str); }
}  // namespace md::editor