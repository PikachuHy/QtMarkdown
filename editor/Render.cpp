//
// Created by pikachu on 5/22/2021.
//

#include "Render.h"

#include <QCryptographicHash>
#include <QDebug>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QStandardPaths>

#include "Cursor.h"
#include "EditorDocument.h"
#include "PieceTable.h"
#include "debug.h"
#include "latex.h"
#include "platform/qt/graphic_qt.h"
using namespace md::parser;
class TexRender {
 public:
  TexRender(QString mathFontPath, QString clmPath) {
    tex::FontSpec math{"xits", mathFontPath.toStdString(),
                       clmPath.toStdString()};
    tex::LaTeX::init(math);
  }
  ~TexRender() { tex::LaTeX::release(); }
};
class RenderPrivate {
 public:
  explicit RenderPrivate(Render *render) : q(render) {}
  void reset(QPainter *painter) {
    m_curX = m_setting.docMargin.left();
    m_curY = m_setting.docMargin.top();
    m_lastMaxHeight = 0;
    QFont font;
#ifdef Q_OS_WIN
    font.setFamily("微软雅黑");
#elif defined(Q_OS_MAC)
    font.setFamily("PingFang SC");
#else
#endif
    font.setPixelSize(16);
    if (painter) {
      m_painter = painter;
      m_painter->setFont(font);
    } else {
      m_font = font;
    }
  }
  void save() {
    if (justCalculate()) {
      m_fonts.push(curFont());
    } else {
      m_painter->save();
    }
  }

  void restore() {
    if (justCalculate()) {
      m_font = m_fonts.pop();
    } else {
      m_painter->restore();
    }
  }

  void setFont(const QFont &font) {
    if (justCalculate()) {
      m_font = font;
    } else {
      m_painter->setFont(font);
    }
  }

  void setPen(const QColor &color) {
    if (justCalculate()) {
      // do nothing
    } else {
      m_painter->setPen(color);
    }
  }

  QFontMetrics fontMetrics() {
    if (justCalculate()) {
      QFontMetrics fm(curFont());
      return fm;
    } else {
      return m_painter->fontMetrics();
    }
  }

  void drawText(const QRectF &r, const QString &text,
                const QTextOption &o = QTextOption()) {
    if (justCalculate()) {
      // do nothing
    } else {
      m_painter->drawText(r, text, o);
    }
  }

  void drawImage(const QRect &r, const QImage &image) {
    if (justCalculate()) {
      // do nothing
    } else {
      m_painter->drawImage(r, image);
    }
  }

  void drawPixmap(const QRect &r, const QPixmap &pm) {
    if (justCalculate()) {
      // do nothing
    } else {
      m_painter->drawPixmap(r, pm);
    }
  }

  void fillRect(const QRect &rect, const QBrush &b) {
    if (justCalculate()) {
      // do nothing
    } else {
      m_painter->fillRect(rect, b);
    }
  }

  QRect textRect(const QString &text) {
    QFontMetrics metrics = fontMetrics();
    const QRect rect(m_curX, m_curY, m_setting.contentMaxWidth(), 0);
    QRect textBoundingRect = metrics.boundingRect(rect, Qt::TextWordWrap, text);
    return textBoundingRect;
  }

  int textWidth(const QString &text) {
    QFontMetrics metrics = fontMetrics();
    int w = metrics.horizontalAdvance(text);
    return w;
  }

  int charWidth(const QChar &ch) {
    QFontMetrics metrics = fontMetrics();
    int w = metrics.horizontalAdvance(ch);
    return w;
  }

  int textHeight() { return fontMetrics().height(); }

  bool currentLineCanDrawText(const QString &text) {
    auto needWidth = textWidth(text);
    if (m_curX + needWidth < m_setting.contentMaxWidth()) {
      return true;
    } else {
      return false;
    }
  }

