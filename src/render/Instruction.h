//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_INSTRUCTION_H
#define QTMARKDOWN_INSTRUCTION_H
#include <QPainter>
#include <QRect>
#include <utility>
#include <vector>

#include "mddef.h"
#include "parser/PieceTable.h"
namespace md::render {
using Painter = QPainter;
using Point = QPoint;
using Rect = QRect;
using Font = QFont;
}  // namespace md::render
namespace md::parser {
class Text;
}
namespace md::render {
struct InstructionPainterConfig {
  Rect rect;
  Font font;
  Brush brush;
  Color pen;
};
class Instruction {
 public:
  explicit Instruction(InstructionPainterConfig config, bool lineHeight = true)
      : m_config(std::move(config)), m_lineH(lineHeight) {}
  virtual void run(Painter& painter, Point offset, DocPtr doc) const = 0;
  [[nodiscard]] virtual int height() const {
    if (m_lineH)
      return m_config.rect.height();
    else
      return 0;
  }
  [[nodiscard]] const InstructionPainterConfig& config() const { return m_config; }
  bool containX(int x) const { return x >= m_config.rect.x() && m_config.rect.x() + m_config.rect.width() >= x; }

 private:
 protected:
  static Rect updateRect(Rect rect, Point offset);

 protected:
  InstructionPainterConfig m_config;
  bool m_lineH;
};
class Cell {
 public:
  Cell(Font font, Point pos, bool bol, bool eol) : m_font(font), m_pos(pos), m_bol(bol), m_eol(eol) {}
  Font font() const { return m_font; }
  Point pos() const { return m_pos; }
  bool bol() const { return m_bol; }
  bool eol() const { return m_eol; }
  virtual bool isTextCell() = 0;
  virtual SizeType length() = 0;

 private:
  Font m_font;
  Point m_pos;
  // 视觉行开始
  bool m_bol;
  // 视觉行结束
  bool m_eol;
};
class TextCell : public Cell {
 public:
  TextCell(parser::Text* text, SizeType offset, SizeType length, Font font, Point pos, bool bol, bool eol)
      : Cell(font, pos, bol, eol), m_text(text), m_offset(offset), m_length(length) {}
  auto text() const { return m_text; }
  auto offset() const { return m_offset; }
  bool isTextCell() override { return true; }
  SizeType length() override { return m_length; }

 private:
  parser::Text* m_text;
  SizeType m_offset;
  SizeType m_length;
};
struct StaticTextCell : public Cell {
 public:
  StaticTextCell(String text, Font font, Point pos, bool bol, bool eol) : Cell(font, pos, bol, eol), m_text(text) {}
  bool isTextCell() override { return false; }
  SizeType length() override { return 0; }

 private:
  String m_text;
};
using VisualItem = InstructionPtr;
using LogicalItem = Cell*;
using VisualLine = InstructionPtrList;
using LogicalLine = QList<LogicalItem>;
class Block {
 public:
  Block(parser::Node* node) : m_node(node) {}
  void newVisualLine() { m_visualLines.push_back(VisualLine()); }
  void newLogicalLine() { m_logicalLines.push_back(LogicalLine()); }
  void appendVisualItem(VisualItem item);
  void appendLogicalItem(LogicalItem item);
  [[nodiscard]] int height() const {
    int h = 0;
    for (const auto& list : m_visualLines) {
      int maxH = 0;
      for (auto it : list) {
        maxH = std::max(maxH, it->height());
      }
      h += maxH;
    }
    return h;
  }
  [[nodiscard]] SizeType countOfVisualLine() const { return m_visualLines.size(); }
  [[nodiscard]] SizeType countOfLogicalLine() const { return m_logicalLines.size(); }
  [[nodiscard]] SizeType countOfVisualItem(SizeType indexOfLine) const;
  [[nodiscard]] const VisualItem& visualItemAt(SizeType indexOfLine, SizeType indexOfItem) const;
  void insertVisualItem(SizeType indexOfLine, SizeType indexOfItem, VisualItem item);
  void insertNewVisualLineAt(SizeType index, Instruction* instruction) {
    m_visualLines.insert(index, InstructionPtrList());
    m_visualLines[index].push_back(instruction);
  }
  [[nodiscard]] const QList<VisualLine>& visualLines() const { return m_visualLines; }
  [[nodiscard]] const QList<LogicalLine>& logicalLines() const { return m_logicalLines; }
  [[nodiscard]] SizeType maxOffsetOfLogicalLine(SizeType index) const;

 private:
  // 视觉行
  QList<VisualLine> m_visualLines;
  // 逻辑行
  QList<LogicalLine> m_logicalLines;
  // 方便调试用
  parser::Node* m_node;
};
class InstructionGroup2 {
 public:
  void createNewLine() { m_instructions.append(InstructionPtrList()); }
  void appendItem(Instruction* instruction) { m_instructions.back().push_back(instruction); }
  [[nodiscard]] auto begin() const { return m_instructions.begin(); }
  [[nodiscard]] auto end() const { return m_instructions.end(); }
  [[nodiscard]] auto size() const { return m_instructions.size(); }
  void insertNewLineAt(SizeType index, Instruction* instruction) {
    m_instructions.insert(index, InstructionPtrList());
    m_instructions[index].push_back(instruction);
  }
  [[nodiscard]] auto at(SizeType index) const { return m_instructions.at(index); }
  InstructionPtrList& operator[](SizeType index) { return m_instructions[index]; }
  [[nodiscard]] int height() const {
    int h = 0;
    for (const auto& list : m_instructions) {
      int maxH = 0;
      for (auto it : list) {
        maxH = std::max(maxH, it->height());
      }
      h += maxH;
    }
    return h;
  }

 private:
  QList<InstructionPtrList> m_instructions;
};

class TextInstruction : public Instruction {
 public:
  TextInstruction(parser::PieceTableItem item, InstructionPainterConfig config)
      : Instruction(std::move(config)), m_item(item) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override;
  String textString(DocPtr doc) const;

 private:
  parser::PieceTableItem m_item;
};
class StaticTextInstruction : public Instruction {
 public:
  StaticTextInstruction(String text, InstructionPainterConfig config) : Instruction(std::move(config)), m_text(text) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override;

 private:
  String m_text;
};
class ImageInstruction : public Instruction {
 public:
  void run(Painter& painter, Point offset, DocPtr doc) const override;

 private:
  parser::PieceTableItem m_item;
};
class StaticImageInstruction : public Instruction {
 public:
  StaticImageInstruction(String path, InstructionPainterConfig config) : Instruction(std::move(config)), m_path(path) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override;

 private:
  String m_path;
};
class FillRectInstruction : public Instruction {
 public:
  explicit FillRectInstruction(InstructionPainterConfig config, bool lineHeight = true)
      : Instruction(std::move(config), lineHeight) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override;

 private:
};
class EllipseInstruction : public Instruction {
 public:
  explicit EllipseInstruction(InstructionPainterConfig config) : Instruction(std::move(config)) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override;
};
class LatexInstruction : public Instruction {
 public:
  LatexInstruction(String latex, InstructionPainterConfig config) : Instruction(config), m_latex(latex) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override;

 private:
  String m_latex;
};
}  // namespace md::render
#endif  // QTMARKDOWN_INSTRUCTION_H
