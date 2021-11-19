//
// Created by PikachuHy on 2021/11/5.
//

#include "QtWidgetMarkdownEditor.h"

#include <QInputMethod>
#include <QInputMethodEvent>
#include <QPainter>
#include <QRect>
#include <QScrollBar>
#include <QVariant>

#include "debug.h"
#include "editor/Editor.h"
namespace md::editor {
QtWidgetMarkdownEditor::QtWidgetMarkdownEditor(QWidget *parent) : QAbstractScrollArea(parent), m_offset(0, 0) {
  setFocusPolicy(Qt::StrongFocus);
  setAttribute(Qt::WA_InputMethodEnabled);
  setMouseTracking(true);
  m_editor = std::make_shared<Editor>();
  DEBUG << "viewport size:" << viewport()->sizeHint();
  m_cursorTimer.start(500);
  connect(&m_cursorTimer, &QTimer::timeout, [this]() { this->viewport()->update(); });
}
void QtWidgetMarkdownEditor::loadFile(QString path) {
  m_editor->loadFile(path);
  viewport()->update();
  QSize areaSize = viewport()->size();
  QSize widgetSize = this->size();
  verticalScrollBar()->setPageStep(areaSize.height());
  horizontalScrollBar()->setPageStep(areaSize.width());
#if 0
  verticalScrollBar()->setPageStep(areaSize.height());
  horizontalScrollBar()->setPageStep(areaSize.width());
  verticalScrollBar()->setRange(0, widgetSize.height() - areaSize.height());
  horizontalScrollBar()->setRange(0, widgetSize.width() - areaSize.width());
#endif

  verticalScrollBar()->setRange(0, m_editor->height() - areaSize.height());
  horizontalScrollBar()->setRange(0, m_editor->width() - areaSize.width());
  DEBUG << "viewport size:" << viewport()->sizeHint();
}
void QtWidgetMarkdownEditor::paintEvent(QPaintEvent *event) {
  QPainter painter(viewport());
  m_editor->drawDoc(m_offset, painter);
}
void QtWidgetMarkdownEditor::scrollContentsBy(int dx, int dy) {
  auto x = m_offset.x() + dx;
  auto y = m_offset.y() + dy;
  m_offset = QPoint(x, y);
  viewport()->update();
}
QSize QtWidgetMarkdownEditor::viewportSizeHint() const { return {m_editor->width(), m_editor->height()}; }
void QtWidgetMarkdownEditor::keyPressEvent(QKeyEvent *event) {
  m_editor->keyPressEvent(event);
  viewport()->update();
}
QVariant QtWidgetMarkdownEditor::inputMethodQuery(Qt::InputMethodQuery query) const {
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
  return QAbstractScrollArea::inputMethodQuery(query);
}
void QtWidgetMarkdownEditor::inputMethodEvent(QInputMethodEvent *event) {
  auto str = event->commitString();
  DEBUG << str;
  m_editor->insertText(str);
}
void QtWidgetMarkdownEditor::mousePressEvent(QMouseEvent *event) {
  m_editor->mousePressEvent(Point(0, 0), event);
  viewport()->update();
}
void QtWidgetMarkdownEditor::mouseMoveEvent(QMouseEvent *event) { setCursor(QCursor(Qt::IBeamCursor)); }
void QtWidgetMarkdownEditor::reload() {}
}  // namespace md::editor