  int countOfThisLineCanDraw(const QString &text) {
    // 计算这一行可以画多少个字符
    auto ch_w = charWidth(text.at(0));
    int left_w = m_setting.contentMaxWidth() - m_curX;
    int may_ch_count = left_w / ch_w - 1;
    // 可能根本画不了
    if (may_ch_count <= 0) return 0;
    if (currentLineCanDrawText(text.left(may_ch_count + 1))) {
      while (currentLineCanDrawText(text.left(may_ch_count + 1))) {
        may_ch_count++;
      }
    } else {
      while (!currentLineCanDrawText(text.left(may_ch_count))) {
        may_ch_count--;
      }
    }
    return may_ch_count;
  }
  QList<QRect> drawText(Text *text) {
    QList<QRect> rects;
    // 先将Text*转成PieceTable*
    // 然后渲染PieceTable
    auto table = m_doc->pieceTable(text);
    if (!table) return {};
    qsizetype totalOffset = 0;
    for (auto item : *table) {
      qsizetype startIndex = 0;
      while (!currentLineCanDrawText(item.mid(startIndex))) {
        auto count = countOfThisLineCanDraw(item.mid(startIndex));
        if (count == 0) {
          moveToNewLine();
          continue;
        }
        const QString &textToDraw = item.mid(startIndex, count);
        auto rect = drawTextInCurrentLine(textToDraw);
        Cell cell{curFont(), rect, textToDraw, table, totalOffset, text};
        m_doc->appendCell(cell);
        rects.append(rect);
        startIndex += count;
        totalOffset += count;
        moveToNewLine();
      }
      auto lastText = item.mid(startIndex);
      if (!lastText.isEmpty()) {
        auto rect = drawTextInCurrentLine(lastText);
        rects.append(rect);
        Cell cell{curFont(), rect, lastText, table, totalOffset, text};
        m_doc->appendCell(cell);
        totalOffset += lastText.size();
      }
    }
    return rects;
  }
  QList<QRect> drawText(QString text) {
    if (text == "\r") return {};
    if (text.isEmpty()) return {};
    // 一个文本可能一行画不完，需要多个大rect去存
    // 小大rect去存每个字
    QList<QRect> rects;
    while (!currentLineCanDrawText(text)) {
      auto count = countOfThisLineCanDraw(text);
      // 处理画不了的特殊情况
      if (count == 0) {
        moveToNewLine();
        continue;
      }
      auto rect = drawTextInCurrentLine(text.left(count));

      Cell cell{curFont(), rect, text.left(count)};
      m_doc->appendCell(cell);
      rects.append(rect);
      text = text.right(text.size() - count);
      moveToNewLine();
    }
    if (!text.isEmpty()) {
      auto rect = drawTextInCurrentLine(text);
      rects.append(rect);
      Cell cell{curFont(), rect, text};
      m_doc->appendCell(cell);
    }
    return rects;
  }

  QRect drawTextInCurrentLine(const QString &text, bool adjustX = true,
                              bool adjustY = true) {
    QRect rect = textRect(text);
    drawText(rect, text);
    if (adjustX) {
      m_curX += rect.width();
    }
    if (adjustY) {
      m_lastMaxHeight = qMax(m_lastMaxHeight, rect.height());
    }
    return rect;
  }

  void moveToNewLine() {
    m_curY += m_lastMaxHeight;
    m_curX = m_setting.docMargin.left();
    m_curY += m_setting.lineSpacing;
    m_lastMaxHeight = 0;
    m_doc->createNewLineData();
  }

  int realHeight() const {
    return m_curY + m_lastMaxHeight + m_setting.docMargin.bottom();
  }

  int realWidth() const { return m_lastMaxWidth; }

  bool justCalculate() const { return m_justCalculate; }

  QFont curFont() {
    if (justCalculate()) {
      return m_font;
    } else {
      return m_painter->font();
    }
  }
  QFont codeFont() {
    auto font = curFont();
#ifdef Q_OS_WiN
    font.setFamily("Cascadia Code");
    font.setPixelSize(20);
#else
    font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPixelSize(20);
#endif
    return font;
  }
  void drawRect(const QRect &r, const QColor fb = Qt::black) {
    if (justCalculate()) {
      // do nothing
    } else {
      m_painter->save();
      m_painter->setPen(fb);
      m_painter->drawRect(r);
      m_painter->restore();
    }
  }

