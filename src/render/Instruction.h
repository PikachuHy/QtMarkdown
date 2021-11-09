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
// 仅用来占位
class DummyInstruction : public Instruction {
 public:
  DummyInstruction(InstructionPainterConfig config, bool lineHeight = true) : Instruction(config, lineHeight) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override {}
};
class Cell {
 public:
  Cell(Font font, Point pos, bool bol, bool eol) : m_font(font), m_pos(pos), m_bol(bol), m_eol(eol) {}
  [[nodiscard]] Font font() const { return m_font; }
  [[nodiscard]] Point pos() const { return m_pos; }
  [[nodiscard]] bool bol() const { return m_bol; }
  void setEol(bool eol) { m_eol = eol; }
  [[nodiscard]] bool eol() const { return m_eol; }
  void setRect(Rect rect) { m_rect = rect; }
  virtual bool isTextCell() const { return false; };
  virtual bool isStaticTextCell() const { return false; }
  virtual SizeType length() = 0;
  virtual int width(DocPtr doc) const { return 0; }
  virtual int height() const { return 0; }

 protected:
  Font m_font;
  Point m_pos;
  // 视觉行开始
  bool m_bol;
  // 视觉行结束
  bool m_eol;
  Rect m_rect;
};
class TextCell : public Cell {
 public:
  TextCell(parser::Text* text, SizeType offset, SizeType length, Font font, Point pos, bool bol, bool eol)
      : Cell(font, pos, bol, eol), m_text(text), m_offset(offset), m_length(length) {}
  [[nodiscard]] auto text() const { return m_text; }
  [[nodiscard]] auto offset() const { return m_offset; }
  SizeType length() override { return m_length; }
  [[nodiscard]] bool isTextCell() const override { return true; }
  int width(DocPtr doc) const override;
  [[nodiscard]] int height() const override { return m_rect.height(); }
  String toString(DocPtr doc) const;

 private:
  parser::Text* m_text;
  SizeType m_offset;
  SizeType m_length;
};
struct StaticTextCell : public Cell {
 public:
  StaticTextCell(String text, Font font, Point pos, bool bol, bool eol) : Cell(font, pos, bol, eol), m_text(text) {}

  SizeType length() override { return 0; }
  [[nodiscard]] bool isStaticTextCell() const override { return true; }
  [[nodiscard]] int width() const {
    QFontMetrics fm(m_font);
    return fm.horizontalAdvance(m_text);
  }

 private:
  String m_text;
};
using VisualItem = InstructionPtr;
using LogicalItem = Cell*;
using VisualLine = InstructionPtrList;
class LogicalLine {
 public:
  LogicalLine(int x, int h) : m_x(x), m_h(h) {}
  LogicalItem& operator[](SizeType index);
  const LogicalItem& operator[](SizeType index) const { return m_items[index]; }
  [[nodiscard]] bool empty() const { return m_items.empty(); }
  void push_back(LogicalItem item) { m_items.push_back(item); }
  LogicalItem front();
  LogicalItem back();
  [[nodiscard]] SizeType size() const { return m_items.size(); }
  [[nodiscard]] auto begin() const { return m_items.begin(); }
  [[nodiscard]] auto end() const { return m_items.end(); }
  [[nodiscard]] int height() const;
  void setX(int x) { m_x = x; }
  [[nodiscard]] int x() const { return m_x; }
  void setHeight(int h) { m_h = h; }

 private:
  std::vector<LogicalItem> m_items;
  // 存放在视觉行的x值，在逻辑Item是空的时候有用
  int m_x{};
  int m_h{};
};
class Block {
 public:
  explicit Block(parser::Node* node) : m_node(node) {}
  void newVisualLine() { m_visualLines.push_back(VisualLine()); }
  void newLogicalLine(int x, int h) { m_logicalLines.push_back(LogicalLine(x, h)); }
  void appendVisualItem(VisualItem item);
  void appendLogicalItem(LogicalItem item);
  [[nodiscard]] int height() const {
    int h = 0;
#if 0
    for (const auto& list : m_visualLines) {
      int maxH = 0;
      for (auto it : list) {
        maxH = std::max(maxH, it->height());
      }
      h += maxH;
    }
#endif
    for (const auto& list : m_logicalLines) {
      h += list.height();
    }
    return h;
  }
  [[nodiscard]] SizeType countOfVisualLine() const { return m_visualLines.size(); }
  [[nodiscard]] SizeType countOfLogicalLine() const { return m_logicalLines.size(); }
  [[nodiscard]] SizeType countOfVisualItem(SizeType indexOfLine) const;
  [[nodiscard]] SizeType countOfLogicalItem(SizeType indexOfLine) const;
  [[nodiscard]] const VisualItem& visualItemAt(SizeType indexOfLine, SizeType indexOfItem) const;
  void insertVisualItem(SizeType indexOfLine, SizeType indexOfItem, VisualItem item);
  void insertNewVisualLineAt(SizeType index, Instruction* instruction) {
    m_visualLines.insert(index, InstructionPtrList());
    m_visualLines[index].push_back(instruction);
  }
  [[nodiscard]] const QList<VisualLine>& visualLines() const { return m_visualLines; }
  [[nodiscard]] const QList<LogicalLine>& logicalLines() const { return m_logicalLines; }
  [[nodiscard]] QList<LogicalLine>& logicalLines() { return m_logicalLines; }
  [[nodiscard]] SizeType maxOffsetOfLogicalLine(SizeType index) const;

 private:
  // 视觉行
  QList<VisualLine> m_visualLines;
  // 逻辑行
  QList<LogicalLine> m_logicalLines;
  // 方便调试用
  parser::Node* m_node;
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
  StaticTextInstruction(String text, InstructionPainterConfig config, bool lineHeight = true)
      : Instruction(std::move(config), lineHeight), m_text(text) {}
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
