//
// Created by pikachu on 5/22/2021.
//

#include "QtQuickMarkdownItem.h"

#include <QCursor>
#include <QDebug>
#include <QFile>
#include <QPainter>
#include <QTimer>

#include "Cursor.h"
#include "Document.h"
#include "EditorDocument.h"
#include "Parser.h"
#include "Render.h"
#include "debug.h"

QtQuickMarkdownItem::QtQuickMarkdownItem(QQuickItem *parent)
    : QQuickPaintedItem(parent),
      m_render(nullptr),
      m_lastWidth(-1),
      m_lastImplicitWidth(-1),
      m_cursor(new Cursor()),
      m_holdCtrl(false) {
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setFlag(ItemAcceptsInputMethod, true);
  QTimer::singleShot(0, this, [this]() {
    this->calculateHeight();
    this->update();
  });
  connect(this, &QtQuickMarkdownItem::widthChanged, [this]() {
    if (this->m_lastWidth == this->width()) return;
    this->m_lastWidth = this->width();
    calculateHeight();
  });
  connect(this, &QtQuickMarkdownItem::implicitWidthChanged, [this]() {
    if (this->m_lastImplicitWidth == this->implicitWidth()) return;
    this->m_lastImplicitWidth = this->implicitWidth();
    calculateHeight();
  });

  m_cursorTimer.setInterval(500);
  m_cursorTimer.start();
  connect(&m_cursorTimer, &QTimer::timeout, this, [this]() { update(); });
}

QtQuickMarkdownItem::~QtQuickMarkdownItem() { delete m_cursor; }

void QtQuickMarkdownItem::paint(QPainter *painter) {
  if (!m_render) return;
  if (width() == 0) return;
  m_render->setJustCalculate(false);
  m_render->reset(painter);
  m_doc->draw(m_render);
  if (hasFocus()) {
    m_doc->updateCursor(m_cursor);
    m_render->highlight(m_cursor);
    m_cursor->draw(*painter);
    m_cursor->setNeedUpdateCoord(false);
  }
}

void QtQuickMarkdownItem::setText(const QString &text) {
  if (text == m_text) return;
  this->m_text = text;
  m_doc = new EditorDocument(text);
  m_cursor->setEditorDocument(m_doc);
  calculateHeight();
}

QString QtQuickMarkdownItem::text() const { return m_text; }

QString QtQuickMarkdownItem::source() const { return m_source; }

void QtQuickMarkdownItem::setSource(const QString &source) {
  this->m_source = source;
  QFile file(source);
  if (file.exists()) {
    bool ok = file.open(QIODevice::ReadOnly);
    if (ok) {
      setText(file.readAll());
    } else {
      qWarning() << "file open fail." << source;
    }
  } else {
    qWarning() << "file not exist." << source;
  }
}

void QtQuickMarkdownItem::calculateHeight() {
  int w = static_cast<int>(width());
  if (w == 0) return;
  delete m_render;
  EditorDocument doc(m_text);
  if (w < 200) {
    w = 400;
  }
  QString path = m_path;
  if (path.isEmpty()) {
    path = m_source;
  }
  RenderSetting renderSetting;
  renderSetting.maxWidth = w;
  m_render = new Render(path, m_doc, renderSetting);
  m_cursor->setRender(m_render);
  m_render->setJustCalculate(true);
  doc.draw(m_render);
  setImplicitHeight(m_render->realHeight());
  setHeight(m_render->realHeight());
  emit heightChanged();
}

QString QtQuickMarkdownItem::path() const { return m_path; }

void QtQuickMarkdownItem::setPath(const QString &path) {
  m_path = path;
  calculateHeight();
  update();
}