  void drawCodeBlock(const String &code) {
    moveToNewLine();
    auto y = m_curY;
    m_curY += m_setting.codeMargin.top();
    m_curX += m_setting.codeMargin.left();
    save();
    // #f9f9f9
    //        m_painter.setBackground(QBrush(QColor(249, 249, 249)));
    setFont(codeFont());
    auto rect = textRect(code);
    const QRect bgRect =
        QRect(0, y, m_setting.contentMaxWidth(), rect.height());
    fillRect(bgRect, QBrush(QColor(249, 249, 249)));
    drawText(rect, code);
    restore();
    m_curY += rect.height();
    if (!justCalculate()) {
      auto e = new Element::CodeBlock();
      e->code = code;
      QString copyBtnFilePath = ":icon/copy_32x32.png";
      QFile copyBtnFile(copyBtnFilePath);
      if (copyBtnFile.exists()) {
        QPixmap copyBtnImg(copyBtnFilePath);
        QRect copyBtnRect(
            QPoint(bgRect.width() - copyBtnImg.width(), bgRect.y()),
            copyBtnImg.size());
        e->rect = copyBtnRect;
        m_doc->appendCodeBlock(e);
        drawPixmap(copyBtnRect, copyBtnImg);
      } else {
        qWarning() << "copy btn file not exist." << copyBtnFilePath;
      }
    }
  }

  void drawCodeBlock(Text *text) {
    moveToNewLine();
    auto y = m_curY;
    m_curY += m_setting.codeMargin.top();
    m_curX += m_setting.codeMargin.left();
    save();
    auto table = m_doc->pieceTable(text);
    if (!table) return;
    // #f9f9f9
    //        m_painter.setBackground(QBrush(QColor(249, 249, 249)));
    setFont(codeFont());
    //    drawText(rect, code);
    auto rects = drawText(text);
    restore();
    int codeH = 0;
    for (auto rect : rects) {
      codeH += rect.height();
    }
    m_curY += codeH;
    const QRect bgRect = QRect(0, y, m_setting.contentMaxWidth(), codeH);
    //    fillRect(bgRect, QBrush(QColor(249, 249, 249)));
    if (!justCalculate()) {
      auto e = new Element::CodeBlock();
      //      e->code = code;
      QString copyBtnFilePath = ":icon/copy_32x32.png";
      QFile copyBtnFile(copyBtnFilePath);
      if (copyBtnFile.exists()) {
        QPixmap copyBtnImg(copyBtnFilePath);
        QRect copyBtnRect(
            QPoint(bgRect.width() - copyBtnImg.width(), bgRect.y()),
            copyBtnImg.size());
        e->rect = copyBtnRect;
        m_doc->appendCodeBlock(e);
        drawPixmap(copyBtnRect, copyBtnImg);
      } else {
        qWarning() << "copy btn file not exist." << copyBtnFilePath;
      }
    }
  }

  void drawLatex(const String &latex, bool inlineLatex = false) {
    try {
      float textSize = m_setting.latexFontSize;
      auto render =
          tex::LaTeX::parse(latex.toStdString(), m_setting.contentMaxWidth(),
                            textSize, textSize / 3.f, 0xff424242);
      if (justCalculate()) {
      } else {
        tex::Graphics2D_qt g2(m_painter);
        auto x = inlineLatex
                     ? m_curX
                     : (m_setting.contentMaxWidth() - render->getWidth()) / 2;
        render->draw(g2, x, m_curY);
      }
      m_curX += render->getWidth();
      delete render;
    } catch (const std::exception &ex) {
      qDebug() << "ERROR" << ex.what();
      drawText("Render LaTeX fail: " + latex);
    }
  }

