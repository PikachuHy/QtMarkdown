//
// Created by PikachuHy on 2021/11/6.
//

#include "QtQuickMarkdownEditor.h"

#include <QCursor>
#include <QGuiApplication>

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
  setImplicitHeight(m_editor->height());
  emit cursorCoordChanged(m_editor->cursorCoord());
  emit implicitHeightChanged();
}
void QtQuickMarkdownEditor::setText(const QString &text) {}
void QtQuickMarkdownEditor::setSource(const QString &source) {
  if (m_source == source) return;
  emit sourceChanged(m_source);
  m_source = source;
  m_isNewDoc = false;
  m_editor->loadFile(source);
  setImplicitWidth(m_editor->width());
  setImplicitHeight(m_editor->height());
  emit implicitHeightChanged();
}
void QtQuickMarkdownEditor::setPath(const QString &path) {}
void QtQuickMarkdownEditor::keyPressEvent(QKeyEvent *event) {
  if ((event->modifiers() & Qt::Modifier::CTRL) && event->key() == Qt::Key_S) {
    if (m_isNewDoc) {
      emit docSave();
    } else {
      bool ok = m_editor->saveToFile(m_source);
      if (ok) {
        DEBUG << "save success";
      } else {
        DEBUG << "save fail";
      }
    }
  } else {
    m_editor->keyPressEvent(event);
  }
  this->update();
  setImplicitWidth(m_editor->width());
  setImplicitHeight(m_editor->height());
}
void QtQuickMarkdownEditor::focusOutEvent(QFocusEvent *event) { forceActiveFocus(); }
void QtQuickMarkdownEditor::hoverMoveEvent(QHoverEvent *event) { setCursor(QCursor(Qt::IBeamCursor)); }
void QtQuickMarkdownEditor::mousePressEvent(QMouseEvent *event) {
  m_editor->mousePressEvent(event);
  this->update();
}
void QtQuickMarkdownEditor::keyReleaseEvent(QKeyEvent *event) {}
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
void QtQuickMarkdownEditor::inputMethodEvent(QInputMethodEvent *event) {
  auto str = event->commitString();
  if (!str.isEmpty()) {
    m_editor->insertText(str);
    this->update();
  }
}
void QtQuickMarkdownEditor::newDoc() {
  m_editor->reset();
  m_isNewDoc = true;
}
void QtQuickMarkdownEditor::saveToFile(const QString &path) {
  m_source = path;
  m_isNewDoc = false;
  bool ok = m_editor->saveToFile(m_source);
  if (ok) {
    DEBUG << "save success";
  } else {
    DEBUG << "save fail";
  }
}
}  // namespace md::editor