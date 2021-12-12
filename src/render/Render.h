//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_RENDER_H
#define QTMARKDOWN_RENDER_H
#include "QtMarkdown_global.h"
#include "Instruction.h"
#include "mddef.h"
#include "parser/Document.h"
#include "Element.h"
namespace md::render {
struct RenderSetting {
  bool highlightCurrentLine = false;
  int lineSpacing = 10;
  int maxWidth = 800;
  int latexFontSize = 20;
#ifdef Q_OS_ANDROID
  String zhTextFont = "Noto Sans CJK SC";
//  String zhTextFont = "MI Lan Pro VF";
#else
  String zhTextFont = "苹方-简";
#endif
  String enTextFont = "Times New Roman";
  StringList resPathList;
  std::array<int, 6> headerFontSize = {36, 28, 24, 20, 16, 14};
  QMargins docMargin = QMargins(100, 20, 20, 20);
  QMargins codeMargin = QMargins(10, 20, 20, 10);
  QMargins listMargin = QMargins(15, 20, 20, 10);
  QMargins checkboxMargin = QMargins(15, 20, 20, 10);
  QMargins quoteMargin = QMargins(10, 20, 20, 10);
  [[nodiscard]] int contentMaxWidth() const { return maxWidth - docMargin.left() - docMargin.right(); }
};
class LogicalLine;
class QTMARKDOWNSHARED_EXPORT VisualLine {
 public:
  VisualLine(Point pos, int h) : m_pos(pos), m_h(h) {}
  int height() const;
  int width() const;
  SizeType length() const;
  bool hasCell(Cell* cell) const;
  std::pair<Cell*, int> cellAtX(int x, DocPtr doc) const;
  Point pos() const { return m_pos; }

 private:
  std::vector<Cell*> m_cells;
  Point m_pos;
  int m_h;
  friend class RenderPrivate;
  friend class LogicalLine;
};
class QTMARKDOWNSHARED_EXPORT LogicalLine {
  using VisualLineList = std::vector<VisualLine>;

 public:
  int height() const;
  int width() const;
  std::pair<Point, int> cursorAt(SizeType offset, DocPtr doc);
  bool hasTextAt(SizeType offset) const;
  // 第一个Text是当前offset所在Text结点
  // 第二个int是当前offset在结点中还有的offset
  std::pair<parser::Text*, int> textAt(SizeType offset) const;
  SizeType length() const;
  String left(SizeType length, DocPtr doc) const;
  bool canMoveDown(SizeType offset, DocPtr doc) const;
  bool canMoveUp(SizeType offset, DocPtr doc) const;
  SizeType moveDown(SizeType offset, int x, DocPtr doc) const;
  SizeType moveUp(SizeType offset, int x, DocPtr doc) const;
  SizeType moveToX(int x, DocPtr doc, bool lastLine = false) const;
  SizeType moveToBol(SizeType offset, DocPtr doc) const;
  std::pair<SizeType, int> moveToEol(SizeType offset, DocPtr) const;
  auto empty() const { return m_cells.empty(); }
  SizeType offsetAt(Point pos, DocPtr doc, int lineSpacing) const;
  int visualLineAt(SizeType offset, DocPtr doc) const;
  VisualLine& visualLineAt(int index);
  const VisualLine& visualLineAt(int index) const;
  auto countOfVisualLine() const { return m_lines.size(); }
  bool isBol(SizeType offset, const DocPtr doc) const;

 private:
  SizeType totalOffset(Cell* cell, SizeType delta) const;

 private:
  VisualLineList m_lines;
  std::vector<Cell*> m_cells;
  Point m_pos;
  int m_h;
  int m_padding = 0;
  friend class RenderPrivate;
};
class QTMARKDOWNSHARED_EXPORT Block {
 public:
  using LogicalLineList = std::vector<LogicalLine>;
  explicit Block(parser::Node* node) : m_node(node) {}
  void appendInstruction(Instruction* instruction);
  void appendElement(Element element) { m_elements.push_back(element); }
  void insertInstruction(SizeType index, Instruction* instruction);
  int width() const;
  [[nodiscard]] int height() const;
  [[nodiscard]] LogicalLineList lines() const { return m_logicalLines; }
  [[nodiscard]] auto begin() const { return m_instructions.begin(); }
  [[nodiscard]] auto end() const { return m_instructions.end(); }
  auto countOfLogicalLine() const { return m_logicalLines.size(); }
  const LogicalLine& logicalLineAt(SizeType index) const;
  const ElementList& elementList() const { return m_elements; }

 private:
  // 逻辑行
  LogicalLineList m_logicalLines;
  // 绘图指令
  InstructionPtrList m_instructions;
  ElementList m_elements;

  // 方便调试用
  parser::Node* m_node;
  friend class RenderPrivate;
};

class QTMARKDOWNSHARED_EXPORT Render {
 public:
  static Block render(parser::Node* node, sptr<RenderSetting> setting, DocPtr doc);

 private:
};
using BlockList = std::vector<Block>;
}  // namespace md::render
#endif  // QTMARKDOWN_RENDER_H
