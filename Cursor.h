//
// Created by pikachu on 2021/10/27.
//

#ifndef QTMARKDOWN_CURSOR_H
#define QTMARKDOWN_CURSOR_H
#include <QRect>
#include <QPainter>
#include <QDebug>
#define DEBUG qDebug().noquote() << "[debug]" << __FUNCTION__ << QString(__FILE_NAME__) + QString(":") + QString::number(__LINE__)
class Render;
struct CursorCoord {
  qsizetype lineNo{};
  qsizetype cellNo{};
  qsizetype offset{};
};
QDebug operator<<(QDebug debug, const CursorCoord &c);
class Cursor {
public:
  Cursor();
  ~Cursor() = default;
  void moveTo(QPoint pos);
  void moveTo(int x, int y);
  void moveX(int x);
  void moveY(int y);
  void updateHeight(int h);
  void draw(QPainter& painter);
  int x();
  int y();
  QPoint pos() const;
  QRect region() const;
  void moveLeft();
  void moveRight();
  void moveUp();
  void moveDown();
  void setLineNo(qsizetype lineNo);
  [[nodiscard]] qsizetype lineNo() const;
  void setCellNo(qsizetype cellNo);
  [[nodiscard]] qsizetype cellNo() const;
  void setOffset(qsizetype offset);
  [[nodiscard]] qsizetype offset() const;
  void setRender(Render* render);
  void setNeedUpdateCoord(bool flag);
  bool needUpdateCoord();
  CursorCoord coord();
  void setCursorCoord(CursorCoord coord);
private:
  int m_x;
  int m_y;
  int m_h;
  bool m_show;
  CursorCoord m_coord;
  Render* m_render;
  bool m_needUpdateCoord;
};

#endif // QTMARKDOWN_CURSOR_H
