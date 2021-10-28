//
// Created by pikachu on 5/22/2021.
//

#include "Render.h"
#include "Cursor.h"
#include "latex.h"
#include "platform/qt/graphic_qt.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QStandardPaths>
#include <utility>
struct Cell {
  QFont font;
  QRect rect;
  QString text;
};
class LineData {
public:
  void appendCell(const Cell &cell) { m_cells.append(cell); }
  bool contains(Cursor &cursor) {
    auto pos = cursor.pos();
    return std::any_of(m_cells.begin(), m_cells.end(), [pos](const auto &cell) {
      auto rect = cell.rect;
      // 只需要计算y是不是在两个值之间
      auto y1 = rect.y();
      auto y2 = rect.y() + rect.height();
      if (y1 <= pos.y() && pos.y() < y2) {
        return true;
      }
      return false;
    });
  }
  QList<Cell> &cells() { return m_cells; }
  std::pair<qsizetype, qsizetype> lastCoord() {
    auto cellNo = m_cells.size() - 1;
    auto offset = m_cells[cellNo].text.size();
    return {cellNo, offset};
  }

private:
  QList<Cell> m_cells;
};
class TexRender {
public:
  TexRender(QString mathFontPath, QString clmPath) {
    tex::FontSpec math{"xits", mathFontPath.toStdString(),
                       clmPath.toStdString()};
    tex::LaTeX::init(math);
  }
  ~TexRender() { tex::LaTeX::release(); }
};
Render::Render(QString filePath, RenderSetting setting)
    : m_painter(nullptr), m_filePath(std::move(filePath)),
      m_justCalculate(false), m_setting(setting) {
  m_lastMaxWidth = m_setting.maxWidth;
  reset(m_painter);
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
          return;
        }
        qDebug() << "rewrite cached file: " << dst;
      }
    }
    bool ok = QFile::copy(src, dst);
    if (!ok) {
      qDebug() << "copy" << src << "to" << dst << "fail";
    }
  };
  copy(":/font/XITSMath-Regular.otf", mathFontPath);
  copy(":/font/XITSMath-Regular.clm", clmPath);
  m_texRender = new TexRender(mathFontPath, clmPath);
}
Render::~Render() { delete m_texRender; }

void Render::reset(QPainter *painter) {
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
  qDeleteAll(m_lineData);
  m_lineData.clear();
}

void Render::save() {
  if (justCalculate()) {
    m_fonts.push(curFont());
  } else {
    m_painter->save();
  }
}

void Render::restore() {
  if (justCalculate()) {
    m_font = m_fonts.pop();
  } else {
    m_painter->restore();
  }
}

void Render::setFont(const QFont &font) {
  if (justCalculate()) {
    m_font = font;
  } else {
    m_painter->setFont(font);
  }
}

void Render::setPen(const QColor &color) {
  if (justCalculate()) {
    // do nothing
  } else {
    m_painter->setPen(color);
  }
}

QFontMetrics Render::fontMetrics() {
  if (justCalculate()) {
    QFontMetrics fm(curFont());
    return fm;
  } else {
    return m_painter->fontMetrics();
  }
}

void Render::drawText(const QRectF &r, const QString &text,
                      const QTextOption &o) {
  if (justCalculate()) {
    // do nothing
  } else {
    m_painter->drawText(r, text, o);
  }
}

void Render::drawImage(const QRect &r, const QImage &image) {
  if (justCalculate()) {
    // do nothing
  } else {
    m_painter->drawImage(r, image);
  }
}

void Render::drawPixmap(const QRect &r, const QPixmap &pm) {
  if (justCalculate()) {
    // do nothing
  } else {
    m_painter->drawPixmap(r, pm);
  }
}

void Render::fillRect(const QRect &rect, const QBrush &b) {
  if (justCalculate()) {
    // do nothing
  } else {
    m_painter->fillRect(rect, b);
  }
}

