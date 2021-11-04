//
// Created by pikachu on 2021/3/19.
//

#include "Editor.h"

#include <QApplication>
#include <QClipboard>
#include <QDate>
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QMovie>
#include <QPainter>
#include <QProcess>
#include <QScrollArea>
#include <QStyle>
#include <QTemporaryFile>
#include <QTextBlock>
#include <QTextFrame>
#include <QTextList>
#include <QTextTable>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <array>

#include "EditorDocument.h"
#include "QtMarkdownParser"
#include "Render.h"

class EditorWidget : public QWidget {
  Q_OBJECT
 public:
  explicit EditorWidget(Editor *parent = nullptr);

 protected:
  void paintEvent(QPaintEvent *e) override;

  void mouseMoveEvent(QMouseEvent *event) override;

  void mousePressEvent(QMouseEvent *event) override;

  void resizeEvent(QResizeEvent *event) override;

 public:
  bool eventFilter(QObject *watched, QEvent *event) override;
  void loadFile(const QString &path);
  void reload();
  void setLinkClickedCallback(std::function<bool(QString)> &fn);

 private:
  void drawInBackground();
  void drawAsync();

 private:
  QImage m_buffer;
  bool m_needDraw;
  int m_rightMargin;
  QList<Element::Link *> m_links;
  QList<Element::Image *> m_images;
  QList<Element::CodeBlock *> m_codes;
  Editor *m_editor;
  bool m_isDrawing;
  int m_maxWidth;
  QSize m_fixedSize;
  QString m_filePath;
  std::function<bool(QString)> m_linkClickedCallback;
  EditorDocument *m_doc;
};

template <typename T>
void checkFuture(QFuture<T> future, std::function<void(T)> callback) {
  if (!future.isFinished()) {
    QTimer::singleShot(100,
                       [future, callback]() { checkFuture(future, callback); });
  } else {
    callback(future.result());
  }
}
EditorWidget::EditorWidget(Editor *parent)
    : QWidget(parent), m_needDraw(true), m_editor(parent), m_doc(nullptr) {
  m_rightMargin = 0;
  setMouseTracking(true);
  m_buffer = QImage(QSize(800, 600), QImage::Format_RGB32);
  m_buffer.fill(Qt::white);
  m_linkClickedCallback = [](const QString &) { return false; };
}

void EditorWidget::paintEvent(QPaintEvent *e) {
  Q_UNUSED(e);
  //    qDebug() << e;
  if (m_needDraw) {
    m_needDraw = false;
    this->drawInBackground();
  }
  QPainter painter(this);
  painter.drawImage(0, 0, m_buffer);
}

void EditorWidget::mouseMoveEvent(QMouseEvent *event) {
  auto pos = event->pos();
  for (auto link : m_links) {
    for (auto rect : link->rects) {
      if (rect.contains(pos)) {
        setCursor(QCursor(Qt::PointingHandCursor));
        return;
      }
    }
  }
  for (auto image : m_images) {
    if (image->rect.contains(pos)) {
      setCursor(QCursor(Qt::PointingHandCursor));
      return;
    }
  }
  for (auto code : m_codes) {
    if (code->rect.contains(pos)) {
      setCursor(QCursor(Qt::PointingHandCursor));
      return;
    }
  }

  setCursor(QCursor(Qt::ArrowCursor));
}

void EditorWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() != Qt::LeftButton) {
    return;
  }
  auto pos = event->pos();
  for (auto link : m_links) {
    for (auto rect : link->rects) {
      if (rect.contains(pos)) {
        if (m_linkClickedCallback(link->url)) {
          // 用户已处理
        } else {
          auto ret = QMessageBox::question(this, tr("Open URL"),
                                           QString("%1").arg(link->url));
          if (ret == QMessageBox::Yes) {
            QDesktopServices::openUrl(QUrl(link->url));
          }
        }
      }
    }
  }
  for (auto image : m_images) {
    if (image->rect.contains(pos)) {
      QDialog dialog;
      dialog.setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
      auto hbox = new QHBoxLayout();
      auto imgLabel = new QLabel();
      if (image->path.endsWith(".gif")) {
        auto m = new QMovie(image->path);
        imgLabel->setMovie(m);
        m->start();
      } else {
        imgLabel->setPixmap(QPixmap(image->path));
      }
      hbox->addWidget(imgLabel);
      hbox->setContentsMargins(0, 0, 0, 0);
      dialog.setLayout(hbox);
      dialog.exec();
    }
  }
  for (auto code : m_codes) {
    if (code->rect.contains(pos)) {
      QApplication::clipboard()->setText(code->code);
    }
  }
}

