//
// Created by pikachu on 2021/10/27.
//

#include "Cursor.h"
#include <QDebug>
#include "Render.h"
#include "EditorDocument.h"
#include "debug.h"
QDebug operator<<(QDebug debug, const CursorCoord &c)
{
  QDebugStateSaver saver(debug);
  debug.nospace() << '(' << c.lineNo << ", " << c.cellNo << ", " << c.offset << ')';

  return debug;
}
void Cursor::moveLeft(qsizetype length, bool ignoreCell) {
  if (!m_doc) return;
  m_doc->moveCursorLeft(this, length, ignoreCell);
}
void Cursor::moveRight(qsizetype length, bool ignoreCell) {
  if (!m_doc) return;
  m_doc->moveCursorRight(this, length, ignoreCell);
}
void Cursor::moveUp() {
  if (!m_doc) return;
  m_doc->moveCursorUp(this);
}
void Cursor::moveDown() {
  if (!m_doc) return;
  m_doc->moveCursorDown(this);
}
Cursor::Cursor() {
  m_x = 0;
  m_y = 0;
  m_h = 20;
  m_show = true;
  m_render = nullptr;
}
void Cursor::moveTo(QPoint pos) {
  m_x = pos.x();
  m_y = pos.y();
}
void Cursor::moveTo(int x, int y) {
  m_x = x;
  m_y= y;
}
void Cursor::moveX(int x) {
  m_x = x;
}
void Cursor::moveY(int y) {
  m_y = y;
}
void Cursor::updateHeight(int h) {
  m_h = h;
}
void Cursor::draw(QPainter &painter) {
  if (m_show) {
    painter.drawRect(m_x, m_y, 1, m_h);
  }
  m_show = !m_show;
}
QPoint Cursor::pos() const {
  return QPoint(m_x, m_y);
}
QRect Cursor::region() const {
  return QRect(m_x, m_y, 1, m_h);
}
void Cursor::setLineNo(qsizetype lineNo) {
  DEBUG << m_coord << "->" << lineNo;
  m_coord.lineNo = lineNo;
}
void Cursor::setCellNo(qsizetype cellNo) { m_coord.cellNo = cellNo; }
void Cursor::setOffset(qsizetype offset) {
  m_coord.offset = offset;
}
qsizetype Cursor::lineNo() const { return m_coord.lineNo; }
qsizetype Cursor::cellNo() const { return m_coord.cellNo; }
qsizetype Cursor::offset() const { return m_coord.offset; }
void Cursor::setRender(Render *render) {
  m_render = render;
}
void Cursor::setNeedUpdateCoord(bool flag) {
  m_needUpdateCoord = flag;
}
bool Cursor::needUpdateCoord() { return m_needUpdateCoord; }
CursorCoord Cursor::coord() { return m_coord; }
void Cursor::setCursorCoord(CursorCoord coord) {
  m_coord = coord;
}
int Cursor::x() { return m_x; }
int Cursor::y() { return m_y; }
void Cursor::setEditorDocument(EditorDocument* doc) {
  m_doc = doc;
}

void Cursor::insertText(QString text) {
    m_doc->insertText(text, *this);
}
void Cursor::removeText(qsizetype length) {
  m_doc->removeText(length, *this);
}
