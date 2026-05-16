//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_RENDER_H
#define QTMARKDOWN_RENDER_H
#include "QtMarkdown_global.h"
#include "Instruction.h"
#include "mddef.h"
#include "parser/Document.h"
#include "parser/IBufferProvider.h"
#include "Element.h"
#include "core/IImageProvider.h"
namespace md::render {
class IFontMetricsProvider;
struct RenderSetting {
  bool highlightCurrentLine = false;
  int blockSpacing = 10;
  int lineSpacing = 10;
  int maxWidth = 800;
  int latexFontSize = 20;
  int paragraphIntent = 2;
#ifdef __ANDROID__
  String zhTextFont = "Noto Sans CJK SC";
#else
  String zhTextFont = "PingFang SC";
#endif
  StringList resPathList;
  std::array<int, 6> headerFontSize = {36, 28, 24, 20, 16, 14};
  editor::core::Margins docMargin{100, 20, 20, 20};
  editor::core::Margins codeMargin{10, 20, 20, 10};
  editor::core::Margins listMargin{15, 20, 20, 10};
  editor::core::Margins checkboxMargin{15, 20, 20, 10};
  editor::core::Margins quoteMargin{10, 20, 20, 10};
  [[nodiscard]] int contentMaxWidth() const { return maxWidth - docMargin.left - docMargin.right; }
};
class LogicalLine;
class QTMARKDOWNRENDER_EXPORT VisualLine {
 public:
  VisualLine(Point pos, int h) : m_pos(pos), m_h(h) {}
  VisualLine(const VisualLine&) = delete;
  VisualLine& operator=(const VisualLine&) = delete;
  VisualLine(VisualLine&&) noexcept = default;
  VisualLine& operator=(VisualLine&&) noexcept = default;
  int height() const;
  int width() const;
  SizeType length() const;
  std::pair<Cell*, int> cellAtX(int x, const parser::IBufferProvider& doc) const;
  Point pos() const { return m_pos; }

 private:
  std::vector<std::unique_ptr<Cell>> m_cells;
  Point m_pos;
  int m_h;
  friend class LayoutPass;
  friend class LogicalLine;
};
class QTMARKDOWNRENDER_EXPORT LogicalLine {
  using VisualLineList = std::vector<VisualLine>;

 public:
  LogicalLine() = default;
  LogicalLine(const LogicalLine&) = delete;
  LogicalLine& operator=(const LogicalLine&) = delete;
  LogicalLine(LogicalLine&&) noexcept = default;
  LogicalLine& operator=(LogicalLine&&) noexcept = default;
  int height() const;
  int width() const;
  std::tuple<Point, int, int> cursorAt(SizeType offset, const parser::IBufferProvider& doc) const;
  bool hasTextAt(SizeType offset) const;
  // 第一个Text是当前offset所在Text结点
  // 第二个int是当前offset在结点中还有的offset
  std::pair<parser::Text*, int> textAt(SizeType offset) const;
  SizeType length() const;
  String left(SizeType length, const parser::IBufferProvider& doc) const;
  bool canMoveDown(SizeType offset, const parser::IBufferProvider& doc) const;
  bool canMoveUp(SizeType offset, const parser::IBufferProvider& doc) const;
  SizeType moveDown(SizeType offset, int x, const parser::IBufferProvider& doc) const;
  SizeType moveUp(SizeType offset, int x, const parser::IBufferProvider& doc) const;
  SizeType moveToX(int x, const parser::IBufferProvider& doc, bool lastLine = false) const;
  SizeType moveToBol(SizeType offset, const parser::IBufferProvider& doc) const;
  std::pair<SizeType, int> moveToEol(SizeType offset, const parser::IBufferProvider& /*doc*/) const;
  auto empty() const { return m_cells.empty(); }
  SizeType offsetAt(Point pos, const parser::IBufferProvider& doc, int lineSpacing) const;
  int visualLineAt(SizeType offset, const parser::IBufferProvider& doc) const;
  VisualLine& visualLineAt(int index);
  const VisualLine& visualLineAt(int index) const;
  auto countOfVisualLine() const { return m_lines.size(); }
  bool isBol(SizeType offset, const parser::IBufferProvider& doc) const;

  const std::vector<Cell*>& cells() const { return m_cells; }

 private:
  SizeType totalOffset(Cell* cell, SizeType delta) const;

 private:
  VisualLineList m_lines;
  std::vector<Cell*> m_cells;
  Point m_pos;
  int m_h;
  int m_padding = 0;
  friend class LayoutPass;
};
class QTMARKDOWNRENDER_EXPORT Block {
 public:
  using LogicalLineList = std::vector<LogicalLine>;
  Block() = default;
  Block(const Block&) = delete;
  Block& operator=(const Block&) = delete;
  Block(Block&&) noexcept = default;
  Block& operator=(Block&&) noexcept = default;
  void setInstructions(InstructionPtrList instructions) { m_instructions = std::move(instructions); }
  void appendElement(Element element) { m_elements.push_back(element); }
  int width() const;
  [[nodiscard]] int height() const;
  [[nodiscard]] const LogicalLineList& lines() const { return m_logicalLines; }
  [[nodiscard]] auto begin() const { return m_instructions.begin(); }
  [[nodiscard]] auto end() const { return m_instructions.end(); }
  auto countOfLogicalLine() const { return m_logicalLines.size(); }
  const LogicalLine& logicalLineAt(SizeType index) const;
  const ElementList& elementList() const { return m_elements; }

 private:
  // Destruction order: m_instructions (non-owning raw Cell*) destroyed BEFORE m_logicalLines.
  // m_logicalLines owns cells via VisualLine::vector<unique_ptr<Cell>>.
  // C++ destroys members in reverse declaration order, so m_instructions is destroyed first.
  // The raw Cell* pointers in Instructions dangle only after the Instructions themselves are gone.
  //
  // NOTE: No static_assert with offsetof here — Block is not a standard-layout type
  // (has std::vector members, members under different access specifiers).
  // Applying offsetof to non-standard-layout types is conditionally-supported UB in C++17.
  // 逻辑行
  LogicalLineList m_logicalLines;
  // 绘图指令
  InstructionPtrList m_instructions;
  ElementList m_elements;

  // Non-owning pointer to the AST node this Block was rendered from.
  // The AST (parser::Document) must outlive this Block.
  // For debugging only — prefer Element/Instruction data in production paths.
  friend class LayoutPass;
};

class QTMARKDOWNRENDER_EXPORT Render {
 public:
  static Block render(parser::Node* node, sptr<RenderSetting> setting, const parser::IBufferProvider& doc,
                      IFontMetricsProvider* fontMetrics = nullptr,
                      editor::core::IImageProvider* imageProvider = nullptr);

 private:
};
using BlockList = std::vector<Block>;
}  // namespace md::render
#endif  // QTMARKDOWN_RENDER_H
