//
// Created by PikachuHy on 2021/11/6.
//

#ifndef QTMARKDOWN_CURSOR_H
#define QTMARKDOWN_CURSOR_H
#include "QtMarkdown_global.h"
#include "render/mddef.h"
#include "CursorCoord.h"
#include "core/Types.h"
namespace md::editor {
class QTMARKDOWNSHARED_EXPORT Cursor {
 public:
  [[nodiscard]] CursorCoord coord() const { return m_coord; };
  void setCoord(CursorCoord coord);
  int x() const { return m_pos.x; }
  void setX(int x) { m_pos.x = x; };
  int y() const { return m_pos.y; }
  [[nodiscard]] core::Point pos() const { return m_pos; }
  void setPos(core::Point pos) { m_pos = pos; }
  [[nodiscard]] int height() const { return m_h; }
  void setHeight(int h) { m_h = h; }
  [[nodiscard]] int ascent() const { return m_ascent; }
  void setAscent(int a) { m_ascent = a; }

 private:
  CursorCoord m_coord{};
  core::Point m_pos{};
  int m_h = 20;
  int m_ascent = 0;
};
class QTMARKDOWNSHARED_EXPORT SelectionRange {
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
