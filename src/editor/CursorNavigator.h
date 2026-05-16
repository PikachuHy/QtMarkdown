//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_CURSORNAVIGATOR_H
#define QTMARKDOWN_CURSORNAVIGATOR_H
#include "CursorCoord.h"
#include "render/mddef.h"
#include "render/Render.h"
#include "parser/IBufferProvider.h"
#include "parser/Node.h"

namespace md::editor {
class Cursor;

class CursorNavigator {
 public:
  CursorNavigator(const render::BlockList& blocks, const parser::IBufferProvider& doc,
                  const parser::Container& root, const render::RenderSetting& setting)
      : m_blocks(blocks), m_doc(doc), m_root(root), m_setting(setting) {}

  CursorCoord moveCursorToRight(CursorCoord coord);
  CursorCoord moveCursorToLeft(CursorCoord coord);
  CursorCoord moveCursorToBol(CursorCoord coord);
  std::pair<CursorCoord, int> moveCursorToEol(CursorCoord coord);
  CursorCoord moveCursorToUp(CursorCoord coord, core::Point pos);
  CursorCoord moveCursorToDown(CursorCoord coord, core::Point pos);
  CursorCoord moveCursorToPos(core::Point pos);
  CursorCoord moveCursorToBeginOfDocument();
  CursorCoord moveCursorToEndOfDocument();
  bool isBol(const CursorCoord& coord) const;

  std::tuple<core::Point, int, int> mapToScreen(const CursorCoord& coord);
  void updateCursor(Cursor& cursor, const CursorCoord& coord, bool updatePos = true);

 private:
  const render::BlockList& m_blocks;
  const parser::IBufferProvider& m_doc;
  const parser::Container& m_root;
  const render::RenderSetting& m_setting;
};

}  // namespace md::editor

#endif  // QTMARKDOWN_CURSORNAVIGATOR_H