void QtQuickMarkdownItem::mousePressEvent(QMouseEvent *event) {
  if (event->button() != Qt::LeftButton) {
    return;
  }
  forceActiveFocus();
  auto pos = event->pos();
  m_cursor->moveTo(pos);
  m_doc->fixCursorPos(m_cursor);
  update();
  for (auto link : m_doc->links()) {
    for (auto rect : link->rects) {
      if (rect.contains(pos) && m_holdCtrl) {
        qDebug() << "click link:" << link->url;
        emit linkClicked(link->url);
      }
    }
  }
  for (auto image : m_doc->images()) {
    if (image->rect.contains(pos)) {
      qDebug() << "click image:" << image->path;
      emit imageClicked(image->path);
    }
  }
  for (const auto &item : m_doc->codes()) {
    if (item->rect.contains(pos)) {
      qDebug() << "copy code:" << item->code;
      emit codeCopied(item->code);
    }
  }
}

void QtQuickMarkdownItem::hoverMoveEvent(QHoverEvent *event) {
  // 消除警告
  auto posf = event->position();
  QPoint pos(posf.x(), posf.y());
  for (auto link : m_doc->links()) {
    for (auto rect : link->rects) {
      if (rect.contains(pos)) {
        setCursor(QCursor(Qt::PointingHandCursor));
        return;
      }
    }
  }
  for (auto image : m_doc->images()) {
    if (image->rect.contains(pos)) {
      setCursor(QCursor(Qt::PointingHandCursor));
      return;
    }
  }
  for (auto code : m_doc->codes()) {
    if (code->rect.contains(pos)) {
      setCursor(QCursor(Qt::PointingHandCursor));
      return;
    }
  }

  setCursor(QCursor(Qt::IBeamCursor));
}

void QtQuickMarkdownItem::keyPressEvent(QKeyEvent *event) {
  if (event->key() == Qt::Key_Left) {
    m_cursor->moveLeft();
    update();
  } else if (event->key() == Qt::Key_Right) {
    m_cursor->moveRight();
    update();
  } else if (event->key() == Qt::Key_Up) {
    m_cursor->moveUp();
    update();
  } else if (event->key() == Qt::Key_Down) {
    m_cursor->moveDown();
    update();
  } else if (event->modifiers() & Qt::Modifier::CTRL) {
    m_holdCtrl = true;
  } else if (event->key() == Qt::Key_Backspace) {
//    DEBUG << m_cursor->coord();
    m_cursor->removeText();
    m_cursor->moveLeft(1, true);
//    m_render->setJustCalculate(true);
//    m_doc->draw(m_render);
//    DEBUG << m_cursor->coord();
  }
  else {
    // 需要把输入的字符插入到PieceTable中
    m_cursor->insertText(event->text());
    m_render->setJustCalculate(true);
    m_doc->draw(m_render);
    m_cursor->moveRight(event->text().size(), true);
  }
}

void QtQuickMarkdownItem::keyReleaseEvent(QKeyEvent *event) {
  if (event->modifiers() & Qt::Modifier::CTRL) {
    m_holdCtrl = false;
  }
  QQuickItem::keyReleaseEvent(event);
}

void QtQuickMarkdownItem::focusInEvent(QFocusEvent *event) {
  forceActiveFocus();
}

QVariant QtQuickMarkdownItem::inputMethodQuery(
    Qt::InputMethodQuery query) const {
  switch (query) {
    case Qt::ImCursorRectangle: {
      auto rect = QRect(m_cursor->pos(), QSize(5, m_cursor->h()));
      return rect;
    }
    case Qt::ImCursorPosition: {
      return m_cursor->pos();
    }
    default: {
    }
  }
  return QQuickItem::inputMethodQuery(query);
}

void QtQuickMarkdownItem::inputMethodEvent(QInputMethodEvent *event) {
  const auto& commitStr = event->commitString();
  if (!commitStr.isEmpty()) {
    m_cursor->insertText(commitStr);
    m_render->setJustCalculate(true);
    m_doc->draw(m_render);
    m_cursor->moveRight(commitStr.size(), true);
  }
}