QRect Render::textRect(const QString &text) {
  QFontMetrics metrics = fontMetrics();
  const QRect rect(m_curX, m_curY, m_setting.contentMaxWidth(), 0);
  QRect textBoundingRect = metrics.boundingRect(rect, Qt::TextWordWrap, text);
  return textBoundingRect;
}

int Render::textWidth(const QString &text) {
  QFontMetrics metrics = fontMetrics();
  int w = metrics.horizontalAdvance(text);
  return w;
}

int Render::charWidth(const QChar &ch) {
  QFontMetrics metrics = fontMetrics();
  int w = metrics.horizontalAdvance(ch);
  return w;
}

int Render::textHeight() { return fontMetrics().height(); }

bool Render::currentLineCanDrawText(const QString &text) {
  auto needWidth = textWidth(text);
  if (m_curX + needWidth < m_setting.contentMaxWidth()) {
    return true;
  } else {
    return false;
  }
}

int Render::countOfThisLineCanDraw(const QString &text) {
  // 计算这一行可以画多少个字符
  auto ch_w = charWidth(text.at(0));
  int left_w = m_setting.contentMaxWidth() - m_curX;
  int may_ch_count = left_w / ch_w - 1;
  // 可能根本画不了
  if (may_ch_count <= 0)
    return 0;
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

QList<QRect> Render::drawText(QString text) {
  if (text == "\r")
    return {};
  if (text.isEmpty())
    return {};
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
    m_lineData.back()->appendCell(cell);
    rects.append(rect);
    text = text.right(text.size() - count);
    moveToNewLine();
  }
  if (!text.isEmpty()) {
    auto rect = drawTextInCurrentLine(text);
    rects.append(rect);
    Cell cell{curFont(), rect, text};
    m_lineData.back()->appendCell(cell);
  }
  return rects;
}

QRect Render::drawTextInCurrentLine(const QString &text, bool adjustX,
                                    bool adjustY) {
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

void Render::moveToNewLine() {
  m_curY += m_lastMaxHeight;
  m_curX = m_setting.docMargin.left();
  m_curY += m_setting.lineSpacing;
  m_lastMaxHeight = 0;
  auto line = new LineData();
  m_lineData.append(line);
}

void Render::visit(Header *node) {
  std::array<int, 6> fontSize = {36, 28, 24, 20, 16, 14};
  save();
  auto font = curFont();
  font.setPixelSize(fontSize[node->level() - 1]);
  font.setBold(true);
  setFont(font);
  moveToNewLine();
  for (auto it : node->children()) {
    it->accept(this);
  }
  restore();
}

void Render::visit(Text *node) { drawText(node->str()); }

void Render::visit(ItalicText *node) {
  save();
  QFont font = curFont();
  font.setItalic(true);
  setFont(font);
  drawText(node->str());
  restore();
}

void Render::visit(BoldText *node) {
  save();
  QFont font = curFont();
  font.setBold(true);
  setFont(font);
  drawText(node->str());
  restore();
}

void Render::visit(ItalicBoldText *node) {
  save();
  QFont font = curFont();
  font.setItalic(true);
  font.setBold(true);
  setFont(font);
  drawText(node->str());
  restore();
}

void Render::visit(Image *node) {
  moveToNewLine();
  QString imgPath = node->src()->str();
  QDir dir(imgPath);
  if (dir.isRelative()) {
    QDir basePath(m_filePath);
    basePath.cdUp();
    imgPath = QString("%1/%2").arg(basePath.absolutePath(), imgPath);
  }
  QFile file(imgPath);
  if (!file.exists()) {
    qWarning() << "image not exist." << imgPath;
    return;
  }
  QImage image(imgPath);
  int imageMaxWidth = qMin(1080, m_setting.contentMaxWidth());
  int imgWidth = image.width();
  while (imgWidth > imageMaxWidth) {
    imgWidth /= 2;
  }
  image = image.scaledToWidth(imgWidth);
  QRect rect(QPoint(m_curX, m_curY), image.size());
  drawImage(rect, image);
  m_lastMaxHeight = rect.height();
  if (justCalculate()) {
    return;
  }
  auto img = new Element::Image();
  img->path = imgPath;
  img->rect = rect;
  m_images.append(img);
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
  drawImage(playIconRect, playImage);
}

void Render::visit(Link *node) {
  save();
  setPen(Qt::blue);
  auto font = curFont();
  font.setUnderline(true);
  setFont(font);
  auto rects = drawText(node->content()->str());
  if (justCalculate()) {

  } else {
    auto link = new Element::Link();
    link->text = node->content()->str();
    link->url = node->href()->str();
    link->rects = rects;
    m_links.append(link);
  }
  restore();
}

void Render::drawCodeBlock(const String &code) {
  moveToNewLine();
  auto y = m_curY;
  m_curY += m_setting.codeMargin.top();
  m_curX += m_setting.codeMargin.left();
  save();
  // #f9f9f9
  //        m_painter.setBackground(QBrush(QColor(249, 249, 249)));
  setFont(codeFont());
  auto rect = textRect(code);
  const QRect bgRect = QRect(0, y, m_setting.contentMaxWidth(), rect.height());
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
      QRect copyBtnRect(QPoint(bgRect.width() - copyBtnImg.width(), bgRect.y()),
                        copyBtnImg.size());
      e->rect = copyBtnRect;
      m_codes.append(e);
      drawPixmap(copyBtnRect, copyBtnImg);
    } else {
      qWarning() << "copy btn file not exist." << copyBtnFilePath;
    }
  }
}

