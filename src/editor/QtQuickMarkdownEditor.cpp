//
// Created by PikachuHy on 2021/11/6.
//

#include "QtQuickMarkdownEditor.h"

#include <QCursor>
#include <QGuiApplication>
#include <QDesktopServices>
#include <QClipboard>
#include <QFile>

#include "debug.h"
namespace md::editor {
QtQuickMarkdownEditor::QtQuickMarkdownEditor(QQuickItem *parent) : QQuickPaintedItem(parent) {
  m_editor = std::make_shared<Editor>();
  m_editor->setLinkClickedCallback([this](QString url) {
    DEBUG << url;
    emit linkClicked(url);
  });
  m_editor->setImageClickedCallback([this](QString path) {
    DEBUG << path;
    emit imageClicked(path);
  });
  m_editor->setCopyCodeBtnClickedCallback([this](QString code) {
    DEBUG << code;
    emit codeCopied(code);
  });
  m_editor->setCheckBoxClickedCallback([this]() { markContentChanged(); });
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setFlag(ItemAcceptsInputMethod, true);
  m_cursorTimer.start(500);
  connect(&m_cursorTimer, &QTimer::timeout, [this]() {
    m_showCursor = !m_showCursor;
    this->update();
  });
  m_tmpSaveTimer.start(30 * 1000);
  connect(&m_tmpSaveTimer, &QTimer::timeout, this, &QtQuickMarkdownEditor::tmpSave);
  connect(this, &QtQuickMarkdownEditor::widthChanged, this, [this]() {
    int w = this->width();
    if (w > 0) {
      m_editor->setWidth(this->width());
      this->m_editor->renderDocument();
      this->update();
    }
  });
}
void QtQuickMarkdownEditor::paint(QPainter *painter) {
  Q_ASSERT(painter != nullptr);
#ifdef Q_OS_ANDROID
  m_editor->drawDoc(QPoint(0, 0), *painter);
  setImplicitHeight(m_editor->height());
#else
  m_editor->drawSelection(QPoint(0, 0), *painter);
  m_editor->drawDoc(QPoint(0, 0), *painter);
  setImplicitHeight(m_editor->height());
  if (hasActiveFocus() && m_showCursor) {
    m_editor->drawCursor(QPoint(0, 0), *painter);
  }
#endif
  emit cursorCoordChanged(m_editor->cursorCoord());
  emit implicitHeightChanged();
}
void QtQuickMarkdownEditor::setText(const QString &text) {}
void QtQuickMarkdownEditor::setSource(const QString &source) {
  if (m_source == source) return;
  emit sourceChanged(m_source);
  m_source = source;
  m_isNewDoc = false;
  auto tmpPath = this->tmpPath();
  DEBUG << tmpPath;
  int w = this->width();
  if (w > 0) {
    m_editor->setWidth(this->width());
  }
  m_editor->setResPathList(m_resPathList);
  if (QFile(tmpPath).exists()) {
    m_editor->loadFile(tmpPath);
    markContentChanged();
  } else {
    m_editor->loadFile(url2path(source));
  }
  setImplicitWidth(m_editor->width());
  setImplicitHeight(m_editor->height());
  setHeight(m_editor->height());
  emit implicitHeightChanged();
}
void QtQuickMarkdownEditor::addPath(const QString &path) {
    m_resPathList.append(path);
}
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
      this->save();
    }
  } else if ((event->modifiers() & Qt::Modifier::CTRL) && key == Qt::Key_V) {
    auto s = QGuiApplication::clipboard()->text();
    if (!s.isEmpty()) {
      m_editor->insertText(s);
    } else {
      m_editor->keyPressEvent(event);
    }
    markContentChanged();
  } else {
    m_editor->keyPressEvent(event);
    markContentChanged();
  }
  this->update();
  setImplicitWidth(m_editor->width());
  setImplicitHeight(m_editor->height());
  setHeight(m_editor->height());
}
void QtQuickMarkdownEditor::hoverMoveEvent(QHoverEvent *event) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QPoint pos = event->pos();
#else
    QPoint pos(event->position().x(), event->position().y());
#endif
  CursorShape shape = m_editor->cursorShape(QPoint(0, 0), pos);
  setCursor(QCursor(static_cast<Qt::CursorShape>(shape)));
}
void QtQuickMarkdownEditor::mousePressEvent(QMouseEvent *event) {
  forceActiveFocus();
  m_editor->mousePressEvent(QPoint(0, 0), event);
  this->update();
  emit showInputMethod();
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
  if (str.isEmpty()) {
    auto preeditStr = event->preeditString();
    m_editor->setPreedit(preeditStr);
  } else {
    m_editor->commitString(str);
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
  bool ok = m_editor->saveToFile(url2path(m_source));
  if (ok) {
    DEBUG << "save success";
    auto tmpPath = this->tmpPath();
    QFile file(tmpPath);
    if (file.exists()) {
      ok = file.remove();
      if (ok) {
        DEBUG << "remove tmp file success";
      } else {
        DEBUG << "remove tmp file fail";
      }
    }
    emit docSave(m_isNewDoc);
  } else {
    DEBUG << "save fail";
  }
}
QString QtQuickMarkdownEditor::title() { return m_editor->title(); }
void QtQuickMarkdownEditor::mouseMoveEvent(QMouseEvent *event) {
  return m_editor->mouseMoveEvent(Point(0, 0), event);
  this->update();
}
void QtQuickMarkdownEditor::tmpSave() {
  // 新文档暂时不考虑
  if (m_isNewDoc) return;
  if (!m_contentChanged) return;
  auto path = tmpPath();
  // qrc的文件直接忽略
  if (path.startsWith(":")) {
    return;
  }
  auto ok = m_editor->saveToFile(path);
  if (ok) {
    DEBUG << "tmp save success";
  } else {
    DEBUG << "tmp save fail";
  }
}
void QtQuickMarkdownEditor::markContentChanged() {
  emit contentChanged();
  m_contentChanged = true;
}
QString QtQuickMarkdownEditor::tmpPath() {
  auto index = m_source.lastIndexOf("/");
  auto name = m_source.mid(index + 1);
  auto tmpPath = m_source.left(index) + "/~" + name;
  QString prefix = "file://";
  if (tmpPath.startsWith(prefix)) {
    return url2path(tmpPath);
  }
  return tmpPath;
}
void QtQuickMarkdownEditor::save() { this->saveToFile(url2path(m_source)); }
QString QtQuickMarkdownEditor::url2path(QString url) {
  String path = url;
  String prefix = "file://";
  if (url.startsWith(prefix)) {
#ifdef Q_OS_WIN
    for (char ch = 'C'; ch <= 'Z'; ch++) {
      String diskPrefix = prefix + "/" + ch + ":";
      if (url.startsWith(diskPrefix)) {
        path = url.mid(prefix.size() + 1);
        return path;
      }
    }
#else
#endif
    return path.mid(prefix.size());
  }
  return path;
}
}  // namespace md::editor
