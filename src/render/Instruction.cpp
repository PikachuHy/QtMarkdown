//
// Created by PikachuHy on 2021/11/5.
//

#include "Instruction.h"

#include "debug.h"
#include "latex.h"
#include "platform/qt/graphic_qt.h"
namespace md::render {
void TextInstruction::run(Painter& painter, Point offset, DocPtr doc) const {
  Q_ASSERT(doc != nullptr);
  painter.save();
  painter.setFont(m_config.font);
  painter.setPen(m_config.pen);
  auto rect = updateRect(m_config.rect, offset);
  auto s = m_item.toString(doc);
  painter.drawText(rect, s);
  painter.restore();
}
void ImageInstruction::run(Painter& painter, Point offset, DocPtr doc) const {}
void StaticImageInstruction::run(Painter& painter, Point offset, DocPtr doc) const {
  auto rect = updateRect(m_config.rect, offset);
  painter.drawPixmap(rect, QPixmap(m_path));
}
void StaticTextInstruction::run(Painter& painter, Point offset, DocPtr doc) const {
  Q_ASSERT(doc != nullptr);
  painter.save();
  painter.setPen(m_config.pen);
  painter.setFont(m_config.font);
  auto rect = updateRect(m_config.rect, offset);
  painter.drawText(rect, m_text);
  painter.restore();
}
void FillRectInstruction::run(Painter& painter, Point offset, DocPtr doc) const {
  painter.save();
  auto rect = updateRect(m_config.rect, offset);
  painter.fillRect(rect, m_config.brush);
  painter.restore();
}
void EllipseInstruction::run(Painter& painter, Point offset, DocPtr doc) const {
  painter.save();
  painter.setBrush(m_config.brush);
  auto rect = updateRect(m_config.rect, offset);
  painter.drawEllipse(rect);
  painter.restore();
}
void LatexInstruction::run(Painter& painter, Point offset, DocPtr doc) const {
  painter.save();
  auto rect = updateRect(m_config.rect, offset);
  try {
    float textSize = m_config.font.pixelSize();
    auto render = tex::LaTeX::parse(m_latex.toStdString(), m_config.rect.width(), textSize, textSize / 3.f, 0xff424242);
    tex::Graphics2D_qt g2(&painter);
    render->draw(g2, rect.x(), rect.y());
    delete render;
  } catch (const std::exception& ex) {
    qDebug() << "ERROR" << ex.what();
    painter.drawText(rect.x(), rect.y(), "Render LaTeX fail: " + m_latex);
  }
  painter.restore();
}
Rect Instruction::updateRect(Rect rect, Point offset) {
  int x = offset.x() + rect.x();
  int y = offset.y() + rect.y();
  return {x, y, rect.width(), rect.height()};
}
}  // namespace md::render
