//
// Created by PikachuHy on 2021/11/6.
//

#ifndef QTMARKDOWN_CURSOR_H
#define QTMARKDOWN_CURSOR_H
#include "mddef.h"
namespace md::editor {
struct CursorCoord {
  // Block 下标
  SizeType blockNo{};
  // 逻辑行 下标
  SizeType lineNo{};
  // 逻辑行里 Cell 下标
  // 用于辅助判断键盘
  SizeType cellNo{};
  // 逻辑行里到列
  SizeType offset{};
};
QDebug operator<<(QDebug debug, const CursorCoord& c);
class Cursor {
 public:
  [[nodiscard]] CursorCoord coord() const { return m_coord; };
  void setCoord(CursorCoord coord);
  ;
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