void EditorWidget::resizeEvent(QResizeEvent *event) {
  //    qDebug() << __FUNCTION__ ;
  //    m_needDraw = true;
  this->update();
}

bool EditorWidget::eventFilter(QObject *watched, QEvent *event) {
  //    qDebug() << event->type() << watched << m_editor;
  if (watched == m_editor) {
    if (event->type() == QEvent::Resize) {
      QResizeEvent *e = dynamic_cast<QResizeEvent *>(event);
      //            qDebug() << e->size();
      auto scrollBarWidth =
          this->style()->pixelMetric(QStyle::PM_ScrollBarSliderMin);
      this->m_maxWidth = qMax(600, e->size().width()) - scrollBarWidth - 1;
      if (!m_isDrawing) {
        m_needDraw = true;
        //                qDebug() << __FUNCTION__ ;
        this->update();
      }
    }
  }
  return QObject::eventFilter(watched, event);
}

void EditorWidget::loadFile(const QString &path) {
  m_filePath = path;
  m_needDraw = true;
}

void EditorWidget::reload() {
  m_needDraw = true;
  update();
}

void EditorWidget::setLinkClickedCallback(std::function<bool(QString)> &fn) {
  this->m_linkClickedCallback = fn;
}

void EditorWidget::drawInBackground() {
  //    m_maxWidth = qMax(600, parentWidget()->width());
  auto ret = QtConcurrent::run(
      [this](int) {
        this->m_isDrawing = true;
        this->drawAsync();
        return 0;
      },
      0);
  checkFuture<int>(ret, [this](int) {
    setFixedSize(m_fixedSize);
    this->m_isDrawing = false;
    this->update();
  });
}

void EditorWidget::drawAsync() {
#if 0  // 暂时用不到Widget，后面再改吧
    QFile mdFile(m_filePath);
    if (!mdFile.exists()) {
        qDebug() << "file not exist:" << mdFile.fileName();
        return;
    }
    mdFile.open(QIODevice::ReadOnly);
    auto mdText = mdFile.readAll();
    mdFile.close();
    EditorDocument doc(mdText);
    RenderSetting renderSetting;
    renderSetting.maxWidth = m_maxWidth;
    Render visitor(m_filePath, renderSetting);
    visitor.setJustCalculate(true);
    doc.accept(&visitor);
    int h = visitor.realHeight();
    if (h < 0) {
        h = 600;
    }
    h = std::max(h, this->height());
    auto w = qMax(m_maxWidth, visitor.realWidth());
    // qDebug() << "set size:" << w << h;
    m_fixedSize = QSize(w, h);
    m_buffer = QImage(m_fixedSize, QImage::Format_RGB32);
    QPainter painter(&m_buffer);
    painter.setRenderHint(QPainter::Antialiasing);
    m_buffer.fill(Qt::white);
    visitor.setJustCalculate(false);
    visitor.reset(&painter);
    doc.accept(&visitor);
    m_links = visitor.links();
    m_images = visitor.images();
    m_codes = visitor.codes();
#endif
}

Editor::Editor(QWidget *parent) : QScrollArea(parent) {
  setWidgetResizable(true);
  m_editorWidget = new EditorWidget(this);
  setWidget(m_editorWidget);
  installEventFilter(m_editorWidget);
}

void Editor::loadFile(const QString &path) { m_editorWidget->loadFile(path); }

void Editor::reload() { m_editorWidget->reload(); }

void Editor::setLinkClickedCallback(std::function<bool(QString)> fn) {
  m_editorWidget->setLinkClickedCallback(fn);
}

#include "Editor.moc"
