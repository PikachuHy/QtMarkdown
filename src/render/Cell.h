//
// Created by PikachuHy on 2021/11/17.
//

#ifndef QTMARKDOWN_CELL_H
#define QTMARKDOWN_CELL_H
#include "QtMarkdown_global.h"
#include <utility>

#include "debug.h"
#include "mddef.h"
#include "parser/IBufferProvider.h"
#include "parser/Text.h"
namespace md::render {
class IFontMetricsProvider;

class QTMARKDOWNRENDER_EXPORT Cell {
 public:
  Cell(Point pos, Size size) : m_pos(pos), m_size(size) {}
  virtual ~Cell() = default;
  // 逻辑长度
  [[nodiscard]] virtual SizeType length() = 0;
  // 像素宽度
  [[nodiscard]] int width() const { return m_size.width; };
  // 像素高度
  [[nodiscard]] int height() const { return m_size.height; };
  // 如果长度为length的子串，占用像素宽度
  [[nodiscard]] virtual int width(SizeType length, const parser::IBufferProvider& doc) const = 0;
  // 返回关联的Text节点和在此cell内的偏移（非TextCell返回nullptr/0）
  [[nodiscard]] virtual parser::Text* textNode() const { return nullptr; }
  [[nodiscard]] virtual SizeType textOffset() const { return 0; }
  [[nodiscard]] virtual int ascent() const { return 0; }

 protected:
  Point m_pos;
  Size m_size;
  friend class LayoutPass;
  friend class LogicalLine;
  friend class VisualLine;
};
class TextInstruction;
class QTMARKDOWNRENDER_EXPORT TextCell : public Cell {
 public:
  TextCell(parser::Text* text, SizeType offset, SizeType length, Point pos, Size size, const Color& fg,
           const Font& font, IFontMetricsProvider* fm)
      : Cell(pos, size), m_fg(fg), m_font(font), m_text(text), m_offset(offset), m_length(length), m_fm(fm) {
    ASSERT(fm != nullptr);
  }
  SizeType length() override;
  int width(SizeType length, const parser::IBufferProvider& doc) const override;
  parser::Text* textNode() const override { return m_text; }
  SizeType textOffset() const override { return m_offset; }
  int ascent() const override;
  parser::Text* text() const { return m_text; }

 private:
  Color m_fg;
  Font m_font;
  parser::Text* m_text;
  SizeType m_offset;
  SizeType m_length;
  IFontMetricsProvider* m_fm;
  friend class TextInstruction;
  friend class LogicalLine;
};
class LatexInstruction;
class QTMARKDOWNRENDER_EXPORT InlineLatexCell : public Cell {
 public:
  InlineLatexCell(Point pos, Size size) : Cell(pos, size) {}
  SizeType length() override { return 1; }
  int width(SizeType length, const parser::IBufferProvider& doc) const override;

 private:
  friend class LatexInstruction;
};
}  // namespace md::render
#endif  // QTMARKDOWN_CELL_H
