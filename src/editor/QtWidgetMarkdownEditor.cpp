//
// Created by PikachuHy on 2021/11/5.
//

#include "QtWidgetMarkdownEditor.h"

#include <QApplication>
#include <QClipboard>
#include <QDesktopServices>
#include <QDialog>
#include <QHBoxLayout>
#include <QInputMethod>
#include <QInputMethodEvent>
#include <QLabel>
#include <QMovie>
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
  m_editor->setLinkClickedCallback([](QString url) {
    DEBUG << url;
    QDesktopServices::openUrl(url);
  });
  m_editor->setCopyCodeBtnClickedCallback([this](QString code) {
    DEBUG << code;
    QApplication::clipboard()->setText(code);
  });
  m_editor->setImageClickedCallback([this](QString path) {
    QDialog dialog;
    dialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    auto hbox = new QHBoxLayout();
    auto imgLabel = new QLabel();
    if (path.endsWith(".gif")) {
      auto m = new QMovie(path);
      imgLabel->setMovie(m);
      m->start();
    } else {
      imgLabel->setPixmap(QPixmap(path));
    }
    hbox->addWidget(imgLabel);
    hbox->setContentsMargins(0, 0, 0, 0);
    dialog.setLayout(hbox);
    dialog.exec();
  });
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
  m_editor->drawSelection(m_offset, painter);
  m_editor->drawDoc(m_offset, painter);
  if (hasFocus()) {
    m_editor->drawCursor(m_offset, painter);
  }
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
      return m_editor->cursorRect();
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
  if (!str.isEmpty()) {
    m_editor->commitString(str);
  }
  auto preeditStr = event->preeditString();
  if (!preeditStr.isEmpty()) {
    m_editor->setPreedit(preeditStr);
  }
  viewport()->update();
}
void QtWidgetMarkdownEditor::mousePressEvent(QMouseEvent *event) {
  m_editor->mousePressEvent(m_offset, event);
  viewport()->update();
}
void QtWidgetMarkdownEditor::mouseMoveEvent(QMouseEvent *event) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QPoint pos = event->pos();
#else
    QPoint pos(event->position().x(), event->position().y());
#endif
  CursorShape shape = m_editor->cursorShape(m_offset, pos);
  setCursor(QCursor(static_cast<Qt::CursorShape>(shape)));
  m_editor->mouseMoveEvent(m_offset, event);
  viewport()->update();
}
void QtWidgetMarkdownEditor::reload() {}
void QtWidgetMarkdownEditor::mouseReleaseEvent(QMouseEvent *event) { m_editor->mouseReleaseEvent(m_offset, event); }
}  // namespace md::editor