void Render::visit(CodeBlock *node) {
  if (node && node->code()) {
    auto code = node->code()->str();
    drawCodeBlock(code);
  }
}

void Render::visit(InlineCode *node) {
  save();
  // #f9f9f9
  //        m_painter.setBackground(QBrush(QColor(249, 249, 249)));
  setFont(codeFont());
  QString code = node->code() ? node->code()->str() : " ";
  if (currentLineCanDrawText(code)) {
    auto rect = textRect(code);
    fillRect(rect, QBrush(QColor(249, 249, 249)));
    drawText(rect, code);
    m_curX += rect.width();
    m_lastMaxHeight = qMax(m_lastMaxHeight, rect.height());
  } else {
    drawCodeBlock(code);
  }
  restore();
}

void Render::visit(LatexBlock *node) {
  moveToNewLine();
  auto latex = node->code()->str();
  try {
    float textSize = m_setting.latexFontSize;
    auto render =
        tex::LaTeX::parse(latex.toStdString(), m_setting.contentMaxWidth(),
                          textSize, textSize / 3.f, 0xff424242);
    if (justCalculate()) {

    } else {
      tex::Graphics2D_qt g2(m_painter);
      auto x = (m_setting.contentMaxWidth() - render->getWidth()) / 2;
      render->draw(g2, x, m_curY);
    }
    m_curY += render->getHeight();
    delete render;
  } catch (const std::exception &ex) {
    qDebug() << "ERROR" << ex.what();
    drawText("Render LaTeX fail: " + latex);
  }
}

void Render::visit(InlineLatex *node) {
  QString key = node->code()->str();
  QString imgFilename;
  if (m_cacheLatexImage.contains(key) &&
      QFile(m_cacheLatexImage[key]).exists()) {
    imgFilename = m_cacheLatexImage[key];
  } else {
    // return;
    QTemporaryFile tmpFile;
    if (tmpFile.open()) {
      tmpFile.write(node->code()->str().toUtf8());
      tmpFile.close();
      QStringList args;
      imgFilename = tmpFile.fileName() + ".png";
      args << tmpFile.fileName() << imgFilename;
      //                qDebug() << args;
      QProcess p;
      p.start("latex2png.exe", args);
      auto ok = p.waitForFinished();
      if (!ok) {
        qDebug() << "LaTex.exe run fail";
        return;
      }
      if (!QFile(imgFilename).exists()) {
        qDebug() << "file not exist." << imgFilename;
        return;
      }
    } else {
      qDebug() << "tmp file open fail." << tmpFile.fileName();
      return;
    }
  }
  QPixmap image(imgFilename);
  // 如果画不下，硬画的话会卡死
  // 所以要换行
  if (m_curX + image.width() + 5 + 5 < m_setting.contentMaxWidth()) {
    m_curX += 5;
  } else {
    moveToNewLine();
  }
  const QRect rect = QRect(QPoint(m_curX, m_curY), image.size());
  m_curX += image.width();
  m_curX += 5;
  m_lastMaxHeight = qMax(m_lastMaxHeight, image.height());
  drawPixmap(rect, image);
}

