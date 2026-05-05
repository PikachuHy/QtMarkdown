//
// Created by PikachuHy on 2021/11/6.
//

#include "QtQuickMarkdownEditor.h"

#include <QCursor>
#include <QGuiApplication>
#include <QDesktopServices>
#include <QClipboard>
#include <QFile>

#include "platform/qt/QtAdapters.h"
#include "platform/qt/QtLatexPlatform.h"
#include "debug.h"
namespace md::editor {
QtQuickMarkdownEditor::QtQuickMarkdownEditor(QQuickItem *parent) : QQuickPaintedItem(parent) {
  md::platform::qt::initLatex();
  m_editor = std::make_shared<Editor>();
  m_editor->setLinkClickedCallback([this](String url) {
    DEBUG << url;
    emit linkClicked(toQString(url));
  });
  m_editor->setImageClickedCallback([this](String path) {
    DEBUG << path;
    emit imageClicked(toQString(path));
  });
  m_editor->setCopyCodeBtnClickedCallback([this](String code) {
    DEBUG << code;
    emit codeCopied(toQString(code));
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
  QtPainterAdapter adapter(painter);
  core::Point offset(0, 0);
#ifdef __ANDROID__
  m_editor->drawDoc(adapter, offset);
  setImplicitHeight(m_editor->height());
#else
  m_editor->drawSelection(adapter, offset);
  m_editor->drawDoc(adapter, offset);
  setImplicitHeight(m_editor->height());
  if (hasActiveFocus()) {
    m_editor->drawCursor(adapter, offset);
  }
#endif
  emit cursorCoordChanged(toQString(m_editor->cursorCoord()));
  emit implicitHeightChanged();
}
void QtQuickMarkdownEditor::setText(const QString &text) {}
void QtQuickMarkdownEditor::setSource(const QString &source) {
  if (m_source == source) return;
  emit sourceChanged(m_source);
  m_source = source;
  m_isNewDoc = false;
  auto tmpPath = this->tmpPath();
  DEBUG << tmpPath.toStdString();
  int w = this->width();
  if (w > 0) {
    m_editor->setWidth(this->width());
  }
  {
    StringList pathList;
    for (const auto& p : m_resPathList) {
      pathList.push_back(String(p.toStdString()));
    }
    m_editor->setResPathList(pathList);
  }
  if (QFile(tmpPath).exists()) {
    m_editor->loadFile(String(tmpPath.toStdString()));
    markContentChanged();
  } else {
    m_editor->loadFile(String(url2path(source).toStdString()));
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
      m_editor->insertText(String(s.toStdString()));
    } else {
      QtKeyEvent adapter(event);
      m_editor->keyPressEvent(adapter);
    }
    markContentChanged();
  } else {
    QtKeyEvent adapter(event);
    m_editor->keyPressEvent(adapter);
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
  CursorShape shape = m_editor->cursorShape(core::Point(0, 0), fromQPoint(pos));
  setCursor(QCursor(static_cast<Qt::CursorShape>(shape)));
}
void QtQuickMarkdownEditor::mousePressEvent(QMouseEvent *event) {
  forceActiveFocus();
  QtMouseEvent adapter(event);
  m_editor->mousePressEvent(core::Point(0, 0), adapter);
  this->update();
  emit showInputMethod();
}
void QtQuickMarkdownEditor::keyReleaseEvent(QKeyEvent *event) {
  QtKeyEvent adapter(event);
  m_editor->keyReleaseEvent(adapter);
  this->update();
}
QVariant QtQuickMarkdownEditor::inputMethodQuery(Qt::InputMethodQuery query) const {
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
  return QQuickItem::inputMethodQuery(query);
}
void QtQuickMarkdownEditor::inputMethodEvent(QInputMethodEvent *event) {
  auto str = String(event->commitString().toStdString());
  if (str.isEmpty()) {
    auto preeditStr = String(event->preeditString().toStdString());
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
  bool ok = m_editor->saveToFile(String(url2path(m_source).toStdString()));
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
QString QtQuickMarkdownEditor::title() { return toQString(m_editor->title()); }
void QtQuickMarkdownEditor::mouseMoveEvent(QMouseEvent *event) {
  QtMouseEvent adapter(event);
  m_editor->mouseMoveEvent(core::Point(0, 0), adapter);
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
  auto ok = m_editor->saveToFile(String(path.toStdString()));
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
  QString prefix = "file://";
  if (url.startsWith(prefix)) {
#ifdef Q_OS_WIN
    for (char ch = 'C'; ch <= 'Z'; ch++) {
      QString diskPrefix = prefix + "/" + ch + ":";
      if (url.startsWith(diskPrefix)) {
        return url.mid(prefix.size() + 1);
      }
    }
#else
#endif
    return url.mid(prefix.size());
  }
  return url;
}
}  // namespace md::editor