  void drawEnter() {
    if (justCalculate()) return;
    save();
    QString enterStr = QString("↲");
    m_painter->setPen(Qt::blue);
    auto rect = textRect(enterStr);
    drawText(rect, enterStr);
    restore();
  }

 private:
  friend class Render;
  Render *q;

  QPainter *m_painter{};
  RenderSetting m_setting;
  int m_curX{};
  int m_curY{};
  int m_lastMaxHeight{};
  int m_lastMaxWidth{};
  QString m_filePath;
  QMap<QString, QString> m_cacheLatexImage;
  bool m_justCalculate{};
  QStack<QFont> m_fonts;
  QFont m_font;
  TexRender *m_texRender{};
  EditorDocument *m_doc{};
};
Render::Render(QString filePath, EditorDocument *doc, RenderSetting setting)
    : d(new RenderPrivate(this)) {
  d->m_painter = nullptr;
  d->m_filePath = filePath;
  d->m_justCalculate = false;
  d->m_doc = doc;
  d->m_setting = setting;
  d->m_lastMaxWidth = d->m_setting.maxWidth;
  d->reset(nullptr);
  auto paths = QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
  QString tmpPath;
  if (paths.isEmpty()) {
    tmpPath = QDir::homePath();
  } else {
    tmpPath = paths.first();
  }
  QString mathFontPath = tmpPath + "/XITSMath-Regular.otf";
  QString clmPath = tmpPath + "/XITSMath-Regular.clm";
  auto copy = [](QString src, QString dst) {
    auto dir = QFileInfo(dst).absoluteDir();
    if (!dir.exists()) {
      qDebug() << "mkdir" << dir.absolutePath();
      dir.mkdir(dir.absolutePath());
    }
    if (QFile::exists(dst)) {
      // 计算缓存文件的md5
      QFile oldFile(dst);
      if (!oldFile.open(QIODevice::ReadOnly)) {
        QFile::moveToTrash(dst);
      } else {
        auto cachedFileMd5 = QCryptographicHash::hash(oldFile.readAll(),
                                                      QCryptographicHash::Md5);
        QFile newFile(src);
        newFile.open(QIODevice::ReadOnly);
        auto newFileMd5 = QCryptographicHash::hash(newFile.readAll(),
                                                   QCryptographicHash::Md5);
        if (cachedFileMd5 == newFileMd5) {
          qDebug() << "hit cache math font:" << dst;
          return;
        }
        qDebug() << "rewrite cached file: " << dst;
      }
    }
    bool ok = QFile::copy(src, dst);
    if (!ok) {
      qDebug() << "copy" << src << "to" << dst << "fail";
    } else {
      qDebug() << "copy" << src << "to" << dst << "success";
    }
  };
  copy(":/font/XITSMath-Regular.otf", mathFontPath);
  copy(":/font/XITSMath-Regular.clm", clmPath);
  d->m_texRender = new TexRender(mathFontPath, clmPath);
}
Render::~Render() { delete d; }

void Render::visit(Header *node) {
  std::array<int, 6> fontSize = {36, 28, 24, 20, 16, 14};
  d->save();
  auto font = d->curFont();
  font.setPixelSize(fontSize[node->level() - 1]);
  font.setBold(true);
  d->setFont(font);
  d->moveToNewLine();
  for (auto it : node->children()) {
    it->accept(this);
  }
  d->restore();
}

void Render::visit(Text *node) { d->drawText(node); }

void Render::visit(ItalicText *node) {
  d->save();
  QFont font = d->curFont();
  font.setItalic(true);
  d->setFont(font);
  d->drawText(node->text());
  d->restore();
}

void Render::visit(BoldText *node) {
  d->save();
  QFont font = d->curFont();
  font.setBold(true);
  d->setFont(font);
  d->drawText(node->text());
  d->restore();
}

void Render::visit(ItalicBoldText *node) {
  d->save();
  QFont font = d->curFont();
  font.setItalic(true);
  font.setBold(true);
  d->setFont(font);
  d->drawText(node->text());
  d->restore();
}