void Render::visit(Paragraph *node) {
  if (node->children().empty())
    return;
  save();
  moveToNewLine();
  for (auto it : node->children()) {
    it->accept(this);
  }
  restore();
}

void Render::visit(CheckboxList *node) {
  for (const auto &item : node->children()) {
    moveToNewLine();
    m_curX += m_setting.checkboxMargin.left();
    item->accept(this);
  }
}

void Render::visit(CheckboxItem *node) {
  save();
  auto font = curFont();
  // 计算高度偏移
  auto h1 = textHeight();
  save();
  if (node->isChecked()) {
    QPixmap image(":icon/checkbox-selected_64x64.png");
    const QRect rect = QRect(QPoint(m_curX, m_curY), QSize(h1, h1));
    drawPixmap(rect, image);
  } else {
    QPixmap image(":icon/checkbox-unselected_64x64.png");
    const QRect rect = QRect(QPoint(m_curX, m_curY), QSize(h1, h1));
    drawPixmap(rect, image);
  }
  m_curX += h1 + 10;
  restore();
  font = curFont();
  font.setStrikeOut(node->isChecked());
  setFont(font);
  for (const auto &item : node->children()) {
    item->accept(this);
  }
  restore();
}

void Render::visit(UnorderedList *node) {
  for (const auto &item : node->children()) {
    moveToNewLine();
    m_curX += m_setting.listMargin.left();
    if (!justCalculate()) {
      save();
      auto h = textHeight();
      m_painter->setBrush(QBrush(Qt::black));
      auto size = 5;
      auto y = m_curY + (h - size) / 2 + 2;
      m_painter->drawEllipse(m_curX, y, size, size);
      restore();
    }
    m_curX += 15;
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
    moveToNewLine();
    m_curX += m_setting.listMargin.left();
    QString numStr = QString("%1.  ").arg(i);
    drawText(numStr);
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
  moveToNewLine();
  int startY = m_curY;
  for (auto child : node->children()) {
    m_curX += m_setting.quoteMargin.left();
    child->accept(this);
    moveToNewLine();
  }
  int endY = m_curY;
  const QRect rect = QRect(2, startY, 5, endY - startY);
  // #eee
  fillRect(rect, QBrush(QColor(238, 238, 238)));
}

void Render::visit(Table *node) {}

int Render::realHeight() const { return m_curY + m_lastMaxHeight + m_setting.docMargin.bottom(); }

int Render::realWidth() const { return m_lastMaxWidth; }

const QList<Element::Link *> &Render::links() { return m_links; }

const QList<Element::Image *> &Render::images() { return m_images; }

const QList<Element::CodeBlock *> &Render::codes() { return m_codes; }

bool Render::justCalculate() const { return m_justCalculate; }

void Render::setJustCalculate(bool flag) { m_justCalculate = flag; }

QFont Render::curFont() {
  if (justCalculate()) {
    return m_font;
  } else {
    return m_painter->font();
  }
}

void Render::setPainter(QPainter *painter) { m_painter = painter; }

QFont Render::codeFont() {
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
void Render::drawRect(const QRect &r, const QColor fb) {
  if (justCalculate()) {
    // do nothing
  } else {
    m_painter->save();
    m_painter->setPen(fb);
    m_painter->drawRect(r);
    m_painter->restore();
  }
}
void Render::highlight(Cursor *cursor) {
  if (!cursor)
    return;
  if (!m_setting.highlightCurrentLine)
    return;
  auto pos = cursor->pos();
  for (int i = 0; i < m_lineData.size(); ++i) {
    auto line = m_lineData[i];
    if (line->contains(*cursor)) {
      if (!line->cells().isEmpty()) {
        auto rect = line->cells().first().rect;
        m_painter->drawText(rect.x(), rect.y(), QString::number(i + 1));
      }
      for (const auto &cell : line->cells()) {
        drawRect(cell.rect);
      }
    }
  }
}
void Render::updateCursor(Cursor *cursor) {
  if (!cursor)
    return;
  auto coord = cursor->coord();
  auto cell = m_lineData[coord.lineNo]->cells()[coord.cellNo];
  if (cursor->offset() == 0) {
    cursor->moveTo(cell.rect.x(), cell.rect.y());
  } else {
    auto text = cell.text.left(cursor->offset());
    QFontMetrics fm(cell.font);
    auto w = fm.horizontalAdvance(text);
    cursor->moveTo(cell.rect.x() + w, cell.rect.y());
  }
  cursor->updateHeight(cell.rect.height());
}
void Render::moveCursorLeft(Cursor *cursor) {
  if (!cursor)
    return;
  auto coord = cursor->coord();
  if (coord.offset > 0) {
    coord.offset--;
    cursor->setCursorCoord(coord);
    return;
  }
  if (coord.cellNo > 0) {
    coord.cellNo--;
    coord.offset = m_lineData[coord.lineNo]->cells()[coord.cellNo].text.size();
    cursor->setCursorCoord(coord);
    return;
  }
  if (coord.lineNo > 0) {
    coord.lineNo--;
    coord.cellNo = m_lineData[coord.lineNo]->cells().size() - 1;
    coord.offset = m_lineData[coord.lineNo]->cells()[coord.cellNo].text.size();
    cursor->setCursorCoord(coord);
    return;
  }
  // 如果已经在第一行的行首，不用处理
}
void Render::moveCursorRight(Cursor *cursor) {
  if (!cursor)
    return;
  auto coord = cursor->coord();
  if (coord.offset <
      m_lineData[coord.lineNo]->cells()[coord.cellNo].text.size()) {
    coord.offset++;
    cursor->setCursorCoord(coord);
    return;
  }
  if (coord.cellNo < m_lineData[coord.lineNo]->cells().size() - 1) {
    coord.cellNo++;
    coord.offset = 0;
    cursor->setCursorCoord(coord);
    return;
  }
  if (coord.lineNo < m_lineData.size() - 1) {
    coord.lineNo++;
    coord.cellNo = 0;
    coord.offset = 0;
    cursor->setCursorCoord(coord);
    return;
  }
  // 如果已经在最后一行的最后一个位置，不用处理
}
void Render::moveCursorDown(Cursor *cursor) {
  if (!cursor)
    return;
  auto coord = cursor->coord();
  if (coord.lineNo < m_lineData.size() - 1) {
    // 先x不变，去下一行里找y
    coord.lineNo++;
    auto line = m_lineData[coord.lineNo];
    bool findCell = false;
    for (int i = 0; i < line->cells().size(); ++i) {
      auto cell = line->cells()[i];
      auto rect = cell.rect;
      bool hasFixPos = false;
      if (rect.x() <= cursor->x() && cursor->x() < rect.x() + rect.width()) {
        // 修正光标的x,y值
        auto newX = rect.x();
        QFontMetrics fm(cell.font);
        for (int j = 0; j < cell.text.size(); j++) {
          auto ch = cell.text[j];
          auto w = fm.horizontalAdvance(ch);
          if (newX <= cursor->x() && cursor->x() <= newX + w) {
            coord.cellNo = i;
            if (cursor->x() - newX > w / 2) {
              coord.offset = j + 1;
            } else {
              coord.offset = j;
            }
            cursor->setCursorCoord(coord);
            cursor->moveTo(newX + w - 1, rect.y());
            cursor->updateHeight(rect.height());
            hasFixPos = true;
            break;
          }
          newX += w;
        }
        if (!hasFixPos)
          continue;
        findCell = true;
        break;
      }
    }
    if (!findCell) {
      // 如果没找到cell，就取该行最后一个位置
      auto [cellNo, offset] = line->lastCoord();
      coord.cellNo = cellNo;
      coord.offset = offset;
      cursor->setCursorCoord(coord);
    }
    return;
  }
  // 如果已经在最后一行了，直接去到最后一行的最后一个位置
  auto line = m_lineData[coord.lineNo];
  coord.cellNo = line->cells().size() - 1;
  coord.offset = line->cells()[coord.cellNo].text.size();
  cursor->setCursorCoord(coord);
}
void Render::moveCursorUp(Cursor *cursor) {
  if (!cursor)
    return;
  auto coord = cursor->coord();
  if (coord.lineNo > 0) {
    // 先x不变，去下一行里找y
    coord.lineNo--;
    auto line = m_lineData[coord.lineNo];
    bool findCell = false;
    for (int i = 0; i < line->cells().size(); ++i) {
      auto cell = line->cells()[i];
      auto rect = cell.rect;
      if (rect.x() <= cursor->x() && cursor->x() <= rect.x() + rect.width()) {
        // 修正光标的x,y值
        auto newX = rect.x();
        QFontMetrics fm(cell.font);
        bool hasFixPos = false;
        for (int j = 0; j < cell.text.size(); j++) {
          auto ch = cell.text[j];
          auto w = fm.horizontalAdvance(ch);
          if (newX <= cursor->x() && cursor->x() <= newX + w) {
            coord.cellNo = i;
            if (cursor->x() - newX > w / 2) {
              coord.offset = j + 1;
            } else {
              coord.offset = j;
            }
            cursor->setCursorCoord(coord);
            cursor->moveTo(newX + w - 1, rect.y());
            cursor->updateHeight(rect.height());
            hasFixPos = true;
            break;
          }
          newX += w;
        }
        if (!hasFixPos)
          continue;
        findCell = true;
        break;
      }
    }
    if (!findCell) {
      // 如果没找到cell，就取该行最后一个位置
      auto [cellNo, offset] = line->lastCoord();
      coord.cellNo = cellNo;
      coord.offset = offset;
      cursor->setCursorCoord(coord);
    }
    return;
  }
  // 如果已经在第一行了，直接去到第一行的第一个位置
  coord.cellNo = 0;
  coord.offset = 0;
  cursor->setCursorCoord(coord);
}
void Render::fixCursorPos(Cursor *cursor) {
  if (!cursor)
    return;
  auto pos = cursor->pos();
  auto coord = cursor->coord();
  for (int i = 0; i < m_lineData.size(); ++i) {
    auto line = m_lineData[i];
    if (!line->contains(*cursor))
      continue;
    coord.lineNo = i;
    // 修正光标的x,y值
    bool hasFixCursor = false;
    for (int cellNo = 0; cellNo < line->cells().size(); ++cellNo) {
      auto cell = line->cells()[cellNo];
      auto newX = cell.rect.x();
      QFontMetrics fm(cell.font);
      for (int j = 0; j < cell.text.size(); j++) {
        auto ch = cell.text[j];
        auto w = fm.horizontalAdvance(ch);
        if (newX <= pos.x() && pos.x() < newX + w) {
          // 找到所在的文字后，移动到文字后面
          cursor->moveTo(newX + w - 1, cell.rect.y());
          cursor->updateHeight(cell.rect.height());
          hasFixCursor = true;
          coord.cellNo = cellNo;
          coord.offset = j + 1;
          break;
        }
        newX += w;
      }
    }
    if (!hasFixCursor) {
      // 如果没有修正，那么取最后一个矩形的坐标
      if (!line->cells().isEmpty()) {
        auto rect = line->cells().back().rect;
        cursor->moveTo(rect.x() + rect.width(), rect.y());
        cursor->updateHeight(rect.height());
        auto [cellNo, offset] = line->lastCoord();
        coord.cellNo = cellNo;
        coord.offset = offset;
      }
    }
    break;
  }
  cursor->setCursorCoord(coord);
}
