//
// Created by PikachuHy on 2021/11/5.
//

#include "Instruction.h"

#include "debug.h"
#include "latex.h"
#include "parser/Text.h"
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
  // debug用
  QRect debugRect(rect.x() - 1, rect.y() - 1, rect.width() + 2, rect.height() + 2);
  painter.drawRect(debugRect);
  //  DEBUG << s << rect << debugRect;
  auto font = painter.font();
  font.setPixelSize(6);
  painter.setFont(font);
  painter.setPen(Qt::red);
  painter.drawText(rect.x(), rect.y(), QString("%1, %2").arg(rect.width()).arg(rect.height()));
  painter.restore();
}
String TextInstruction::textString(DocPtr doc) const { return m_item.toString(doc); }
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
SizeType Block::maxOffsetOfLogicalLine(SizeType index) const {
  if (m_logicalLines.empty()) return 0;
  ASSERT(index >= 0 && index < m_logicalLines.size());
  auto line = m_logicalLines[index];
  SizeType totalOffset = 0;
  for (auto cell : line) {
    if (!cell->isTextCell()) continue;
    auto textCell = (TextCell*)cell;
    totalOffset += textCell->length();
  }
  return totalOffset;
}
void Block::appendVisualItem(VisualItem item) {
  ASSERT(!m_visualLines.empty());
  m_visualLines.back().push_back(item);
}
void Block::appendLogicalItem(LogicalItem item) {
  ASSERT(!m_logicalLines.empty());
  m_logicalLines.back().push_back(item);
}
SizeType Block::countOfVisualItem(SizeType indexOfLine) const {
  ASSERT(indexOfLine >= 0 && indexOfLine < m_visualLines.size());
  return m_visualLines[indexOfLine].size();
}
const VisualItem& Block::visualItemAt(SizeType indexOfLine, SizeType indexOfItem) const {
  ASSERT(indexOfLine >= 0 && indexOfLine < m_visualLines.size());
  ASSERT(indexOfItem >= 0 && indexOfItem < m_visualLines[indexOfLine].size());
  return m_visualLines[indexOfLine][indexOfItem];
}
void Block::insertVisualItem(SizeType indexOfLine, SizeType indexOfItem, VisualItem item) {
  ASSERT(indexOfLine >= 0 && indexOfLine < m_visualLines.size());
  ASSERT(indexOfItem >= 0 && indexOfItem < m_visualLines[indexOfLine].size());
  auto& line = m_visualLines[indexOfLine];
  line.insert(line.begin() + indexOfItem, item);
}
SizeType Block::countOfLogicalItem(SizeType indexOfLine) const {
  if (m_logicalLines.empty()) return 0;
  ASSERT(indexOfLine >= 0 && indexOfLine < m_logicalLines.size());
  auto line = m_logicalLines[indexOfLine];
  if (line.empty()) return 0;
  return line.size();
}
int TextCell::width(DocPtr doc) const {
  auto s = m_text->toString(doc).mid(m_offset, m_length);
  QFontMetrics fm(m_font);
  return fm.horizontalAdvance(s);
}

String TextCell::toString(DocPtr doc) const { return m_text->toString(doc).mid(m_offset, m_length); }
LogicalItem LogicalLine::front() {
  ASSERT(!m_items.empty());
  return m_items.front();
}
LogicalItem LogicalLine::back() {
  ASSERT(!m_items.empty());
  return m_items.back();
}
int LogicalLine::height() const {
  if (m_items.empty()) {
    return m_h;
  }
  std::vector<int> hs;
  std::vector<SizeType> startIndex;
  int curMaxH = 0;
  for (SizeType i = 0; i < m_items.size(); ++i) {
    auto cell = m_items[i];
    curMaxH = std::max(curMaxH, cell->height());
    if (cell->eol()) {
      hs.push_back(curMaxH);
      curMaxH = 0;
    }
    if (cell->bol()) {
      startIndex.push_back(i);
    }
  }
  ASSERT(!m_items.empty());
  if (!m_items.back()->eol()) {
    hs.push_back(curMaxH);
  }
  ASSERT(hs.size() == startIndex.size());
  int h = 0;
  for (int i = 0; i < hs.size(); ++i) {
    h += hs[i];
  }
  return h;
}
LogicalItem& LogicalLine::operator[](SizeType index) {
  ASSERT(index >= 0 && index < m_items.size());
  return m_items[index];
}
}  // namespace md::render