void Render::visit(StrickoutText *node) {
  d->save();
  QFont font = d->curFont();
  font.setStrikeOut(true);
  d->setFont(font);
  d->drawText(node->text());
  d->restore();
}

void Render::visit(Image *node) {
  d->moveToNewLine();
  QString imgPath = d->m_doc->text2str(node->src());
  QDir dir(imgPath);
  if (dir.isRelative()) {
    QDir basePath(d->m_filePath);
    basePath.cdUp();
    imgPath = QString("%1/%2").arg(basePath.absolutePath(), imgPath);
  }
  QFile file(imgPath);
  if (!file.exists()) {
    qWarning() << "image not exist." << imgPath;
    return;
  }
  QImage image(imgPath);
  int imageMaxWidth = qMin(1080, d->m_setting.contentMaxWidth());
  int imgWidth = image.width();
  while (imgWidth > imageMaxWidth) {
    imgWidth /= 2;
  }
  image = image.scaledToWidth(imgWidth);
  QRect rect(QPoint(d->m_curX, d->m_curY), image.size());
  d->drawImage(rect, image);
  d->m_lastMaxHeight = rect.height();
  if (d->justCalculate()) {
    return;
  }
  auto img = new Element::Image();
  img->path = imgPath;
  img->rect = rect;
  d->m_doc->appendImage(img);
  if (!imgPath.endsWith(".gif")) {
    return;
  }
  QString playIconPath = ":/icon/play_64x64.png";
  QFile playIconFile(playIconPath);
  if (!playIconFile.exists()) {
    qWarning() << "play icon file not exist.";
    return;
  }
  QImage playImage(playIconPath);
  // 计算播放图标所在位置
  // 播放图标放在中心位置
  int x = (rect.width() - playImage.width()) / 2;
  int y = (rect.height() - playImage.height()) / 2;
  QRect playIconRect(QPoint(rect.x() + x, rect.y() + y), playImage.size());
  d->drawImage(playIconRect, playImage);
}

void Render::visit(Link *node) {
  d->save();
  d->setPen(Qt::blue);
  auto font = d->curFont();
  font.setUnderline(true);
  d->setFont(font);
  auto rects = d->drawText(node->content());
  if (d->justCalculate()) {
  } else {
    auto link = new Element::Link();
    link->text = d->m_doc->text2str(node->content());
    link->url = d->m_doc->text2str(node->href());
    link->rects = rects;
    d->m_doc->appendLink(link);
  }
  d->restore();
}

void Render::visit(CodeBlock *node) {
  if (!node) {
    DEBUG << "node is nullptr";
  }
  d->save();
  d->setFont(d->codeFont());
  for (auto child : node->children()) {
    child->accept(this);
  }
  d->restore();
}

void Render::visit(InlineCode *node) {
  d->save();
  // #f9f9f9
  //        m_painter.setBackground(QBrush(QColor(249, 249, 249)));
  d->setFont(d->codeFont());
  node->code()->accept(this);
#if 1
  QString code = d->m_doc->text2str(node->code());
  if (d->currentLineCanDrawText(code)) {
    auto rect = d->textRect(code);
    d->fillRect(rect, QBrush(QColor(249, 249, 249)));
    d->drawText(rect, code);
    d->m_curX += rect.width();
    d->m_lastMaxHeight = qMax(d->m_lastMaxHeight, rect.height());
  } else {
    d->drawCodeBlock(code);
  }
#endif
  d->restore();
}

void Render::visit(LatexBlock *node) {
  d->moveToNewLine();
  QString latex;
  for (auto child : node->children()) {
    if (child->type() == NodeType::text) {
      latex += d->m_doc->text2str((Text *)child);
    } else if (child->type() == NodeType::lf) {
      latex += "\n";
    }
  }
  d->drawLatex(latex);
}

void Render::visit(InlineLatex *node) {
  auto latex = d->m_doc->text2str(node->code());
  d->drawLatex(latex, true);
}

