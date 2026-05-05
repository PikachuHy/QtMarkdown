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

#include "platform/qt/QtAdapters.h"
#include "platform/qt/QtLatexPlatform.h"
#include "debug.h"
#include "editor/Editor.h"
namespace md::editor {
QtWidgetMarkdownEditor::QtWidgetMarkdownEditor(QWidget *parent) : QAbstractScrollArea(parent), m_offset(0, 0) {
  md::platform::qt::initLatex();
  setFocusPolicy(Qt::StrongFocus);
  setAttribute(Qt::WA_InputMethodEnabled);
  setMouseTracking(true);
  m_editor = std::make_shared<Editor>();
  m_editor->setLinkClickedCallback([](String url) {
    DEBUG << url;
    QDesktopServices::openUrl(toQString(url));
  });
  m_editor->setCopyCodeBtnClickedCallback([this](String code) {
    DEBUG << code;
    QApplication::clipboard()->setText(toQString(code));
  });
  m_editor->setImageClickedCallback([this](String path) {
    QDialog dialog;
    dialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    auto hbox = new QHBoxLayout();
    auto imgLabel = new QLabel();
    if (path.endsWith(".gif")) {
      auto m = new QMovie(toQString(path));
      imgLabel->setMovie(m);
      m->start();
    } else {
      imgLabel->setPixmap(QPixmap(toQString(path)));
    }
    hbox->addWidget(imgLabel);
    hbox->setContentsMargins(0, 0, 0, 0);
    dialog.setLayout(hbox);
    dialog.exec();
  });
  DEBUG << "viewport size:" << viewport()->sizeHint().width() << viewport()->sizeHint().height();
  m_cursorTimer.start(500);
  connect(&m_cursorTimer, &QTimer::timeout, [this]() { this->viewport()->update(); });
}
void QtWidgetMarkdownEditor::loadFile(QString path) {
  m_editor->loadFile(String(path.toStdString()));
  viewport()->update();
  QSize areaSize = viewport()->size();
  QSize widgetSize = this->size();
  verticalScrollBar()->setPageStep(areaSize.height());
  horizontalScrollBar()->setPageStep(areaSize.width());
  verticalScrollBar()->setRange(0, m_editor->height() - areaSize.height());
  horizontalScrollBar()->setRange(0, m_editor->width() - areaSize.width());
  DEBUG << "viewport size:" << viewport()->sizeHint().width() << viewport()->sizeHint().height();
}
void QtWidgetMarkdownEditor::paintEvent(QPaintEvent *event) {
  QPainter qpainter(viewport());
  QtPainterAdapter adapter(&qpainter);
  auto offset = fromQPoint(m_offset);
  m_editor->drawSelection(adapter, offset);
  m_editor->drawDoc(adapter, offset);
  if (hasFocus()) {
    m_editor->drawCursor(adapter, offset);
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
  QtKeyEvent adapter(event);
  m_editor->keyPressEvent(adapter);
  viewport()->update();
}
QVariant QtWidgetMarkdownEditor::inputMethodQuery(Qt::InputMethodQuery query) const {
  switch (query) {
    case Qt::ImCursorRectangle: {
      return toQRect(m_editor->cursorRect());
    }
    case Qt::ImCursorPosition: {
      return toQPoint(m_editor->cursorPos());
    }
    default: {
    }
  }
  return QAbstractScrollArea::inputMethodQuery(query);
}
void QtWidgetMarkdownEditor::inputMethodEvent(QInputMethodEvent *event) {
  auto str = event->commitString();
  if (!str.isEmpty()) {
    m_editor->commitString(String(str.toStdString()));
  }
  auto preeditStr = event->preeditString();
  if (!preeditStr.isEmpty()) {
    m_editor->setPreedit(String(preeditStr.toStdString()));
  }
  viewport()->update();
}
void QtWidgetMarkdownEditor::mousePressEvent(QMouseEvent *event) {
  QtMouseEvent adapter(event);
  m_editor->mousePressEvent(fromQPoint(m_offset), adapter);
  viewport()->update();
}
void QtWidgetMarkdownEditor::mouseMoveEvent(QMouseEvent *event) {
  QtMouseEvent adapter(event);
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QPoint pos = event->pos();
#else
    QPoint pos(event->position().x(), event->position().y());
#endif
  CursorShape shape = m_editor->cursorShape(fromQPoint(m_offset), fromQPoint(pos));
  setCursor(QCursor(static_cast<Qt::CursorShape>(shape)));
  m_editor->mouseMoveEvent(fromQPoint(m_offset), adapter);
  viewport()->update();
}
void QtWidgetMarkdownEditor::reload() {}
void QtWidgetMarkdownEditor::mouseReleaseEvent(QMouseEvent *event) {
  QtMouseEvent adapter(event);
  m_editor->mouseReleaseEvent(fromQPoint(m_offset), adapter);
}
}  // namespace md::editor
