//
// Created by PikachuHy on 2021/11/6.
//

#include "QtQuickMarkdownEditor.h"

#include <QCursor>
#include <QGuiApplication>
#include <QDesktopServices>
#include <QClipboard>

#include "debug.h"
namespace md::editor {
QtQuickMarkdownEditor::QtQuickMarkdownEditor(QQuickItem *parent) : QQuickPaintedItem(parent) {
  m_editor = std::make_shared<Editor>();
  m_editor->setLinkClickedCallback([](QString url) {
    DEBUG << url;
    QDesktopServices::openUrl(url);
  });
  m_editor->setImageClickedCallback([this](QString path) {
    DEBUG << path;
    emit imageClicked(path);
  });
  m_editor->setCopyCodeBtnClickedCallback([this](QString code) {
    DEBUG << code;
    QGuiApplication::clipboard()->setText(code);
  });
  m_editor->setCheckBoxClickedCallback([this]() { emit contentChanged(); });
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setFlag(ItemAcceptsInputMethod, true);
  m_cursorTimer.start(500);
  connect(&m_cursorTimer, &QTimer::timeout, [this]() {
    m_showCursor = !m_showCursor;
    this->update();
  });
}
void QtQuickMarkdownEditor::paint(QPainter *painter) {
  Q_ASSERT(painter != nullptr);
  m_editor->drawDoc(QPoint(0, 0), *painter);
  setImplicitHeight(m_editor->height());
  if (hasActiveFocus() && m_showCursor) {
    m_editor->drawCursor(QPoint(0, 0), *painter);
  }
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
  int key = event->key();
  // 移动光标时避免闪烁
  if (key == Qt::Key_Left || key == Qt::Key_Right || key == Qt::Key_Up || key == Qt::Key_Down) {
    m_showCursor = true;
  }
  if ((event->modifiers() & Qt::Modifier::CTRL) && key == Qt::Key_S) {
    if (m_isNewDoc) {
      emit docSave(m_isNewDoc);
    } else {
      bool ok = m_editor->saveToFile(m_source);
      if (ok) {
        DEBUG << "save success";
        emit docSave(m_isNewDoc);
      } else {
        DEBUG << "save fail";
      }
    }
  } else if ((event->modifiers() & Qt::Modifier::CTRL) && key == Qt::Key_V) {
    auto s = QGuiApplication::clipboard()->text();
    if (!s.isEmpty()) {
      m_editor->insertText(s);
    } else {
      m_editor->keyPressEvent(event);
    }
    emit contentChanged();
  } else {
    m_editor->keyPressEvent(event);
    emit contentChanged();
  }
  this->update();
  setImplicitWidth(m_editor->width());
  setImplicitHeight(m_editor->height());
}
void QtQuickMarkdownEditor::hoverMoveEvent(QHoverEvent *event) {
  QPoint pos(event->position().x(), event->position().y());
  CursorShape shape = m_editor->cursorShape(QPoint(0, 0), pos);
  setCursor(QCursor(static_cast<Qt::CursorShape>(shape)));
}
void QtQuickMarkdownEditor::mousePressEvent(QMouseEvent *event) {
  forceActiveFocus();
  m_editor->mousePressEvent(QPoint(0, 0), event);
  this->update();
}
void QtQuickMarkdownEditor::keyReleaseEvent(QKeyEvent *event) {
  m_editor->keyReleaseEvent(event);
  this->update();
}
QVariant QtQuickMarkdownEditor::inputMethodQuery(Qt::InputMethodQuery query) const {
  switch (query) {
    case Qt::ImCursorRectangle: {
      return m_editor->cursorRect();
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
    m_editor->commitString(str);
  }
  auto preeditStr = event->preeditString();
  if (!preeditStr.isEmpty()) {
    m_editor->setPreedit(preeditStr);
  }
  this->update();
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
QString QtQuickMarkdownEditor::title() { return m_editor->title(); }
}  // namespace md::editor