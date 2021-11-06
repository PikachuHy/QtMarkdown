//
// Created by PikachuHy on 2021/11/5.
//

#include "QtWidgetMarkdownEditor.h"

#include <QPainter>
#include <QScrollBar>

#include "debug.h"
using namespace md::editor;
QtWidgetMarkdownEditor::QtWidgetMarkdownEditor(QWidget *parent) : QAbstractScrollArea(parent), m_offset(0, 0) {
  m_editor = std::make_shared<Editor>();
  DEBUG << "viewport size:" << viewport()->sizeHint();
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
  m_editor->paintEvent(m_offset, painter);
}
void QtWidgetMarkdownEditor::scrollContentsBy(int dx, int dy) {
  auto x = m_offset.x() + dx;
  auto y = m_offset.y() + dy;
  m_offset = QPoint(x, y);
  viewport()->update();
}
QSize QtWidgetMarkdownEditor::viewportSizeHint() const { return {m_editor->width(), m_editor->height()}; }