void Render::visit(Paragraph *node) {
  if (node->children().empty()) return;
  d->save();
  d->moveToNewLine();
  for (auto it : node->children()) {
    it->accept(this);
  }
  d->restore();
}

void Render::visit(CheckboxList *node) {
  for (const auto &item : node->children()) {
    d->moveToNewLine();
    d->m_curX += d->m_setting.checkboxMargin.left();
    item->accept(this);
  }
}

void Render::visit(CheckboxItem *node) {
  d->save();
  auto font = d->curFont();
  // 计算高度偏移
  auto h1 = d->textHeight();
  d->save();
  if (node->isChecked()) {
    QPixmap image(":icon/checkbox-selected_64x64.png");
    const QRect rect = QRect(QPoint(d->m_curX, d->m_curY), QSize(h1, h1));
    d->drawPixmap(rect, image);
  } else {
    QPixmap image(":icon/checkbox-unselected_64x64.png");
    const QRect rect = QRect(QPoint(d->m_curX, d->m_curY), QSize(h1, h1));
    d->drawPixmap(rect, image);
  }
  d->m_curX += h1 + 10;
  d->restore();
  font = d->curFont();
  font.setStrikeOut(node->isChecked());
  d->setFont(font);
  for (const auto &item : node->children()) {
    item->accept(this);
  }
  d->restore();
}

void Render::visit(UnorderedList *node) {
  for (const auto &item : node->children()) {
    d->moveToNewLine();
    d->m_curX += d->m_setting.listMargin.left();
    if (!d->justCalculate()) {
      d->save();
      auto h = d->textHeight();
      d->m_painter->setBrush(QBrush(Qt::black));
      auto size = 5;
      auto y = d->m_curY + (h - size) / 2 + 2;
      d->m_painter->drawEllipse(d->m_curX, y, size, size);
      d->restore();
    }
    d->m_curX += 15;
    item->accept(this);
  }
}

void Render::visit(UnorderedListItem *node) {
  for (const auto &item : node->children()) {
    item->accept(this);
  }
}

void Render::visit(OrderedList *node) {
  int i = 0;
  for (const auto &item : node->children()) {
    i++;
    d->moveToNewLine();
    d->m_curX += d->m_setting.listMargin.left();
    QString numStr = QString("%1.  ").arg(i);
    d->drawText(numStr);
    item->accept(this);
  }
}

void Render::visit(OrderedListItem *node) {
  for (const auto &item : node->children()) {
    item->accept(this);
  }
}

void Render::visit(Hr *node) {}

void Render::visit(QuoteBlock *node) {
  d->moveToNewLine();
  int startY = d->m_curY;
  for (auto child : node->children()) {
    d->m_curX += d->m_setting.quoteMargin.left();
    child->accept(this);
    d->moveToNewLine();
  }
  int endY = d->m_curY;
  const QRect rect = QRect(2, startY, 5, endY - startY);
  // #eee
  d->fillRect(rect, QBrush(QColor(238, 238, 238)));
}

void Render::visit(Table *node) {}
void Render::visit(Lf *node) {
  d->drawEnter();
  d->moveToNewLine();
}
void Render::highlight(Cursor *cursor) {
  if (!cursor) return;
  if (!d->m_setting.highlightCurrentLine) return;
  for (int i = 0; i < d->m_doc->lineData().size(); ++i) {
    auto line = d->m_doc->lineData()[i];
    if (line->contains(*cursor)) {
      if (!line->cells().isEmpty()) {
        auto rect = line->cells().first().rect;
        d->m_painter->drawText(rect.x(), rect.y(), QString::number(i + 1));
      }
      for (const auto &cell : line->cells()) {
        d->drawRect(cell.rect);
      }
    }
  }
}
void Render::setJustCalculate(bool flag) { d->m_justCalculate = flag; }
void Render::reset(QPainter *painter) { d->reset(painter); }
int Render::realHeight() const { return d->realHeight(); }
int Render::realWidth() const { return d->realWidth(); }
