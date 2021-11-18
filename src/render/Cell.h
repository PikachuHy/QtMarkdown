//
// Created by PikachuHy on 2021/11/17.
//

#ifndef QTMARKDOWN_CELL_H
#define QTMARKDOWN_CELL_H
#include <utility>

#include "mddef.h"
#include "parser/Text.h"
namespace md::render {

class Cell {
 public:
  Cell(Point pos, Size size) : m_pos(pos), m_size(size) {}
  // 逻辑长度
  [[nodiscard]] virtual SizeType length() = 0;
  // 像素宽度
  [[nodiscard]] int width() const { return m_size.width(); };
  // 像素高度
  [[nodiscard]] int height() const { return m_size.height(); };
  // 如果长度为length的子串，占用像素宽度
  [[nodiscard]] virtual int width(SizeType length, DocPtr doc) const = 0;

 protected:
  Point m_pos;
  Size m_size;
  friend class RenderPrivate;
  friend class LogicalLine;
  friend class VisualLine;
};
class TextInstruction;
class TextCell : public Cell {
 public:
  TextCell(parser::Text* text, SizeType offset, SizeType length, Point pos, Size size, const Color& fg,
           const Font& font)
      : Cell(pos, size), m_fg(fg), m_font(font), m_text(text), m_offset(offset), m_length(length) {}
  SizeType length() override;
  int width(SizeType length, DocPtr doc) const override;

 private:
  Color m_fg;
  Font m_font;
  parser::Text* m_text;
  SizeType m_offset;
  SizeType m_length;
  friend class TextInstruction;
  friend class LogicalLine;
};
class StaticTextInstruction;
class StaticTextCell : public Cell {
 public:
  StaticTextCell(String text, Point pos, const Size& size, const Color& fg, const Font& font)
      : Cell(pos, size), m_text(std::move(text)), m_fg(fg), m_font(font) {}
  SizeType length() override;
  int width(SizeType length, DocPtr doc) const override;

 private:
  String m_text;
  Color m_fg;
  Font m_font;
  friend class StaticTextInstruction;
};
class ImageInstruction;
class ImageCell : public Cell {
 public:
  ImageCell(parser::Image* node, String path, Point pos, Size size)
      : Cell(pos, size), m_node(node), m_path(std::move(path)) {}
  SizeType length() override;
  int width(SizeType length, DocPtr doc) const override;

 private:
  String m_path;
  parser::Image* m_node;
  friend class ImageInstruction;
};
}  // namespace md::render
#endif  // QTMARKDOWN_CELL_H
