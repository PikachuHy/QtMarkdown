//
// Created by PikachuHy on 2021/11/5.
//

#include "Render.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QStack>
#include <QStandardPaths>

#include "Instruction.h"
#include "debug.h"
#include "latex.h"
#include "parser/Text.h"
#include "platform/qt/graphic_qt.h"
using namespace md::parser;
namespace md::render {
class TexRender {
 public:
  TexRender(const String &mathFontPath, const String &clmPath) {
    tex::FontSpec math{"xits", mathFontPath.toStdString(), clmPath.toStdString()};
    tex::LaTeX::init(math);
  }
  ~TexRender() { tex::LaTeX::release(); }
};
class TexRenderGuard {
 public:
  TexRenderGuard() {
    // 将字体文件复制到临时文件夹
    // 目前latex渲染用到到字体必须是用绝对路径
    // 这里可能是有BUG
    auto paths = QStandardPaths::standardLocations(QStandardPaths::CacheLocation);
    QString tmpPath;
    if (paths.isEmpty()) {
      tmpPath = QDir::homePath();
    } else {
      tmpPath = paths.first();
    }
    QString mathFontPath = tmpPath + "/XITSMath-Regular.otf";
    QString clmPath = tmpPath + "/XITSMath-Regular.clm";
    copy(":/font/XITSMath-Regular.otf", mathFontPath);
    copy(":/font/XITSMath-Regular.clm", clmPath);
    m_texRender = std::make_shared<TexRender>(mathFontPath, clmPath);
    Q_UNUSED(m_texRender);
  }

