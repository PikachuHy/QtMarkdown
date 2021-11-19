//
// Created by PikachuHy on 2021/11/6.
//

#ifndef QTMARKDOWN_CURSOR_H
#define QTMARKDOWN_CURSOR_H
#include "QtMarkdown_global.h"
#include "mddef.h"
#include "CursorCoord.h"
namespace md::editor {
class QTMARKDOWNSHARED_EXPORT Cursor {
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
