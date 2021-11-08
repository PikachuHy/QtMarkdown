//
// Created by PikachuHy on 2021/11/6.
//

#include "QtQuickMarkdownEditor.h"

#include <QCursor>

#include "debug.h"
namespace md::editor {
QtQuickMarkdownEditor::QtQuickMarkdownEditor(QQuickItem *parent) : QQuickPaintedItem(parent) {
  m_editor = std::make_shared<Editor>();
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setFlag(ItemAcceptsInputMethod, true);
  m_cursorTimer.start(500);
  connect(&m_cursorTimer, &QTimer::timeout, [this]() { this->update(); });
}
void QtQuickMarkdownEditor::paint(QPainter *painter) {
  Q_ASSERT(painter != nullptr);
  m_editor->paintEvent(QPoint(0, 0), *painter);
}
void QtQuickMarkdownEditor::setText(const QString &text) {}
void QtQuickMarkdownEditor::setSource(const QString &source) {
  m_editor->loadFile(source);
  setImplicitWidth(m_editor->width());
  setImplicitHeight(m_editor->height());
}
void QtQuickMarkdownEditor::setPath(const QString &path) {}
void QtQuickMarkdownEditor::keyPressEvent(QKeyEvent *event) {
  DEBUG << event;
  m_editor->keyPressEvent(event);
  this->update();
}
void QtQuickMarkdownEditor::focusOutEvent(QFocusEvent *event) { forceActiveFocus(); }
void QtQuickMarkdownEditor::hoverMoveEvent(QHoverEvent *event) { setCursor(QCursor(Qt::IBeamCursor)); }
void QtQuickMarkdownEditor::mousePressEvent(QMouseEvent *event) {
  DEBUG << event;
  m_editor->mousePressEvent(event);
  this->update();
}
void QtQuickMarkdownEditor::keyReleaseEvent(QKeyEvent *event) { DEBUG << event; }
QVariant QtQuickMarkdownEditor::inputMethodQuery(Qt::InputMethodQuery query) const {
  switch (query) {
    case Qt::ImCursorRectangle: {
      auto pos = m_editor->cursorPos();
      auto rect = QRect(pos, QSize(5, 20));
      return rect;
    }
    case Qt::ImCursorPosition: {
      return m_editor->cursorPos();
    }
    default: {
    }
  }
  return QQuickItem::inputMethodQuery(query);
}
void QtQuickMarkdownEditor::inputMethodEvent(QInputMethodEvent *event) { m_editor->insertText(event->commitString()); }
}  // namespace md::editor