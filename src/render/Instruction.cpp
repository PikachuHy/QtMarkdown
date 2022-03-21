//
// Created by PikachuHy on 2021/11/5.
//

#include "Instruction.h"

#include "debug.h"
#include "microtex.h"
#include "parser/Text.h"
#include "graphic_qt.h"
namespace md::render {
void TextInstruction::run(Painter& painter, Point offset, DocPtr doc) const {
  Q_ASSERT(doc != nullptr);
  painter.save();
  painter.setFont(m_cell->m_font);
  painter.setPen(m_cell->m_fg);
  auto s = m_cell->m_text->toString(doc).mid(m_cell->m_offset, m_cell->m_length);
  Rect rect(m_cell->m_pos + offset, m_cell->m_size);
  painter.drawText(rect, s);
#if 0
  // debug用
  QRect debugRect(rect.x() - 1, rect.y() - 1, rect.width() + 2, rect.height() + 2);
  painter.drawRect(debugRect);
  //  DEBUG << s << rect << debugRect;
  auto font = painter.font();
  font.setPixelSize(6);
  painter.setFont(font);
  painter.setPen(Qt::red);
  painter.drawText(rect.x(), rect.y(), QString("%1, %2").arg(rect.width()).arg(rect.height()));
#endif
  painter.restore();
}
void StaticTextInstruction::run(Painter& painter, Point offset, DocPtr doc) const {
  Q_ASSERT(doc != nullptr);
  painter.save();
  painter.setPen(m_cell->m_fg);
  painter.setFont(m_cell->m_font);
  auto rect = Rect(m_cell->m_pos + offset, m_cell->m_size);
  painter.drawText(rect, m_cell->m_text);
#if 0
  // debug用
  QRect debugRect(rect.x() - 1, rect.y() - 1, rect.width() + 2, rect.height() + 2);
  painter.drawRect(debugRect);
  //  DEBUG << s << rect << debugRect;
  auto font = painter.font();
  font.setPixelSize(6);
  painter.setFont(font);
  painter.setPen(Qt::red);
  painter.drawText(rect.x(), rect.y(), QString("%1, %2").arg(rect.width()).arg(rect.height()));
#endif
  painter.restore();
}
void ImageInstruction::run(Painter& painter, Point offset, DocPtr doc) const {
  auto rect = Rect(m_cell->m_pos + offset, m_cell->m_size);
  painter.drawPixmap(rect, QPixmap(m_cell->m_path));
}
void StaticImageInstruction::run(Painter& painter, Point offset, DocPtr doc) const {
  auto rect = Rect(m_pos + offset, m_size);
  painter.drawPixmap(rect, QPixmap(m_path));
}
void FillRectInstruction::run(Painter& painter, Point offset, DocPtr doc) const {
  painter.save();
  auto rect = Rect(m_point + offset, m_size);
  painter.fillRect(rect, m_color);
  painter.restore();
}
void EllipseInstruction::run(Painter& painter, Point offset, DocPtr doc) const {
  painter.save();
  painter.setBrush(Brush(m_color));
  auto rect = Rect(m_point + offset, m_size);
  painter.drawEllipse(rect);
  painter.restore();
}
void LatexInstruction::run(Painter& painter, Point offset, DocPtr doc) const {
  painter.save();
  auto rect = Rect(m_cell->m_pos + offset, m_cell->m_size);
  try {
    float textSize = m_fontSize;
    auto render =
        microtex::MicroTeX::parse(m_latex.toStdString(), m_cell->m_size.width(), textSize, textSize / 3.f, 0xff424242);
    microtex::Graphics2D_qt g2(&painter);
    render->draw(g2, rect.x(), rect.y());
    delete render;
  } catch (const std::exception& ex) {
    qDebug() << "ERROR" << ex.what();
    painter.drawText(rect.x(), rect.y(), "Render LaTeX fail: " + m_latex);
  }
  painter.restore();
}
}  // namespace md::render
