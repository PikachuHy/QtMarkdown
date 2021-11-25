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
  int x() const { return m_pos.x(); }
  void setX(int x) { m_pos.setX(x); };
  int y() const { return m_pos.y(); }
  [[nodiscard]] Point pos() const { return m_pos; }
  void setPos(Point pos) { m_pos = pos; }
  [[nodiscard]] int height() const { return m_h; }
  void setHeight(int h) { m_h = h; }

 private:
  CursorCoord m_coord{};
  Point m_pos{};
  int m_h = 20;
};
class SelectionRange {
 public:
  Cursor caret;
  Cursor anchor;
  std::pair<Cursor, Cursor> range() const {
    if (caret.coord() < anchor.coord()) {
      return {caret, anchor};
    } else {
      return {anchor, caret};
    }
  }
};
}  // namespace md::editor
#endif  // QTMARKDOWN_CURSOR_H
