//
// Created by PikachuHy on 2021/11/5.
//

#include "Instruction.h"

#include <QPainter>

#include "debug.h"
#include "microtex.h"
#include "parser/Text.h"
#include "graphic_qt.h"
namespace md::render {
void TextInstruction::run(Painter& painter, Point offset, const parser::IBufferProvider& doc) const {
  painter.save();
  painter.setFont(m_cell->m_font);
  painter.setPen(m_cell->m_fg);
  auto s = m_cell->m_text->toString(doc).mid(m_cell->m_offset, m_cell->m_length);
  Rect rect(m_cell->m_pos + offset, m_cell->m_size);
  painter.drawText(rect, 0, s);
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
void LatexInstruction::run(Painter& painter, Point offset, const parser::IBufferProvider& /*doc*/) const {
  painter.save();
  auto rect = Rect(m_cell->m_pos + offset, m_cell->m_size);
  try {
    float textSize = m_fontSize;
    auto render =
        microtex::MicroTeX::parse(m_latex.toStdString(), m_cell->m_size.width, textSize, textSize / 3.f, 0xff424242);
    // Access QPainter* via nativePainter() bridge for microtex
    QPainter* qtPainter = static_cast<QPainter*>(painter.nativePainter());
    microtex::Graphics2D_qt g2(qtPainter);
    render->draw(g2, rect.x(), rect.y());
    delete render;
  } catch (const std::exception& ex) {
    DEBUG << "ERROR" << ex.what();
    painter.drawText(Point(rect.x(), rect.y()), String("Render LaTeX fail: ") + m_latex);
  }
  painter.restore();
}
}  // namespace md::render