//
// Created by PikachuHy on 2021/11/6.
//

#include "Cursor.h"

#include "debug.h"
namespace md::editor {
QDebug operator<<(QDebug debug, const CursorCoord &c) {
  QDebugStateSaver saver(debug);
  debug.nospace() << '(' << c.blockNo << ", " << c.lineNo << ", " << c.cellNo << ", " << c.offset << ')';

  return debug;
}
void Cursor::setCoord(CursorCoord coord) {
  DEBUG << coord;
  m_coord = coord;
}
}  // namespace md::editor