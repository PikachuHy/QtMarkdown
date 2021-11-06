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

 private:
 protected:
  static Rect updateRect(Rect rect, Point offset);

 protected:
  InstructionPainterConfig m_config;
  bool m_lineH;
};
class InstructionGroup {
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
