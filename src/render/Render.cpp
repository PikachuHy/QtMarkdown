//
// Created by PikachuHy on 2021/11/5.
//

#include "Render.h"

#include <vector>
#include <filesystem>

#include "Instruction.h"
#include "StringUtil.h"
#include "FontMetricsProvider.h"
#include "debug.h"
#include "microtex.h"
#include "parser/Text.h"
#include "core/IImageProvider.h"
#include "core/Utf8Util.h"
#include "DefaultFontMetrics.h"
using namespace md::parser;
namespace md::render {
// Render内部配置
struct LayoutConfig {
  Font font;
  Color pen;
};
class LayoutPass
    : public NodeVisitor {
 public:
  explicit LayoutPass(Node *node, sptr<RenderSetting> setting, const parser::IBufferProvider& doc,
                      IFontMetricsProvider* fontMetrics = nullptr,
                      editor::core::IImageProvider* imageProvider = nullptr)
      : m_block(), m_setting(setting), m_doc(doc),
        m_fontMetrics(fontMetrics ? fontMetrics : &g_defaultFontMetrics),
        m_hasGui(fontMetrics == nullptr),
        m_imageProvider(imageProvider) {
    ASSERT(m_fontMetrics != nullptr);
    m_config.font.pixelSize = 18;
    m_config.pen = Color::black();
    m_configs.push_back(m_config);
  }
  void visit(Header *node) override {
    ASSERT(node != nullptr);
    save();
    auto font = curFont();
    font.pixelSize = m_setting->headerFontSize[node->level() - 1];
    font.bold = true;
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
    ASSERT(node != nullptr);
    save();
    auto font = curFont();
    setFont(font);
    beginBlock();
    m_curX += curFont().pixelSize * m_setting->paragraphIntent;
    for (auto& it : node->children()) {
      it->accept(this);
    }
    endBlock();
    restore();
  }
  void drawEnglishString(Text *node, const String &str, RenderString s, SizeType &startIndex, SizeType &drawCount) {
    String enStr = str.mid(startIndex, s.length - drawCount);
    auto enStrList = enStr.split(' ');
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

      if (textSize(enSubStr).width + m_setting->docMargin.left < m_setting->contentMaxWidth()) {
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
      m_instructions.push_back(std::make_unique<StaticTextInstruction>(String("-"), hyphenPos, hyphenSize, Color::black(), curFont()));
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
        // 如果是中文的逗号或者句号结尾，就少画一个中文字，把符号画到下一行。
        if (startIndex + count < str.size()) {
          auto cp = md::codePointAt(str.toStdString(), startIndex + count);
          if (cp == 0xFF0C /* ， */ || cp == 0x3002 /* 。 */ || cp == 0x3001 /* 、 */) {
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
    ASSERT(node != nullptr);
    auto str = node->toString(m_doc);
    // 将字符串按中文，英文，emoji切分
    auto stringList = StringUtil::split(str);
    for (auto s : stringList) {
      drawRenderString(node, str, s);
    }
  }
  void visit(Link *node) override {
    ASSERT(node != nullptr);
    save();
    setPen(Color::blue());
    auto font = curFont();
    font.underline = true;
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
    ASSERT(node != nullptr);
    save();
    setFont(codeFont());
    auto codeStr = node->code()->toString(m_doc);
    int x = m_curX;
    int y = m_curY;
    if (currentLineCanDrawText(codeStr)) {
      auto size = textSize(codeStr);
      m_instructions.push_back(std::make_unique<FillRectInstruction>(
          Point(x - 2, y - 2), Size(size.width + 4, size.height + 4), Color(249, 249, 249)));
      node->code()->accept(this);
    }
    restore();
  }
  void visit(CodeBlock *node) override {
    ASSERT(node != nullptr);
    save();
    setFont(codeFont());
    m_rewriteFont = false;
    beginBlock();
    auto x = m_curX;
    auto y = m_curY;
    auto w = m_setting->contentMaxWidth();

    int lineH = textHeight();
    int estimatedH = node->size() * lineH + (node->size() - 1) * m_setting->lineSpacing;
    m_instructions.push_back(std::make_unique<FillRectInstruction>(
        Point(x - 3, y - 3), Size(w + 6, estimatedH + 6), Color(249, 249, 249)));

    for (int i = 0; i < node->size(); ++i) {
      if (i > 0) beginLogicalLine();
      node->childAt(i)->accept(this);
      if (i + 1 < node->size()) endLogicalLine();
    }
    endBlock();

    if (m_hasGui && m_imageProvider) {
      String copyBtnFilePath = ":icon/copy_32x32.png";
      auto image = m_imageProvider->load(copyBtnFilePath);
      if (!image.isNull()) {
        Point pos(x + w - image.width, y);
        m_instructions.push_back(std::make_unique<StaticImageInstruction>(copyBtnFilePath, pos, Size(image.width, image.height), std::move(image)));
        m_block.appendElement({node, pos, Size(image.width, image.height)});
      }
    }
    restore();
  }
  void visit(InlineLatex *node) override {
    ASSERT(node != nullptr);
    save();
    auto latex = node->code()->toString(m_doc);
    try {
      float textSize = m_setting->latexFontSize;
      auto render = std::unique_ptr<microtex::Render>(
          microtex::MicroTeX::parse(latex.toStdString(), m_setting->contentMaxWidth(), textSize,
                                    textSize / 3.f, 0xff424242));
      const Point &point = Point(m_curX, m_curY);
      const Size &size = Size(render->getWidth() + 2, render->getHeight());
      auto cell = std::make_unique<InlineLatexCell>(point, size);
      auto* rawCell = cell.get();
      appendVisualCell(std::move(cell));
      m_instructions.push_back(std::make_unique<LatexInstruction>(rawCell, latex, textSize));
      m_curX += render->getWidth();
    } catch (const std::exception &ex) {
      DEBUG << "ERROR" << ex.what();
      DEBUG << "Render LaTeX fail:" << latex;
    }
    restore();
  }
  void visit(LatexBlock *node) override {
    ASSERT(node != nullptr);
    save();
    beginBlock();
    auto latex = node->toString(m_doc);
    try {
      float textSize = m_setting->latexFontSize;
      auto render = std::unique_ptr<microtex::Render>(
          microtex::MicroTeX::parse(latex.toStdString(), m_setting->contentMaxWidth(), textSize,
                                    textSize / 3.f, 0xff424242));
      const Point &point = Point((m_setting->contentMaxWidth() - render->getWidth()) / 2, m_curY);
      const Size &size = Size(m_setting->contentMaxWidth(), render->getHeight());
      auto cell = std::make_unique<InlineLatexCell>(point, size);
      auto* rawCell = cell.get();
      appendVisualCell(std::move(cell));
      m_instructions.push_back(std::make_unique<LatexInstruction>(rawCell, latex, textSize));
      m_curX += render->getWidth();
    } catch (const std::exception &ex) {
      DEBUG << "ERROR" << ex.what();
      DEBUG << "Render LaTeX fail:" << latex;
    }
    endBlock();
    restore();
  }
  void visit(ItalicText *node) override {
    ASSERT(node != nullptr);
    save();
    auto font = curFont();
    font.italic = true;
    setFont(font);
    node->text()->accept(this);
    restore();
  }
  void visit(BoldText *node) override {
    ASSERT(node != nullptr);
    save();
    auto font = curFont();
    font.bold = true;
    setFont(font);
    node->text()->accept(this);
    restore();
  }
  void visit(ItalicBoldText *node) override {
    ASSERT(node != nullptr);
    save();
    auto font = curFont();
    font.italic = true;
    font.bold = true;
    setFont(font);
    node->text()->accept(this);
    restore();
  }
  void visit(StrickoutText *node) override {
    ASSERT(node != nullptr);
    save();
    auto font = curFont();
    font.strikeOut = true;
    setFont(font);
    node->text()->accept(this);
    restore();
  }
  void visit(Image *node) override {
    ASSERT(node != nullptr);
    beginVisualLine();
    String imgPath = node->src()->toString(m_doc);
#if defined(__ANDROID__) || defined(__unix__)
    if (!imgPath.startsWith("/")) {
      for (const auto &resPath : m_setting->resPathList) {
        String newImgPath = resPath + "/" + imgPath;
        if (m_imageProvider && m_imageProvider->exists(newImgPath)) {
          imgPath = newImgPath;
        }
      }
    }
#endif

    if (!m_imageProvider || !m_imageProvider->exists(imgPath)) {
      std::cerr << "image not exist: " << imgPath << std::endl;
      return;
    }
    auto image = m_imageProvider->load(imgPath);
    int imageMaxWidth = std::min(1080, m_setting->contentMaxWidth());
    int imgWidth = image.width;
    while (imgWidth > imageMaxWidth) {
      imgWidth /= 2;
    }
    int displayWidth = imgWidth;
    int displayHeight = (displayWidth * image.height) / image.width;
    const Point &pos = Point(m_curX, m_curY);
    Size imgSize(displayWidth, displayHeight);
    m_instructions.push_back(std::make_unique<ImageInstruction>(imgPath, pos, imgSize, std::move(image)));
    m_curY += displayHeight;
    m_block.appendElement({node, pos, imgSize});
    // TODO: 需要重新考虑图片
    m_block.m_logicalLines.back().m_lines.back().m_h = displayHeight;
    endVisualLine();
    if (!imgPath.endsWith(".gif")) {
      return;
    }
    // gif加一个播放的图标
    String playIconPath = ":/icon/play_64x64.png";
    auto playIcon = m_imageProvider->load(playIconPath);
    if (playIcon.isNull()) return;
    // 计算播放图标所在位置
    // 播放图标放在中心位置
    int x = (displayWidth - playIcon.width) / 2 + pos.x;
    int y = (displayHeight - playIcon.height) / 2 + pos.y;
    m_instructions.push_back(std::make_unique<StaticImageInstruction>(playIconPath, Point(x, y), Size(playIcon.width, playIcon.height), std::move(playIcon)));
  }
  void visit(CheckboxList *node) override {
    ASSERT(node != nullptr);
    beginBlock(false);
    for (const auto &item : node->children()) {
      beginLogicalLine(false);
      item->accept(this);
      endLogicalLine();
    }
    endBlock();
  }
  void visit(CheckboxItem *node) override {
    ASSERT(node != nullptr);
    save();
    int oldX = m_curX;
    m_curX += m_setting->checkboxMargin.left;
    auto font = curFont();
    // 计算高度偏移
    auto h1 = textHeight();

    save();
    const Point &pos = Point(m_curX, m_curY);
    const Size &size = Size(h1, h1);
    if (node->isChecked()) {
      String imagePath = ":icon/checkbox-selected_64x64.png";
      auto image = m_imageProvider ? m_imageProvider->load(imagePath) : editor::core::ImageData();
      m_instructions.push_back(std::make_unique<StaticImageInstruction>(imagePath, pos, size, std::move(image)));
    } else {
      String imagePath = ":icon/checkbox-unselected_64x64.png";
      auto image = m_imageProvider ? m_imageProvider->load(imagePath) : editor::core::ImageData();
      m_instructions.push_back(std::make_unique<StaticImageInstruction>(imagePath, pos, size, std::move(image)));
    }
    m_block.appendElement({node, pos, size});
    m_curX += h1 + 10;
    restore();
    font = curFont();
    font.strikeOut = node->isChecked();
    setFont(font);
    m_block.m_logicalLines.back().m_padding = m_curX - oldX;
    beginVisualLine();
    for (const auto &item : node->children()) {
      item->accept(this);
    }
    restore();
  }
  void visit(UnorderedList *node) override {
    ASSERT(node != nullptr);
    beginBlock(false);
    for (const auto &item : node->children()) {
      beginLogicalLine(false);
      auto oldX = m_curX;
      m_curX += m_setting->listMargin.left;
      auto h = textHeight();
      auto size = 5;
      auto y = m_curY + (h - size) / 2 + 2;
      m_instructions.push_back(std::make_unique<EllipseInstruction>(Point(m_curX, y), Size(size, size), Color::black()));
      m_curX += 15;
      m_block.m_logicalLines.back().m_padding = m_curX - oldX;
      beginVisualLine();
      item->accept(this);
      endLogicalLine();
    }
    endBlock();
  }
  void visit(UnorderedListItem *node) override {
    ASSERT(node != nullptr);
    for (const auto &item : node->children()) {
      item->accept(this);
    }
  }
  void visit(OrderedList *node) override {
    ASSERT(node != nullptr);
    beginBlock(false);
    int i = 0;
    for (const auto &item : node->children()) {
      i++;
      beginLogicalLine(false);
      auto oldX = m_curX;
      m_curX += m_setting->listMargin.left;
      String numStr = std::to_string(i) + ".  ";
      const Size &size = textSize(numStr);
      const Point &pos = Point(m_curX, m_curY);
      m_instructions.push_back(std::make_unique<StaticTextInstruction>(numStr, pos, size, Color::black(), curFont()));
      m_curX += size.width;
      m_block.m_logicalLines.back().m_padding = m_curX - oldX;
      beginVisualLine();
      item->accept(this);
      endLogicalLine();
    }
    endBlock();
  }
  void visit(OrderedListItem *node) override {
    ASSERT(node != nullptr);
    for (const auto &item : node->children()) {
      item->accept(this);
    }
  }
  void visit(Hr *node) override {
    ASSERT(node != nullptr);
    save();
    beginBlock();
    int lineY = m_curY + m_setting->lineSpacing;
    m_instructions.push_back(std::make_unique<FillRectInstruction>(
        Point(m_setting->docMargin.left, lineY),
        Size(m_setting->contentMaxWidth() - m_setting->docMargin.left, 1),
        Color(200, 200, 200)));
    m_curY = lineY + m_setting->lineSpacing;
    endBlock();
    restore();
  }
  void visit(QuoteBlock *node) override {
    ASSERT(node != nullptr);
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
    Color bgColor(238, 238, 238);
    Point pos(m_setting->docMargin.left - m_setting->quoteMargin.left, startY);
    m_instructions.push_back(std::make_unique<FillRectInstruction>(pos, Size(5, endY - startY), bgColor));
    endBlock();
  }
  void visit(Table *node) override {
    ASSERT(node != nullptr);
    save();
    beginBlock();
    auto font = curFont();
    setFont(font);
    // Render header
    if (!node->header().empty()) {
      String headerLine;
      for (const auto& cell : node->header()) {
        headerLine += cell + " | ";
      }
      auto size = textSize(headerLine);
      m_instructions.push_back(std::make_unique<StaticTextInstruction>(
          headerLine, Point(m_curX, m_curY), size, curPen(), curFont()));
      m_curY += size.height + m_setting->lineSpacing;
    }
    // Render content rows
    for (const auto& row : node->content()) {
      String rowLine;
      for (const auto& cell : row) {
        rowLine += cell + " | ";
      }
      auto size = textSize(rowLine);
      m_instructions.push_back(std::make_unique<StaticTextInstruction>(
          rowLine, Point(m_curX, m_curY), size, curPen(), curFont()));
      m_curY += size.height + m_setting->lineSpacing;
    }
    endBlock();
    restore();
  }
  void visit(Lf *node) override {
    ASSERT(node != nullptr);
    save();
#ifdef __ANDROID__
#else
    String enterStr = "↲";
    auto font = curFont();
    font.pixelSize = 12;
    auto size = textSize(enterStr);
    m_instructions.push_back(std::make_unique<StaticTextInstruction>(enterStr, Point(m_curX, m_curY), size, Color::blue(), font));
#endif
    endLogicalLine();
    beginLogicalLine();
    restore();
  }
  [[nodiscard]] Block execute() {
    m_block.setInstructions(std::move(m_instructions));
    return std::move(m_block);
  }

 private:
  void drawHeaderPrefix(int level) {
    save();
    String prefix = std::format("H{}", level);
    auto font = curFont();
    font.pixelSize = font.pixelSize - 4;
    setFont(font);
    auto size = textSize(prefix);
    auto x = m_curX - size.width - 1;
    m_instructions.push_back(std::make_unique<StaticTextInstruction>(prefix, Point(x, m_curY), size, Color::black(), font));
    restore();
  }

  void drawText(Text *node, const String &str, RenderString &s, SizeType offset, SizeType length) {
    save();
    if (s.type == RenderString::English) {
      if (m_rewriteFont) {
        auto font = curFont();
        font.family = m_setting->enTextFont.c_str();
        setFont(font);
      }
    } else if (s.type == RenderString::Chinese) {
      auto font = curFont();
      font.family = m_setting->zhTextFont.c_str();
      setFont(font);
    }
    const String &text = str.mid(offset, length);
    const Size &size = textSize(text);
    auto cell = std::make_unique<TextCell>(node, offset, length, Point(m_curX, m_curY), size, curPen(), curFont(), m_fontMetrics);
    auto* rawCell = cell.get();
    appendVisualCell(std::move(cell));
    m_instructions.push_back(std::make_unique<TextInstruction>(rawCell));
    restore();
    m_curX += size.width;
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
    m_curX = m_setting->docMargin.left;
    logicalLine.m_pos = Point(m_curX, m_curY);
    auto size = m_fontMetrics->size(curFont(), "龙");
    logicalLine.m_h = size.height;
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
    line.m_lines.push_back(VisualLine(Point(m_curX, m_curY), size.height));
  }
  void endVisualLine() {
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
      auto y = cell->m_pos.y;
      auto delta = maxH - cell->height();
      cell->m_pos.y = y + delta / 2;
    }
    m_curY += maxH + m_setting->lineSpacing;
  }
  void moveToNewLine() {
    endVisualLine();
    m_curX = m_setting->docMargin.left;
    beginVisualLine();
  }

 private:
  // 画笔相关到操作
  void save() { m_configs.push_back(m_config); }
  void restore() {
    ASSERT(!m_configs.empty());
    m_config = m_configs.back();
    m_configs.pop_back();
  }
  Font codeFont() {
    auto font = curFont();
#ifdef _WIN32
    font.family = "Cascadia Code";
#elif defined(__APPLE__)
    font.family = "Menlo";
#else
    font.family = "monospace";
#endif
    font.pixelSize = 20;
    return font;
  }
  [[nodiscard]] Font curFont() const { return m_config.font; }
  [[nodiscard]] Color curPen() const { return m_config.pen; }
  void setFont(const Font &font) { m_config.font = font; }
  void setPen(const Color &color) { m_config.pen = color; }
  // 辅助到绘制方法
  Size textSize(const String &text) { return m_fontMetrics->size(curFont(), text); }

  int textWidth(const String &text) {
    return m_fontMetrics->horizontalAdvance(curFont(), text);
  }

  int charWidth(const String &text) {
    return m_fontMetrics->horizontalAdvance(curFont(), text);
  }

  int textHeight() {
    return m_fontMetrics->height(curFont());
  }

  bool currentLineCanDrawText(const String &text) {
    auto needWidth = textWidth(text);
    if (m_curX + needWidth < m_setting->contentMaxWidth()) {
      return true;
    } else {
      return false;
    }
  }

  int countOfThisLineCanDraw(const String &text) {
    // 计算这一行可以画多少个字符
    // Measure the width of the first complete code point, not a single byte
    int cpLen = md::utf8SequenceLength(text[0]);
    auto ch_w = charWidth(text.left(cpLen));
    int left_w = m_setting->contentMaxWidth() - m_curX;
    int may_ch_count = left_w / ch_w - 1;
    // 可能根本画不了
    if (may_ch_count <= 0) return 0;
    // Ensure we don't split multi-byte UTF-8 characters
    auto alignedLeft = [&text](int n) {
      while (n > 0 && (static_cast<unsigned char>(text[n]) & 0xC0) == 0x80) {
        n--;
      }
      return text.left(n);
    };
    if (currentLineCanDrawText(alignedLeft(may_ch_count + 1))) {
      while (currentLineCanDrawText(alignedLeft(may_ch_count + 1))) {
        may_ch_count++;
      }
    } else {
      while (!currentLineCanDrawText(alignedLeft(may_ch_count))) {
        may_ch_count--;
      }
    }
    // Final alignment to code point boundary
    while (may_ch_count > 0 && (static_cast<unsigned char>(text[may_ch_count]) & 0xC0) == 0x80) {
      may_ch_count--;
    }
    return may_ch_count;
  }

 private:
  std::vector<LayoutConfig> m_configs;
  LayoutConfig m_config;
  const parser::IBufferProvider& m_doc;
  Block m_block;

  int m_curX = 0;
  int m_curY = 0;
  sptr<RenderSetting> m_setting;

  bool m_rewriteFont = true;
  InstructionPtrList m_instructions;
  IFontMetricsProvider* m_fontMetrics;
  bool m_hasGui;
  editor::core::IImageProvider* m_imageProvider = nullptr;
};
int VisualLine::height() const { return m_h; }
SizeType VisualLine::length() const {
  auto l = 0;
  for (const auto& cell : m_cells) {
    l += cell->length();
  }
  return l;
}
std::pair<Cell *, int> VisualLine::cellAtX(int x, const parser::IBufferProvider& doc) const {
  if (m_cells.empty()) {
    return {nullptr, 0};
  }
  if (x <= m_cells.front()->m_pos.x) {
    return {m_cells.front().get(), 0};
  }
  auto totalX = m_cells.front()->m_pos.x;
  for (const auto& cell : m_cells) {
    if (totalX <= x && x <= totalX + cell->width()) {
      // 再确定offset
      for (int j = 0; j < cell->length(); ++j) {
        auto w = cell->width(j + 1, doc);
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
  auto w = cell->m_pos.x + cell->m_size.width - m_pos.x;
  return w;
}
int LogicalLine::height() const { return m_h; }
std::tuple<Point, int, int> LogicalLine::cursorAt(SizeType offset, const parser::IBufferProvider& doc) const {
  if (m_cells.empty()) {
    return {Point(m_pos.x + m_padding, m_pos.y), m_h, 0};
  }
  auto totalOffset = 0;
  for (auto cell : m_cells) {
    if (totalOffset <= offset && offset < totalOffset + cell->length()) {
      auto w = cell->width(offset - totalOffset, doc);
      auto pos = Point(cell->m_pos.x + w, cell->m_pos.y + cell->ascent());
      return {pos, cell->m_size.height, cell->ascent()};
    }
    totalOffset += cell->length();
  }
  if (offset == totalOffset) {
    auto cell = m_cells.back();
    auto pos = Point(cell->m_pos.x + cell->width(), cell->m_pos.y + cell->ascent());
    return {pos, cell->m_size.height, cell->ascent()};
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
      return cell->textNode() != nullptr;
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
      auto* textNode = cell->textNode();
      if (!textNode) continue;
      return {textNode, leftOffset + cell->textOffset()};
    }
    totalOffset += cell->length();
  }
  DEBUG << m_cells.size() << offset << totalOffset;
  ASSERT(false && "text not in cell");
}
String LogicalLine::left(SizeType length, const parser::IBufferProvider& doc) const {
  ASSERT(length >= 0 && length <= this->length());
  if (length == 0) return String();
  String s;
  for (auto cell : m_cells) {
    auto* textNode = cell->textNode();
    if (!textNode) continue;
    s += textNode->toString(doc);
    if (s.size() >= length) return s.left(length);
  }
  DEBUG << this->length() << length << s;
  ASSERT(false && "length");
}
bool LogicalLine::canMoveDown(SizeType offset, const parser::IBufferProvider& doc) const {
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
bool LogicalLine::canMoveUp(SizeType offset, const parser::IBufferProvider& doc) const {
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
SizeType LogicalLine::moveDown(SizeType offset, int x, const parser::IBufferProvider& doc) const {
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
SizeType LogicalLine::moveUp(SizeType offset, int x, const parser::IBufferProvider& doc) const {
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
SizeType LogicalLine::moveToX(int x, const parser::IBufferProvider& doc, bool lastLine) const {
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
SizeType LogicalLine::moveToBol(SizeType offset, const parser::IBufferProvider& doc) const {
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
std::pair<SizeType, int> LogicalLine::moveToEol(SizeType offset, const parser::IBufferProvider& /*doc*/) const {
  ASSERT(offset >= 0 && offset <= this->length());
  if (m_cells.empty()) return {0, m_pos.x};
  SizeType totalOffset = 0;
  for (const auto &line : m_lines) {
    if (totalOffset <= offset && offset < totalOffset + line.length()) {
      return {totalOffset + line.length(), line.m_pos.x + line.width()};
    }
    totalOffset += line.length();
  }
  if (totalOffset == offset) {
    ASSERT(!m_lines.empty());
    const auto &line = m_lines.back();
    return {totalOffset, line.m_pos.x + line.width()};
  }
  ASSERT(false && "no offset in line");
}
SizeType LogicalLine::offsetAt(Point pos, const parser::IBufferProvider& doc, int lineSpacing) const {
  int y = m_pos.y;
  // -1表示没有算出offset
  SizeType offset = -1;
  for (const auto &line : m_lines) {
    // 这里要考虑lineSpacing
    if (y - lineSpacing / 2 <= pos.y && pos.y <= y + line.height() + lineSpacing / 2) {
      auto [cell, delta] = line.cellAtX(pos.x, doc);
      offset = this->totalOffset(cell, delta);
      break;
    }
    y += line.height();
  }
  if (offset == -1 && pos.y <= this->height() + m_pos.y) {
    ASSERT(!m_lines.empty());
    const auto &line = m_lines.back();
    auto [cell, delta] = line.cellAtX(pos.x, doc);
    offset = this->totalOffset(cell, delta);
  }
  if (offset != -1) {
    // 修正emoji offset — ensure cursor is at a valid code-point boundary
    if (offset > 0 && offset < this->length()) {
      auto s = this->left(offset + 1, doc);
      // If the byte at offset is a continuation byte, we're inside a multi-byte sequence.
      // Walk backward to the lead byte, then advance past the whole sequence.
      if ((static_cast<unsigned char>(s[offset]) & 0xC0) == 0x80) {
        SizeType p = offset;
        while (p > 0 && (static_cast<unsigned char>(s[p - 1]) & 0xC0) == 0x80) {
          --p;
        }
        if (p > 0 || (static_cast<unsigned char>(s[0]) & 0xC0) != 0x80) {
          int seqLen = md::utf8SequenceLength(s[p]);
          offset = p + seqLen;
        }
      }
    }
    return offset;
  }
  DEBUG << this->left(this->length(), doc);
  DEBUG << "pos=(" << pos.x << "," << pos.y << ") m_pos=(" << m_pos.x << "," << m_pos.y << ") w=" << this->width() << " h=" << this->height();
  ASSERT(false && "no pos in line");
}
int LogicalLine::visualLineAt(SizeType offset, const parser::IBufferProvider& doc) const {
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
bool LogicalLine::isBol(SizeType offset, const parser::IBufferProvider& doc) const {
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
Block Render::render(Node *node, sptr<RenderSetting> setting, const parser::IBufferProvider& doc,
                     IFontMetricsProvider* fontMetrics,
                     editor::core::IImageProvider* imageProvider) {
  ASSERT(node != nullptr);
  LayoutPass render(node, setting, doc, fontMetrics, imageProvider);
  node->accept(&render);
  Block block = render.execute();
  return block;
}
}  // namespace md::render
