//
// Created by PikachuHy on 2021/11/5.
//

#include "Render.h"

#include <QCryptographicHash>
#include <QDir>
#include <QStack>
#include <QStandardPaths>
#include <vector>

#include "Instruction.h"
#include "PaintPass.h"
#include "PaintRecord.h"
#include "StringUtil.h"
#include "FontMetricsProvider.h"
#include "debug.h"
#include "microtex.h"
#include "parser/Text.h"
#include "graphic_qt.h"
using namespace md::parser;
namespace md::render {
class TexRender {
 public:
  TexRender() {
    //microtex::InitFontSenseAuto init;
    //microtex::MicroTeX::init(init);
    microtex::PlatformFactory::registerFactory("qt", std::make_unique<microtex::PlatformFactory_qt>());
    microtex::PlatformFactory::activate("qt");

    microtex::MicroTeX::setRenderGlyphUsePath(true);
  }
  ~TexRender() { microtex::MicroTeX::release(); }
};
class TexRenderGuard {
 public:
  TexRenderGuard() {
    m_texRender = std::make_shared<TexRender>();
    Q_UNUSED(m_texRender);
  }

 private:
  sptr<TexRender> m_texRender;
};
namespace {  // internal linkage -- static storage duration
QtFontMetricsProvider s_defaultFontMetrics;
}
// Render内部配置
struct LayoutConfig {
  Font font;
  Color pen;
};
class LayoutPass
    : public NodeVisitor {
 public:
  explicit LayoutPass(Node *node, sptr<RenderSetting> setting, DocPtr doc,
                      IFontMetricsProvider* fontMetrics = nullptr)
      : m_block(node), m_setting(setting), m_doc(doc),
        m_fontMetrics(fontMetrics ? fontMetrics : &s_defaultFontMetrics),
        m_hasGui(fontMetrics == nullptr) {
    Q_ASSERT(doc != nullptr);
    Q_ASSERT(m_fontMetrics != nullptr);
    m_config.font.setPixelSize(16 + 2);
    m_config.pen = Qt::black;
    m_configs.push_back(m_config);
  }
  void visit(Header *node) override {
    Q_ASSERT(node != nullptr);
    save();
    auto font = curFont();
    font.setPixelSize(m_setting->headerFontSize[node->level() - 1]);
    font.setBold(true);
    setFont(font);
    beginBlock();
    drawHeaderPrefix(node->level());
    for (auto& it : node->children()) {
      it->accept(this);
    }
    endBlock();
    restore();
  }
  void visit(Paragraph *node) override {
    Q_ASSERT(node != nullptr);
    save();
    auto font = curFont();
    setFont(font);
    beginBlock();
    m_curX += curFont().pixelSize() * m_setting->paragraphIntent;
    for (auto& it : node->children()) {
      it->accept(this);
    }
    endBlock();
    restore();
  }
  void drawEnglishString(Text *node, const String &str, RenderString s, SizeType &startIndex, SizeType &drawCount) {
    String enStr = str.mid(startIndex, s.length - drawCount);
    auto enStrList = enStr.split(" ");
    for (int i = 0; i < enStrList.size(); ++i) {
      auto enSubStr = enStrList[i];
      if (currentLineCanDrawText(enSubStr)) {
        auto count = enSubStr.size();
        if (i < enStrList.size() - 1) {
          // 加一个空格
          count++;
        }
        drawText(node, str, s, startIndex, count);
        startIndex += count;
        continue;
      }

      if (textSize(enSubStr).width() + m_setting->docMargin.left() < m_setting->contentMaxWidth()) {
        moveToNewLine();
        // 抵消后面都i++
        i--;
        continue;
      }
      auto count = countOfThisLineCanDraw(str.mid(startIndex, s.length - drawCount));
      DEBUG << count;
      if (count == 0) {
        moveToNewLine();
        continue;
      }
      drawText(node, str, s, startIndex, count);
      startIndex += count;
      // 画不下，就强制加一个连字符
      auto hyphenPos = Point(m_curX, m_curY);
      auto hyphenSize = textSize("-");
      m_paintRecords.push_back(PaintRecord::staticText(String("-"), hyphenPos, hyphenSize, Qt::black, curFont()));
      moveToNewLine();
    }
  }
  void drawRenderString(Text *node, const String &str, RenderString s) {
    SizeType startIndex = s.offset;
    SizeType drawCount = 0;
    while (!currentLineCanDrawText(str.mid(startIndex, s.length - drawCount))) {
      // 如果是英文的话，先按空格分割，然后如果还画不下，去下一行
      // 如果一行都画不下，就暴力分割
      if (s.type == RenderString::English) {
        drawEnglishString(node, str, s, startIndex, drawCount);
      } else {
        auto count = countOfThisLineCanDraw(str.mid(startIndex, s.length - drawCount));
        if (count == 0) {
          moveToNewLine();
          continue;
        }
        if (count + 1 < s.length) {
          auto ts = str.mid(startIndex + count, 1);
          // 如果是中文的逗号或者句号结尾，就少画一个中文字，把符号画到下一行。
          if (ts == "，" || ts == "。") {
            count--;
          }
        }
        drawText(node, str, s, startIndex, count);
        startIndex += count;
        drawCount += count;
        moveToNewLine();
      }
    }
    auto lastLength = s.length + s.offset - startIndex;
    if (lastLength > 0) {
      drawText(node, str, s, startIndex, lastLength);
    }
  }
  void visit(Text *node) override {
    Q_ASSERT(node != nullptr);
    auto str = node->toString(m_doc);
    // 将字符串按中文，英文，emoji切分
    auto stringList = StringUtil::split(str);
    for (auto s : stringList) {
      drawRenderString(node, str, s);
    }
  }
  void visit(Link *node) override {
    Q_ASSERT(node != nullptr);
    save();
    setPen(Qt::blue);
    auto font = curFont();
    font.setUnderline(true);
    setFont(font);
    auto startIndex = m_block.m_logicalLines.back().m_cells.size();
    node->content()->accept(this);
    for (auto i = startIndex; i < m_block.m_logicalLines.back().m_cells.size(); ++i) {
      auto cell = m_block.m_logicalLines.back().m_cells[i];
      m_block.appendElement({node, cell->m_pos, cell->m_size});
    }
    restore();
  }
  void visit(InlineCode *node) override {
    Q_ASSERT(node != nullptr);
    save();
    setFont(codeFont());
    auto codeStr = node->code()->toString(m_doc);
    int x = m_curX;
    int y = m_curY;
    if (currentLineCanDrawText(codeStr)) {
      auto size = textSize(codeStr);
      m_paintRecords.push_back(PaintRecord::fillRect(
          Point(x - 2, y - 2), Size(size.width() + 4, size.height() + 4), QColor(249, 249, 249)));
      node->code()->accept(this);
    }
    restore();
  }
  void visit(CodeBlock *node) override {
    Q_ASSERT(node != nullptr);
    save();
    setFont(codeFont());
    m_rewriteFont = false;
    beginBlock();
    auto x = m_curX;
    auto y = m_curY;
    auto w = m_setting->contentMaxWidth();

    int lineH = textHeight();
    int estimatedH = node->size() * lineH + (node->size() - 1) * m_setting->lineSpacing;
    m_paintRecords.push_back(PaintRecord::fillRect(
        Point(x - 3, y - 3), Size(w + 6, estimatedH + 6), QColor(249, 249, 249)));

    for (int i = 0; i < node->size(); ++i) {
      if (i > 0) beginLogicalLine();
      node->childAt(i)->accept(this);
      if (i + 1 < node->size()) endLogicalLine();
    }
    endBlock();

    if (m_hasGui) {
      QString copyBtnFilePath = ":icon/copy_32x32.png";
      QFile copyBtnFile(copyBtnFilePath);
      ASSERT(copyBtnFile.exists());
      QPixmap copyBtnImg(copyBtnFilePath);
      Point pos(x + w - copyBtnImg.width(), y);
      m_paintRecords.push_back(PaintRecord::staticImage(copyBtnFilePath, pos, copyBtnImg.size()));
      m_block.appendElement({node, pos, copyBtnImg.size()});
    }
    restore();
  }
  void visit(InlineLatex *node) override {
    Q_ASSERT(node != nullptr);
    save();
    auto latex = node->code()->toString(m_doc);
    try {
      float textSize = m_setting->latexFontSize;
      auto render = std::unique_ptr<microtex::Render>(
          microtex::MicroTeX::parse(latex.toStdString(), m_setting->contentMaxWidth(), textSize,
                                    textSize / 3.f, 0xff424242));
      const QPoint &point = Point(m_curX, m_curY);
      const QSize &size = Size(render->getWidth() + 2, render->getHeight());
      auto cell = std::make_unique<InlineLatexCell>(point, size);
      auto* rawCell = cell.get();
      appendVisualCell(std::move(cell));
      m_paintRecords.push_back(PaintRecord::fromCell(rawCell, latex, textSize));
      m_curX += render->getWidth();
    } catch (const std::exception &ex) {
      DEBUG << "ERROR" << ex.what();
      DEBUG << "Render LaTeX fail:" << latex;
    }
    restore();
  }
  void visit(LatexBlock *node) override {
    Q_ASSERT(node != nullptr);
    save();
    beginBlock();
    auto latex = node->toString(m_doc);
    try {
      float textSize = m_setting->latexFontSize;
      auto render = std::unique_ptr<microtex::Render>(
          microtex::MicroTeX::parse(latex.toStdString(), m_setting->contentMaxWidth(), textSize,
                                    textSize / 3.f, 0xff424242));
      const QPoint &point = Point((m_setting->contentMaxWidth() - render->getWidth()) / 2, m_curY);
      const QSize &size = Size(m_setting->contentMaxWidth(), render->getHeight());
      auto cell = std::make_unique<InlineLatexCell>(point, size);
      auto* rawCell = cell.get();
      appendVisualCell(std::move(cell));
      m_paintRecords.push_back(PaintRecord::fromCell(rawCell, latex, textSize));
      m_curX += render->getWidth();
    } catch (const std::exception &ex) {
      DEBUG << "ERROR" << ex.what();
      DEBUG << "Render LaTeX fail:" << latex;
    }
    endBlock();
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
    beginVisualLine();
    QString imgPath = node->src()->toString(m_doc);
#if defined(Q_OS_ANDROID) || defined(Q_OS_UNIX)
    if (!imgPath.startsWith("/")) {
      for (const auto &resPath : m_setting->resPathList) {
        QString newImgPath = resPath + "/" + imgPath;
        if (QFile(newImgPath).exists()) {
          imgPath = newImgPath;
        }
      }
    }
#endif
    QFile file(imgPath);

    if (!file.exists()) {
      qWarning() << "image not exist." << imgPath;
      return;
    }
    QImage image(imgPath);
    int imageMaxWidth = qMin(1080, m_setting->contentMaxWidth());
    int imgWidth = image.width();
    while (imgWidth > imageMaxWidth) {
      imgWidth /= 2;
    }
    image = image.scaledToWidth(imgWidth);
    const QPoint &pos = Point(m_curX, m_curY);
    m_paintRecords.push_back(PaintRecord::image(imgPath, pos, image.size()));
    m_curY += image.height();
    m_block.appendElement({node, pos, image.size()});
    // TODO: 需要重新考虑图片
    m_block.m_logicalLines.back().m_lines.back().m_h = image.height();
    endVisualLine();
    if (!imgPath.endsWith(".gif")) {
      return;
    }
    // gif加一个播放的图标
    QString playIconPath = ":/icon/play_64x64.png";
    QFile playIconFile(playIconPath);
    ASSERT(playIconFile.exists());
    QImage playImage(playIconPath);
    // 计算播放图标所在位置
    // 播放图标放在中心位置
    int x = (image.width() - playImage.width()) / 2 + pos.x();
    int y = (image.height() - playImage.height()) / 2 + pos.y();
    m_paintRecords.push_back(PaintRecord::staticImage(playIconPath, Point(x, y), playImage.size()));
  }
  void visit(CheckboxList *node) override {
    Q_ASSERT(node != nullptr);
    beginBlock(false);
    for (const auto &item : node->children()) {
      beginLogicalLine(false);
      item->accept(this);
      endLogicalLine();
    }
    endBlock();
  }
  void visit(CheckboxItem *node) override {
    Q_ASSERT(node != nullptr);
    save();
    int oldX = m_curX;
    m_curX += m_setting->checkboxMargin.left();
    auto font = curFont();
    // 计算高度偏移
    auto h1 = textHeight();

    save();
    const QPoint &pos = Point(m_curX, m_curY);
    const QSize &size = Size(h1, h1);
    if (node->isChecked()) {
      QString imagePath = ":icon/checkbox-selected_64x64.png";
      m_paintRecords.push_back(PaintRecord::staticImage(imagePath, pos, size));

    } else {
      QString imagePath = ":icon/checkbox-unselected_64x64.png";
      m_paintRecords.push_back(PaintRecord::staticImage(imagePath, pos, size));
    }
    m_block.appendElement({node, pos, size});
    m_curX += h1 + 10;
    restore();
    font = curFont();
    font.setStrikeOut(node->isChecked());
    setFont(font);
    m_block.m_logicalLines.back().m_padding = m_curX - oldX;
    beginVisualLine();
    for (const auto &item : node->children()) {
      item->accept(this);
    }
    restore();
  }
  void visit(UnorderedList *node) override {
    Q_ASSERT(node != nullptr);
    beginBlock(false);
    for (const auto &item : node->children()) {
      beginLogicalLine(false);
      auto oldX = m_curX;
      m_curX += m_setting->listMargin.left();
      auto h = textHeight();
      auto size = 5;
      auto y = m_curY + (h - size) / 2 + 2;
      m_paintRecords.push_back(PaintRecord::ellipse(Point(m_curX, y), Size(size, size), Qt::black));
      m_curX += 15;
      m_block.m_logicalLines.back().m_padding = m_curX - oldX;
      beginVisualLine();
      item->accept(this);
      endLogicalLine();
    }
    endBlock();
  }
  void visit(UnorderedListItem *node) override {
    Q_ASSERT(node != nullptr);
    for (const auto &item : node->children()) {
      item->accept(this);
    }
  }
  void visit(OrderedList *node) override {
    Q_ASSERT(node != nullptr);
    beginBlock(false);
    int i = 0;
    for (const auto &item : node->children()) {
      i++;
      beginLogicalLine(false);
      auto oldX = m_curX;
      m_curX += m_setting->listMargin.left();
      QString numStr = QString("%1.  ").arg(i);
      const Size &size = textSize(numStr);
      const QPoint &pos = Point(m_curX, m_curY);
      m_paintRecords.push_back(PaintRecord::staticText(numStr, pos, size, Qt::black, curFont()));
      m_curX += size.width();
      m_block.m_logicalLines.back().m_padding = m_curX - oldX;
      beginVisualLine();
      item->accept(this);
      endLogicalLine();
    }
    endBlock();
  }
  void visit(OrderedListItem *node) override {
    Q_ASSERT(node != nullptr);
    for (const auto &item : node->children()) {
      item->accept(this);
    }
  }
  void visit(Hr *node) override {
    Q_ASSERT(node != nullptr);
    save();
    beginBlock();
    int lineY = m_curY + m_setting->lineSpacing;
    m_paintRecords.push_back(
        PaintRecord::fillRect(Point(m_setting->docMargin.left(), lineY),
                              Size(m_setting->contentMaxWidth() - m_setting->docMargin.left(), 1),
                              QColor(200, 200, 200)));
    m_curY = lineY + m_setting->lineSpacing;
    endBlock();
    restore();
  }
  void visit(QuoteBlock *node) override {
    Q_ASSERT(node != nullptr);
    beginBlock(false);
    int startY = m_curY;
    for (auto& child : node->children()) {
      beginLogicalLine();
      child->accept(this);
      endLogicalLine();
    }
    int endY = m_curY;
    if (endY > startY) {
      endY -= m_setting->lineSpacing;
    }
    QColor bgColor(238, 238, 238);
    Point pos(m_setting->docMargin.left() - m_setting->quoteMargin.left(), startY);
    m_paintRecords.push_back(PaintRecord::fillRect(pos, Size(5, endY - startY), bgColor));
    endBlock();
  }
  void visit(Table *node) override {
    Q_ASSERT(node != nullptr);
    save();
    beginBlock();
    auto font = curFont();
    setFont(font);
    // Render header
    if (!node->header().isEmpty()) {
      String headerLine;
      for (const auto& cell : node->header()) {
        headerLine += cell + QStringLiteral(" | ");
      }
      auto size = textSize(headerLine);
      m_paintRecords.push_back(
          PaintRecord::staticText(headerLine, Point(m_curX, m_curY), size, curPen(), curFont()));
      m_curY += size.height() + m_setting->lineSpacing;
    }
    // Render content rows
    for (const auto& row : node->content()) {
      String rowLine;
      for (const auto& cell : row) {
        rowLine += cell + QStringLiteral(" | ");
      }
      auto size = textSize(rowLine);
      m_paintRecords.push_back(
          PaintRecord::staticText(rowLine, Point(m_curX, m_curY), size, curPen(), curFont()));
      m_curY += size.height() + m_setting->lineSpacing;
    }
    endBlock();
    restore();
  }
  void visit(Lf *node) override {
    Q_ASSERT(node != nullptr);
    save();
#ifdef Q_OS_ANDROID
#else
    QString enterStr = QString("↲");
    auto font = curFont();
    font.setPixelSize(12);
    auto size = textSize(enterStr);
    m_paintRecords.push_back(PaintRecord::staticText(enterStr, Point(m_curX, m_curY), size, Qt::blue, font));
#endif
    endLogicalLine();
    beginLogicalLine();
    restore();
  }
  [[nodiscard]] std::pair<Block, PaintRecordList> execute() {
    return {std::move(m_block), std::move(m_paintRecords)};
  }

 private:
  void drawHeaderPrefix(int level) {
    save();
    QString prefix = QString("H%1").arg(level);
    auto font = curFont();
    font.setPixelSize(font.pixelSize() - 4);
    setFont(font);
    auto size = textSize(prefix);
    auto x = m_curX - size.width() - 1;
    m_paintRecords.push_back(PaintRecord::staticText(prefix, Point(x, m_curY), size, Qt::black, font));
    restore();
  }

  void drawText(Text *node, const String &str, RenderString &s, SizeType offset, SizeType length) {
    save();
    if (s.type == RenderString::English) {
      if (m_rewriteFont) {
        auto font = curFont();
        font.setFamily(m_setting->enTextFont);
        setFont(font);
      }
    } else if (s.type == RenderString::Chinese) {
      auto font = curFont();
      font.setFamily(m_setting->zhTextFont);
      setFont(font);
    }
    const QString &text = str.mid(offset, length);
    const Size &size = textSize(text);
    auto cell = std::make_unique<TextCell>(node, offset, length, Point(m_curX, m_curY), size, curPen(), curFont(), m_fontMetrics);
    auto* rawCell = cell.get();
    appendVisualCell(std::move(cell));
    m_paintRecords.push_back(PaintRecord::fromCell(rawCell));
    restore();
    m_curX += size.width();
  }
  void appendVisualCell(std::unique_ptr<Cell> cell) {
    ASSERT(!m_block.m_logicalLines.empty());
    auto &logicalLine = m_block.m_logicalLines.back();
    ASSERT(!logicalLine.m_lines.empty());
    auto &line = logicalLine.m_lines.back();
    Cell* rawCell = cell.get();
    logicalLine.m_cells.push_back(rawCell);
    line.m_cells.push_back(std::move(cell));
  }
  void beginBlock(bool initNewLogicalLine = true) {
    if (initNewLogicalLine) {
      beginLogicalLine();
    }
  }
  void endBlock() { endLogicalLine(); }
  void beginLogicalLine(bool initNewVisualLine = true) {
    LogicalLine logicalLine;
    m_curX = m_setting->docMargin.left();
    logicalLine.m_pos = Point(m_curX, m_curY);
    auto size = m_fontMetrics->size(curFont(), "龙");
    logicalLine.m_h = size.height();
    m_block.m_logicalLines.push_back(std::move(logicalLine));
    if (initNewVisualLine) {
      beginVisualLine();
    }
  }
  void endLogicalLine(bool endLastVisualLine = true) {
    if (m_block.m_logicalLines.empty()) {
      DEBUG << "empty logical line";
      return;
    }
    ASSERT(!m_block.m_logicalLines.empty());
    auto &logicalLine = m_block.m_logicalLines.back();
    ASSERT(!logicalLine.m_lines.empty());
    if (endLastVisualLine) {
      endVisualLine();
    }
    int h = 0;
    for (const auto &line : logicalLine.m_lines) {
      h += line.m_h;
      h += m_setting->lineSpacing;
    }
    logicalLine.m_h = h;
  }
  void beginVisualLine() {
    ASSERT(!m_block.m_logicalLines.empty());
    auto &line = m_block.m_logicalLines.back();
    auto size = m_fontMetrics->size(curFont(), "龙");
    line.m_lines.push_back(VisualLine(Point(m_curX, m_curY), size.height()));
  }
  void endVisualLine() {
    // 当视觉行结束时，需要调整每个Cell的y
    // 让中英文的底部是一样的
    ASSERT(!m_block.m_logicalLines.empty());
    auto &logicalLine = m_block.m_logicalLines.back();
    ASSERT(!logicalLine.m_lines.empty());
    auto &line = logicalLine.m_lines.back();
    int maxH = line.m_h;
    for (const auto& cell : line.m_cells) {
      auto h = cell->height();
      maxH = std::max(maxH, h);
    }
    line.m_h = maxH;
    for (auto& cell : line.m_cells) {
      auto y = cell->m_pos.y();
      // 这里的对齐还是不好
      // textSize算出来的大小也不完全是能过包围住文字的
      auto delta = maxH - cell->height();
      cell->m_pos.setY(y + delta / 2);
    }
    m_curY += maxH + m_setting->lineSpacing;
  }
  void moveToNewLine() {
    endVisualLine();
    m_curX = m_setting->docMargin.left();
    beginVisualLine();
  }

 private:
  // 画笔相关到操作
  void save() { m_configs.push_back(m_config); }
  void restore() {
    Q_ASSERT(!m_configs.empty());
    m_config = m_configs.back();
    m_configs.pop_back();
  }
  QFont codeFont() {
    auto font = curFont();
#ifdef Q_OS_WIN
    font.setFamily("Cascadia Code");
#elif defined(Q_OS_MACOS)
    font.setFamily("Menlo");
#else
    font.setFamily("monospace");
#endif
    font.setPixelSize(20);
    return font;
  }
  [[nodiscard]] QFont curFont() const { return m_config.font; }
  [[nodiscard]] Color curPen() const { return m_config.pen; }
  void setFont(const QFont &font) { m_config.font = font; }
  void setPen(const QColor &color) { m_config.pen = color; }
  // 辅助到绘制方法
  Size textSize(const QString &text) { return m_fontMetrics->size(curFont(), text); }

  int textWidth(const QString &text) {
    return m_fontMetrics->horizontalAdvance(curFont(), text);
  }

  int charWidth(const QChar &ch) {
    return m_fontMetrics->horizontalAdvance(curFont(), String(ch));
  }

  int textHeight() {
    return m_fontMetrics->height(curFont());
  }

  bool currentLineCanDrawText(const QString &text) {
    auto needWidth = textWidth(text);
    if (m_curX + needWidth < m_setting->contentMaxWidth()) {
      return true;
    } else {
      return false;
    }
  }

  int countOfThisLineCanDraw(const QString &text) {
    // 计算这一行可以画多少个字符
    auto ch_w = charWidth(text.at(0));
    int left_w = m_setting->contentMaxWidth() - m_curX;
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
  std::vector<LayoutConfig> m_configs;
  LayoutConfig m_config;
  DocPtr m_doc;
  Block m_block;

  int m_curX = 0;
  int m_curY = 0;
  sptr<RenderSetting> m_setting;

  bool m_rewriteFont = true;
  PaintRecordList m_paintRecords;
  IFontMetricsProvider* m_fontMetrics;
  bool m_hasGui;
};
int VisualLine::height() const { return m_h; }
SizeType VisualLine::length() const {
  auto l = 0;
  for (const auto& cell : m_cells) {
    l += cell->length();
  }
  return l;
}
std::pair<Cell *, int> VisualLine::cellAtX(int x, DocPtr doc) const {
  if (m_cells.empty()) {
    return {nullptr, 0};
  }
  if (x <= m_cells.front()->m_pos.x()) {
    return {m_cells.front().get(), 0};
  }
  auto totalX = m_cells.front()->m_pos.x();
  for (const auto& cell : m_cells) {
    if (totalX <= x && x <= totalX + cell->width()) {
      // 再确定offset
      auto textCell = dynamic_cast<TextCell *>(cell.get());
      if (!textCell) break;
      for (int j = 0; j < textCell->length(); ++j) {
        auto w = textCell->width(j + 1, doc);
        if (totalX <= x && x <= totalX + w) {
          auto delta = j;
          // 按中间划分
          if (totalX <= x && x <= totalX + w / 2) {
            delta = j;
          } else {
            delta = j + 1;
          }
          return {cell.get(), delta};
        }
      }
    }
    totalX += cell->width();
  }
  const auto& cell = m_cells.back();
  return {cell.get(), cell->length()};
}
int VisualLine::width() const {
  if (m_cells.empty()) return 0;
  const auto& cell = m_cells.back();
  auto w = cell->m_pos.x() + cell->m_size.width() - m_pos.x();
  return w;
}
int LogicalLine::height() const { return m_h; }
std::pair<Point, int> LogicalLine::cursorAt(SizeType offset, DocPtr doc) const {
  if (m_cells.empty()) {
    return {Point(m_pos.x() + m_padding, m_pos.y()), m_h};
  }
  auto totalOffset = 0;
  for (auto cell : m_cells) {
    if (totalOffset <= offset && offset < totalOffset + cell->length()) {
      // 计算还剩下的偏移量
      auto w = cell->width(offset - totalOffset, doc);
      auto pos = Point(cell->m_pos.x() + w, cell->m_pos.y());
      return {pos, cell->m_size.height()};
    }
    totalOffset += cell->length();
  }
  if (offset == totalOffset) {
    auto cell = m_cells.back();
    auto pos = Point(cell->m_pos.x() + cell->width(), cell->m_pos.y());
    return {pos, cell->m_size.height()};
  }
  DEBUG << m_cells.size() << offset << totalOffset;
  ASSERT(false && "cursor not in cell");
}
SizeType LogicalLine::length() const {
  SizeType length = 0;
  for (auto cell : m_cells) {
    length += cell->length();
  }
  return length;
}

bool LogicalLine::hasTextAt(SizeType offset) const {
  auto totalOffset = 0;
  for (auto cell : m_cells) {
    if (totalOffset <= offset && offset <= totalOffset + cell->length()) {
      auto leftOffset = offset - totalOffset;
      return dynamic_cast<TextCell *>(cell) != nullptr;
    }
    totalOffset += cell->length();
  }
  return false;
}
std::pair<parser::Text *, int> LogicalLine::textAt(SizeType offset) const {
  auto totalOffset = 0;
  for (auto cell : m_cells) {
    if (totalOffset <= offset && offset <= totalOffset + cell->length()) {
      auto leftOffset = offset - totalOffset;
      auto textCell = dynamic_cast<TextCell *>(cell);
      if (!textCell) continue;
      return {textCell->m_text, leftOffset + textCell->m_offset};
    }
    totalOffset += cell->length();
  }
  DEBUG << m_cells.size() << offset << totalOffset;
  ASSERT(false && "text not in cell");
}
String LogicalLine::left(SizeType length, DocPtr doc) const {
  ASSERT(length >= 0 && length <= this->length());
  if (length == 0) return String();
  String s;
  for (auto cell : m_cells) {
    auto textCell = dynamic_cast<TextCell *>(cell);
    if (!textCell) continue;
    s += textCell->m_text->toString(doc);
    if (s.size() >= length) return s.left(length);
  }
  DEBUG << this->length() << length << s;
  ASSERT(false && "length");
}
bool LogicalLine::canMoveDown(SizeType offset, DocPtr doc) const {
  ASSERT(offset >= 0 && offset <= this->length());
  auto totalOffset = 0;
  for (int i = 0; i < m_lines.size(); ++i) {
    auto &line = m_lines[i];
    if (totalOffset <= offset && offset < totalOffset + line.length()) {
      return i + 1 < m_lines.size();
    }
    totalOffset += line.length();
  }
  if (totalOffset == offset) {
    return false;
  }
  DEBUG << this->length() << offset;
  ASSERT(false && "no cell in line");
}
bool LogicalLine::canMoveUp(SizeType offset, DocPtr doc) const {
  ASSERT(offset >= 0 && offset <= this->length());
  if (m_cells.empty()) return false;
  auto totalOffset = 0;
  for (int i = 0; i < m_lines.size(); ++i) {
    auto &line = m_lines[i];
    if (totalOffset <= offset && offset < totalOffset + line.length()) {
      return i > 0;
    }
    totalOffset += line.length();
  }
  if (totalOffset == offset) {
    return m_lines.size() > 1;
  }
  DEBUG << this->length() << offset;
  ASSERT(false && "no cell in line");
}
SizeType LogicalLine::moveDown(SizeType offset, int x, DocPtr doc) const {
  ASSERT(offset >= 0 && offset <= this->length());
  auto totalOffset = 0;
  for (int visualLineNo = 0; visualLineNo < m_lines.size(); ++visualLineNo) {
    if (totalOffset <= offset && offset < totalOffset + m_lines[visualLineNo].length()) {
      ASSERT(visualLineNo + 1 < m_lines.size());
      auto &line = m_lines[visualLineNo + 1];
      auto [cell, delta] = line.cellAtX(x, doc);
      return this->totalOffset(cell, delta);
    }
    totalOffset += m_lines[visualLineNo].length();
  }
  return this->length();
}
SizeType LogicalLine::moveUp(SizeType offset, int x, DocPtr doc) const {
  ASSERT(offset >= 0 && offset <= this->length());
  auto totalOffset = 0;
  for (int i = 0; i < m_lines.size(); ++i) {
    if (totalOffset <= offset && offset < totalOffset + m_lines[i].length()) {
      ASSERT(i - 1 >= 0);
      auto &line = m_lines[i - 1];
      auto [cell, delta] = line.cellAtX(x, doc);
      return this->totalOffset(cell, delta);
    }
    totalOffset += m_lines[i].length();
  }
  if (totalOffset == offset) {
    ASSERT(!m_cells.empty());
    ASSERT(m_lines.size() > 1);
    auto [cell, delta] = m_lines[m_lines.size() - 2].cellAtX(x, doc);
    return this->totalOffset(cell, delta);
  }
  ASSERT(false && "no offset in line");
}
SizeType LogicalLine::totalOffset(Cell *cell, SizeType delta) const {
  if (cell == nullptr) {
    return 0;
  }
  SizeType offset = 0;
  for (auto it : m_cells) {
    if (it == cell) {
      return offset + delta;
    }
    offset += it->length();
  }
  ASSERT(false && "no cell in line");
}
SizeType LogicalLine::moveToX(int x, DocPtr doc, bool lastLine) const {
  auto &line = lastLine ? m_lines.back() : m_lines.front();
  auto [cell, delta] = line.cellAtX(x, doc);
  return totalOffset(cell, delta);
}
int LogicalLine::width() const {
  int w = 0;
  for (const auto &line : m_lines) {
    w = std::max(w, line.width());
  }
  return w + m_padding;
}
SizeType LogicalLine::moveToBol(SizeType offset, DocPtr doc) const {
  ASSERT(offset >= 0 && offset <= this->length());
  if (m_cells.empty()) return 0;
  SizeType totalOffset = 0;
  for (const auto &line : m_lines) {
    if (totalOffset <= offset && offset < totalOffset + line.length()) {
      return totalOffset;
    }
    totalOffset += line.length();
  }
  if (totalOffset == offset) {
    ASSERT(!m_lines.empty());
    return totalOffset - m_lines.back().length();
  }
  ASSERT(false && "no offset in line");
}
std::pair<SizeType, int> LogicalLine::moveToEol(SizeType offset, DocPtr) const {
  ASSERT(offset >= 0 && offset <= this->length());
  if (m_cells.empty()) return {0, m_pos.x()};
  SizeType totalOffset = 0;
  for (const auto &line : m_lines) {
    if (totalOffset <= offset && offset < totalOffset + line.length()) {
      return {totalOffset + line.length(), line.m_pos.x() + line.width()};
    }
    totalOffset += line.length();
  }
  if (totalOffset == offset) {
    ASSERT(!m_lines.empty());
    const auto &line = m_lines.back();
    return {totalOffset, line.m_pos.x() + line.width()};
  }
  ASSERT(false && "no offset in line");
}
SizeType LogicalLine::offsetAt(Point pos, DocPtr doc, int lineSpacing) const {
  int y = m_pos.y();
  // -1表示没有算出offset
  SizeType offset = -1;
  for (const auto &line : m_lines) {
    // 这里要考虑lineSpacing
    if (y - lineSpacing / 2 <= pos.y() && pos.y() <= y + line.height() + lineSpacing / 2) {
      auto [cell, delta] = line.cellAtX(pos.x(), doc);
      offset = this->totalOffset(cell, delta);
      break;
    }
    y += line.height();
  }
  if (offset == -1 && pos.y() <= this->height() + m_pos.y()) {
    ASSERT(!m_lines.empty());
    const auto &line = m_lines.back();
    auto [cell, delta] = line.cellAtX(pos.x(), doc);
    offset = this->totalOffset(cell, delta);
  }
  if (offset != -1) {
    // 修正emoji offset
    if (offset > 0 && offset + 1 < this->length()) {
      auto s = this->left(offset + 1, doc);
      auto ch = s[offset - 1].unicode();
      if (ch == 0xd83d || ch == 0xd83c) {
        offset++;
      }
    }
    return offset;
  }
  DEBUG << this->left(this->length(), doc);
  DEBUG << pos << m_pos << this->width() << this->height();
  ASSERT(false && "no pos in line");
}
int LogicalLine::visualLineAt(SizeType offset, DocPtr doc) const {
  ASSERT(offset >= 0 && offset <= this->length());
  SizeType totalOffset = 0;
  for (int i = 0; i < m_lines.size(); ++i) {
    if (totalOffset <= offset && offset <= totalOffset + m_lines[i].length()) {
      return i;
    }
    totalOffset += m_lines[i].length();
  }
  return m_lines.size() - 1;
}
VisualLine &LogicalLine::visualLineAt(int index) {
  ASSERT(index >= 0 && index < m_lines.size());
  return m_lines[index];
}
const VisualLine &LogicalLine::visualLineAt(int index) const {
  ASSERT(index >= 0 && index < m_lines.size());
  return m_lines[index];
}
bool LogicalLine::isBol(SizeType offset, const DocPtr doc) const {
  if (offset == 0) return true;
  SizeType totalOffset = 0;
  for (const auto &line : m_lines) {
    if (totalOffset == offset) return true;
    totalOffset += line.length();
  }
  return false;
}
int Block::height() const {
  int h = 0;
  for (const auto &line : m_logicalLines) {
    h += line.height();
  }
  return h;
}
const LogicalLine &Block::logicalLineAt(SizeType index) const {
  ASSERT(index >= 0 && index < m_logicalLines.size());
  return m_logicalLines[index];
}
int Block::width() const {
  int w = 0;
  for (const auto &line : m_logicalLines) {
    w = std::max(line.width(), w);
  }
  return w;
}
Block Render::render(Node *node, sptr<RenderSetting> setting, DocPtr doc,
                     IFontMetricsProvider* fontMetrics) {
  Q_ASSERT(node != nullptr);
  Q_ASSERT(doc != nullptr);
  static TexRenderGuard texRenderGuard;
  LayoutPass render(node, setting, doc, fontMetrics);
  node->accept(&render);
  auto [block, paintRecords] = render.execute();
  PaintPass paintPass(paintRecords);
  block.setInstructions(paintPass.execute());
  return std::move(block);
}
}  // namespace md::render
