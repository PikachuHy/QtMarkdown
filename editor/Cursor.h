//
// Created by pikachu on 2021/10/27.
//

#ifndef QTMARKDOWN_CURSOR_H
#define QTMARKDOWN_CURSOR_H
#include <QRect>
#include <QPainter>
class Render;
class EditorDocument;
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
  int h() const { return m_h; };
  QPoint pos() const;
  QRect region() const;
  void moveLeft(qsizetype length = 1, bool ignoreCell = false);
  void moveRight(qsizetype length = 1, bool ignoreCell = false);
  void moveUp();
  void moveDown();
  void setLineNo(qsizetype lineNo);
  [[nodiscard]] qsizetype lineNo() const;
  void setCellNo(qsizetype cellNo);
  [[nodiscard]] qsizetype cellNo() const;
  void setOffset(qsizetype offset);
  [[nodiscard]] qsizetype offset() const;
  void setRender(Render* render);
  void setEditorDocument(EditorDocument* doc);
  void setNeedUpdateCoord(bool flag);
  bool needUpdateCoord();
  CursorCoord coord();
  void setCursorCoord(CursorCoord coord);
  void insertText(QString text);
  void removeText(qsizetype length=1);
private:
  int m_x;
  int m_y;
  int m_h;
  bool m_show;
  CursorCoord m_coord;
  Render* m_render;
  EditorDocument* m_doc;
  bool m_needUpdateCoord;
};

#endif // QTMARKDOWN_CURSOR_H
