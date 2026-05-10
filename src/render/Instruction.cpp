//
// Created by PikachuHy on 2021/11/5.
//

#include "Instruction.h"

#include "FontMetricsProvider.h"
#include "debug.h"
#include "parser/Text.h"
namespace md::render {
void TextInstruction::run(Painter& painter, Point offset, const parser::IBufferProvider& doc) const {
  painter.save();
  painter.setFont(m_cell->m_font);
  painter.setPen(m_cell->m_fg);
  auto s = m_cell->m_text->toString(doc).mid(m_cell->m_offset, m_cell->m_length);
  auto pt = m_cell->m_pos + offset;
  pt.y += m_cell->ascent();
  painter.drawText(pt, s);
  painter.restore();
}
void StaticTextInstruction::run(Painter& painter, Point offset, const parser::IBufferProvider& /*doc*/) const {
  painter.save();
  painter.setPen(m_fg);
  painter.setFont(m_font);
  auto rect = Rect(m_pos + offset, m_size);
  painter.drawText(rect, 0, m_text);
  painter.restore();
}
void ImageInstruction::run(Painter& painter, Point offset, const parser::IBufferProvider& /*doc*/) const {
  auto rect = Rect(m_pos + offset, m_size);
  painter.drawImage(rect, m_image);
}
void StaticImageInstruction::run(Painter& painter, Point offset, const parser::IBufferProvider& /*doc*/) const {
  auto rect = Rect(m_pos + offset, m_size);
  painter.drawImage(rect, m_image);
}
void FillRectInstruction::run(Painter& painter, Point offset, const parser::IBufferProvider& /*doc*/) const {
  painter.save();
  auto rect = Rect(m_point + offset, m_size);
  painter.fillRect(rect, m_color);
  painter.restore();
}
void EllipseInstruction::run(Painter& painter, Point offset, const parser::IBufferProvider& /*doc*/) const {
  painter.save();
  auto rect = Rect(m_point + offset, m_size);
  painter.drawEllipse(rect, m_color);
  painter.restore();
}
void LatexInstruction::run(Painter& painter, Point offset, const parser::IBufferProvider&) const {
  painter.save();
  Rect rect(m_cell->m_pos + offset, m_cell->m_size);
  painter.drawLatex(rect, m_latex, m_fontSize);
  painter.restore();
}
}  // namespace md::render