 private:
  static void copy(const QString &src, const QString &dst) {
    auto dir = QFileInfo(dst).absoluteDir();
    if (!dir.exists()) {
      DEBUG << "mkdir" << dir.absolutePath();
      dir.mkdir(dir.absolutePath());
    }
    if (QFile::exists(dst)) {
      // 计算缓存文件的md5
      QFile oldFile(dst);
      if (!oldFile.open(QIODevice::ReadOnly)) {
        QFile::moveToTrash(dst);
      } else {
        auto cachedFileMd5 = QCryptographicHash::hash(oldFile.readAll(), QCryptographicHash::Md5);
        QFile newFile(src);
        newFile.open(QIODevice::ReadOnly);
        auto newFileMd5 = QCryptographicHash::hash(newFile.readAll(), QCryptographicHash::Md5);
        if (cachedFileMd5 == newFileMd5) {
          DEBUG << "hit cache math font:" << dst;
          return;
        }
        DEBUG << "rewrite cached file: " << dst;
      }
    }
    bool ok = QFile::copy(src, dst);
    if (!ok) {
      DEBUG << "copy" << src << "to" << dst << "fail";
    } else {
      DEBUG << "copy" << src << "to" << dst << "success";
    }
  };
  sptr<TexRender> m_texRender;
};
struct RenderSetting {
  bool highlightCurrentLine = false;
  int lineSpacing = 10;
  int maxWidth = 800;
  int latexFontSize = 20;
  std::array<int, 6> headerFontSize = {36, 28, 24, 20, 16, 14};
  QMargins docMargin = QMargins(50, 20, 20, 20);
  QMargins codeMargin = QMargins(10, 20, 20, 10);
  QMargins listMargin = QMargins(15, 20, 20, 10);
  QMargins checkboxMargin = QMargins(15, 20, 20, 10);
  QMargins quoteMargin = QMargins(10, 20, 20, 10);
  [[nodiscard]] int contentMaxWidth() const { return maxWidth - docMargin.left() - docMargin.right(); }
};
class RenderPrivate
    : public MultipleVisitor<Header, Text, ItalicText, BoldText, ItalicBoldText, StrickoutText, Image, Link, CodeBlock,
                             InlineCode, Paragraph, CheckboxList, CheckboxItem, UnorderedList, UnorderedListItem,
                             OrderedList, OrderedListItem, LatexBlock, InlineLatex, Hr, QuoteBlock, Table, Lf> {
 public:
  explicit RenderPrivate(Node *node, DocPtr doc) : m_instructionGroup(node), m_doc(doc) {
    Q_ASSERT(doc != nullptr);
    m_config.font.setPixelSize(16);
    m_config.pen = Qt::black;
    m_configs.push(m_config);
  }
  void visit(Header *node) override {
    Q_ASSERT(node != nullptr);
    save();
    auto font = curFont();
    font.setPixelSize(m_setting.headerFontSize[node->level() - 1]);
    font.setBold(true);
    setFont(font);
    moveToNewLine();
    newLogicalLine();
    drawHeaderPrefix(node->level());
    for (auto it : node->children()) {
      it->accept(this);
    }
    restore();
  }
  void visit(Paragraph *node) override {
    Q_ASSERT(node != nullptr);
    save();
    auto font = curFont();
    setFont(font);
    moveToNewLine();
    newLogicalLine();
    for (auto it : node->children()) {
      it->accept(this);
    }
    restore();
  }
  void visit(Text *node) override {
    Q_ASSERT(node != nullptr);
    SizeType totalOffset = 0;
    for (auto item : *node) {
      SizeType startIndex = 0;
      auto str = item.toString(m_doc);
      while (!currentLineCanDrawText(str.mid(startIndex))) {
        auto count = countOfThisLineCanDraw(str.mid(startIndex));
        if (count == 0) {
          moveToNewLine();
          continue;
        }
        PieceTableItem newItem{item.bufferType, item.offset + startIndex, count};
        m_instructionGroup.appendLogicalItem(newLogicalItem(node, startIndex, count, true));
        drawTextInCurrentLine(newItem);
        startIndex += count;
        totalOffset += count;
        moveToNewLine();
      }
      auto lastLength = item.length - startIndex;
      if (lastLength > 0) {
        PieceTableItem newItem{item.bufferType, item.offset + startIndex, lastLength};
        m_instructionGroup.appendLogicalItem(newLogicalItem(node, startIndex, lastLength, false));
        drawTextInCurrentLine(newItem);
        totalOffset += lastLength;
      }
    }
  }
  void visit(Link *node) override {
    Q_ASSERT(node != nullptr);
    save();
    setPen(Qt::blue);
    auto font = curFont();
    font.setUnderline(true);
    setFont(font);
    node->content()->accept(this);
    restore();
  }
  void visit(InlineCode *node) override {
    Q_ASSERT(node != nullptr);
    save();
    setFont(codeFont());
    auto row = m_instructionGroup.countOfVisualLine();
    auto col = m_instructionGroup.countOfVisualItem(row - 1);
    if (currentLineCanDrawText(node->code()->toString(m_doc))) {
      node->code()->accept(this);
      auto rect = m_instructionGroup.visualItemAt(row - 1, col)->config().rect;
      m_config.rect = QRect(rect.x() - 5, rect.y() - 5, rect.width() + 10, rect.height() + 10);
      m_config.brush = QBrush(QColor(249, 249, 249));
      m_instructionGroup.insertVisualItem(row - 1, col, new FillRectInstruction(m_config, false));
    }
    restore();
  }
  void visit(CodeBlock *node) override {
    Q_ASSERT(node != nullptr);
    save();
    moveToNewLine();
    newLogicalLine();
    auto x = m_curX;
    auto y = m_curY;
    setFont(codeFont());
    for (auto child : node->children()) {
      child->accept(this);
    }
    auto w = m_setting.contentMaxWidth() + 10;
    auto h = m_instructionGroup.height() + 10;
    m_config.rect = QRect(x - 5, y - 5, w, h);
    m_config.brush = QBrush(QColor(249, 249, 249));
    m_instructionGroup.insertNewVisualLineAt(0, new FillRectInstruction(m_config, false));
    restore();
  }
  void visit(InlineLatex *node) override {
    Q_ASSERT(node != nullptr);
    save();
    auto latex = node->code()->toString(m_doc);
    try {
      float textSize = m_setting.latexFontSize;
      auto render =
          tex::LaTeX::parse(latex.toStdString(), m_setting.contentMaxWidth(), textSize, textSize / 3.f, 0xff424242);
      QRect rect(m_curX, m_curY, render->getWidth(), render->getHeight());
      m_curX += render->getWidth();
      drawLatex(rect, textSize, latex);
      delete render;
    } catch (const std::exception &ex) {
      DEBUG << "ERROR" << ex.what();
      DEBUG << "Render LaTeX fail:" << latex;
    }
    restore();
  }
  void visit(LatexBlock *node) override {
    Q_ASSERT(node != nullptr);
    save();
    moveToNewLine();
    newLogicalLine();
    auto latex = node->toString(m_doc);
    try {
      float textSize = m_setting.latexFontSize;
      auto render =
          tex::LaTeX::parse(latex.toStdString(), m_setting.contentMaxWidth(), textSize, textSize / 3.f, 0xff424242);
      QRect rect((m_setting.contentMaxWidth() - render->getWidth()) / 2, m_curY, m_setting.contentMaxWidth(),
                 render->getHeight());
      m_curX += render->getWidth();
      drawLatex(rect, textSize, latex);
      delete render;
    } catch (const std::exception &ex) {
      DEBUG << "ERROR" << ex.what();
      DEBUG << "Render LaTeX fail:" << latex;
    }
    restore();
  }
  void visit(ItalicText *node) override {
    Q_ASSERT(node != nullptr);
    save();
    QFont font = curFont();
    font.setItalic(true);
    setFont(font);
    node->text()->accept(this);
    restore();
  }
  void visit(BoldText *node) override {
    Q_ASSERT(node != nullptr);
    save();
    QFont font = curFont();
    font.setBold(true);
    setFont(font);
    node->text()->accept(this);
    restore();
  }
  void visit(ItalicBoldText *node) override {
    Q_ASSERT(node != nullptr);
    save();
    QFont font = curFont();
    font.setItalic(true);
    font.setBold(true);
    setFont(font);
    node->text()->accept(this);
    restore();
  }
  void visit(StrickoutText *node) override {
    Q_ASSERT(node != nullptr);
    save();
    QFont font = curFont();
    font.setStrikeOut(true);
    setFont(font);
    node->text()->accept(this);
    restore();
  }
  void visit(Image *node) override {
    Q_ASSERT(node != nullptr);
    moveToNewLine();
    newLogicalLine();
    QString imgPath = node->src()->toString(m_doc);
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
    drawImage(rect, imgPath);
    m_lastMaxHeight = rect.height();
  }
  void visit(CheckboxList *node) override {
    Q_ASSERT(node != nullptr);
    for (const auto &item : node->children()) {
      moveToNewLine();
      newLogicalLine();
      m_curX += m_setting.checkboxMargin.left();
      item->accept(this);
    }
  }
  void visit(CheckboxItem *node) override {
    Q_ASSERT(node != nullptr);
    save();
    auto font = curFont();
    // 计算高度偏移
    auto h1 = textHeight();
    save();
    if (node->isChecked()) {
      QString imagePath = ":icon/checkbox-selected_64x64.png";
      const QRect rect = QRect(QPoint(m_curX, m_curY), QSize(h1, h1));
      drawImage(rect, imagePath);
    } else {
      QString imagePath = ":icon/checkbox-unselected_64x64.png";
      const QRect rect = QRect(QPoint(m_curX, m_curY), QSize(h1, h1));
      drawImage(rect, imagePath);
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
  void visit(UnorderedList *node) override {
    Q_ASSERT(node != nullptr);
    for (const auto &item : node->children()) {
      moveToNewLine();
      newLogicalLine();
      m_curX += m_setting.listMargin.left();
      auto h = textHeight();
      auto size = 5;
      auto y = m_curY + (h - size) / 2 + 2;
      QRect rect(m_curX, y, size, size);
      m_config.rect = rect;
      m_config.brush = Qt::black;
      auto instruction = new EllipseInstruction(m_config);
      m_instructionGroup.appendVisualItem(instruction);
      m_curX += 15;
      item->accept(this);
    }
  }
  void visit(UnorderedListItem *node) override {
    Q_ASSERT(node != nullptr);
    for (const auto &item : node->children()) {
      item->accept(this);
    }
  }
  void visit(OrderedList *node) override {
    Q_ASSERT(node != nullptr);
    int i = 0;
    for (const auto &item : node->children()) {
      i++;
      moveToNewLine();
      newLogicalLine();
      m_curX += m_setting.listMargin.left();
      QString numStr = QString("%1.  ").arg(i);
      m_instructionGroup.appendLogicalItem(new StaticTextCell(numStr, curFont(), Point(m_curX, m_curY), true, false));
      drawTextInCurrentLine(numStr);
      item->accept(this);
    }
  }
  void visit(OrderedListItem *node) override {
    Q_ASSERT(node != nullptr);
    for (const auto &item : node->children()) {
      item->accept(this);
    }
  }
  void visit(Hr *node) override { Q_ASSERT(node != nullptr); }
  void visit(QuoteBlock *node) override {
    Q_ASSERT(node != nullptr);
    moveToNewLine();
    newLogicalLine();
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
  void visit(Table *node) override { Q_ASSERT(node != nullptr); }
  void visit(Lf *node) override {
    Q_ASSERT(node != nullptr);
    save();
    QString enterStr = QString("↲");
    auto font = curFont();
    font.setPixelSize(12);
    setFont(font);
    setPen(Qt::blue);
    auto rect = textRect(enterStr);
    drawText(rect, enterStr);
    moveToNewLine();
    restore();
  }
  [[nodiscard]] Block renderRet() const { return m_instructionGroup; }

 private:
  void drawHeaderPrefix(int level) {
    save();
    QString prefix = QString("H%1").arg(level);
    auto font = curFont();
    font.setPixelSize(font.pixelSize() - 4);
    setFont(font);
    auto rect = textRect(prefix);
    rect.setX(m_curX - rect.width() - 1);
    m_config.rect = rect;
    m_config.font = font;
    m_instructionGroup.appendVisualItem(new StaticTextInstruction(prefix, m_config));
    restore();
  }

 private:
  // 画笔相关到操作
  void save() { m_configs.push(m_config); }
  void restore() {
    Q_ASSERT(!m_configs.isEmpty());
    m_config = m_configs.pop();
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
  [[nodiscard]] QFont curFont() const { return m_config.font; }
  void setFont(const QFont &font) { m_config.font = font; }
  void setPen(const QColor &color) { m_config.pen = color; }
  void moveToNewLine() {
    m_curY += m_lastMaxHeight;
    m_curX = m_setting.docMargin.left();
    m_instructionGroup.newVisualLine();
  }
  void newLogicalLine() { m_instructionGroup.newLogicalLine(); }
  LogicalItem newLogicalItem(Text *node, SizeType offset, SizeType length, bool eol) {
    bool bol = m_instructionGroup.visualLines().back().empty();
    Point pos(m_curX, m_curY);
    auto item = new TextCell(node, offset, length, curFont(), pos, bol, eol);
    return item;
  }
  // 辅助到绘制方法
  void drawTextInCurrentLine(const PieceTableItem &item) {
    QRect rect = textRect(item.toString(m_doc));
    drawText(rect, item);
    m_curX += rect.width();
    m_lastMaxHeight = qMax(m_lastMaxHeight, rect.height());
  }
  void drawTextInCurrentLine(const String &str) {
    QRect rect = textRect(str);
    drawText(rect, str);
    m_curX += rect.width();
    m_lastMaxHeight = qMax(m_lastMaxHeight, rect.height());
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

  QFontMetrics fontMetrics() {
    QFontMetrics fm(curFont());
    return fm;
  }

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

 private:
  // 直接添加绘制指令
  void drawText(const QRect &r, const PieceTableItem &item) {
    m_config.rect = r;
    m_instructionGroup.appendVisualItem(new TextInstruction(item, m_config));
  }
  void drawText(const QRect &r, const String &str) {
    m_config.rect = r;
    m_instructionGroup.appendVisualItem(new StaticTextInstruction(str, m_config));
  }

  void drawImage(const QRect &r, const String &imgPath) {
    m_config.rect = r;
    m_instructionGroup.appendVisualItem(new StaticImageInstruction(imgPath, m_config));
  }
  void fillRect(const QRect &r, const QBrush &b) {
    m_config.rect = r;
    m_config.brush = b;
    m_instructionGroup.appendVisualItem(new FillRectInstruction(m_config));
  }
  void drawLatex(const QRect &r, int size, const String &latex) {
    m_config.rect = r;
    m_config.font.setPixelSize(size);
    m_instructionGroup.appendVisualItem(new LatexInstruction(latex, m_config));
  }

 private:
  InstructionPainterConfig m_config;
  QStack<InstructionPainterConfig> m_configs;
  DocPtr m_doc;
  Block m_instructionGroup;

  int m_curX{};
  int m_curY{};
  int m_lastMaxHeight{};
  int m_lastMaxWidth{};
  RenderSetting m_setting;
};
Block Render::render(Node *node, DocPtr doc) {
  Q_ASSERT(node != nullptr);
  Q_ASSERT(doc != nullptr);
  static TexRenderGuard texRenderGuard;
  RenderPrivate render(node, doc);
  node->accept(&render);
  return render.renderRet();
}
}  // namespace md::render
