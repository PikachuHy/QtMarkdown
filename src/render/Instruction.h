//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_INSTRUCTION_H
#define QTMARKDOWN_INSTRUCTION_H
#include <QPainter>
#include <QRect>
#include <utility>
#include <vector>
#include "QtMarkdown_global.h"
#include "mddef.h"
#include "parser/PieceTable.h"
#include "Cell.h"
namespace md::parser {
class Text;
}
namespace md::render {
class RenderPrivate;
// 基本绘图指令
class QTMARKDOWNSHARED_EXPORT Instruction {
 public:
  virtual void run(Painter& painter, Point offset, DocPtr doc) const = 0;
};
class QTMARKDOWNSHARED_EXPORT TextInstruction : public Instruction {
 public:
  TextInstruction(TextCell* cell) : m_cell(cell) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override;
  String textString(DocPtr doc) const;

 private:
  TextCell* m_cell;
};

class QTMARKDOWNSHARED_EXPORT StaticTextInstruction : public Instruction {
 public:
  StaticTextInstruction(StaticTextCell* cell) : m_cell(cell) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override;

 private:
  StaticTextCell* m_cell;
};
class QTMARKDOWNSHARED_EXPORT ImageInstruction : public Instruction {
 public:
  ImageInstruction(ImageCell* cell) : m_cell(cell) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override;

 private:
  ImageCell* m_cell;
};
class QTMARKDOWNSHARED_EXPORT StaticImageInstruction : public Instruction {
 public:
  StaticImageInstruction(String path, Point pos, Size size) : m_path(std::move(path)), m_pos(pos), m_size(size) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override;

 private:
  String m_path;
  Point m_pos;
  Size m_size;
};
class QTMARKDOWNSHARED_EXPORT FillRectInstruction : public Instruction {
 public:
  explicit FillRectInstruction(Point point, Size size, Color color) : m_point(point), m_size(size), m_color(color) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override;

 private:
  Point m_point;
  Size m_size;
  Color m_color;
};
class QTMARKDOWNSHARED_EXPORT EllipseInstruction : public Instruction {
 public:
  explicit EllipseInstruction(Point point, Size size, Color color) : m_point(point), m_size(size), m_color(color) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override;

 private:
  Point m_point;
  Size m_size;
  Color m_color;
};
class QTMARKDOWNSHARED_EXPORT LatexInstruction : public Instruction {
 public:
  LatexInstruction(Point pos, Size size, String latex, int fontSize)
      : m_pos(pos), m_size(size), m_latex(std::move(latex)), m_fontSize(fontSize) {}
  void run(Painter& painter, Point offset, DocPtr doc) const override;

 private:
  Point m_pos;
  Size m_size;
  String m_latex;
  int m_fontSize;
};
}  // namespace md::render
#endif  // QTMARKDOWN_INSTRUCTION_H
