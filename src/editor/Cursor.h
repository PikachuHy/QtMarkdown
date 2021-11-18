//
// Created by PikachuHy on 2021/11/6.
//

#ifndef QTMARKDOWN_CURSOR_H
#define QTMARKDOWN_CURSOR_H
#include "mddef.h"
namespace md::editor {
struct CursorCoord {
  // Block 下标
  SizeType blockNo = 0;
  // 逻辑行 下标
  SizeType lineNo = 0;
  // 逻辑行里到列
  SizeType offset = 0;
};
QDebug operator<<(QDebug debug, const CursorCoord& c);
class Cursor {
 public:
  [[nodiscard]] CursorCoord coord() const { return m_coord; };
  void setCoord(CursorCoord coord);
  void setX(int x) { m_pos.setX(x); };
  [[nodiscard]] Point pos() const { return m_pos; }
  void setPos(Point pos) { m_pos = pos; }
  [[nodiscard]] int height() const { return m_h; }
  void setHeight(int h) { m_h = h; }

 private:
  CursorCoord m_coord{};
  Point m_pos{};
  int m_h = 20;
};
}  // namespace md::editor
#endif  // QTMARKDOWN_CURSOR_H
