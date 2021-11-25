//
// Created by PikachuHy on 2021/11/19.
//

#include "CursorCoord.h"

#include "debug.h"
namespace md::editor {
QDebug operator<<(QDebug debug, const CursorCoord &c) {
  QDebugStateSaver saver(debug);
  debug.nospace() << '(' << c.blockNo << ", " << c.lineNo << ", " << c.offset << ')';

  return debug;
}
bool CursorCoord::operator<(const CursorCoord &rhs) const {
  if (blockNo < rhs.blockNo) return true;
  if (rhs.blockNo < blockNo) return false;
  if (lineNo < rhs.lineNo) return true;
  if (rhs.lineNo < lineNo) return false;
  return offset < rhs.offset;
}
bool CursorCoord::operator>(const CursorCoord &rhs) const { return rhs < *this; }
bool CursorCoord::operator<=(const CursorCoord &rhs) const { return !(rhs < *this); }
bool CursorCoord::operator>=(const CursorCoord &rhs) const { return !(*this < rhs); }
bool CursorCoord::operator==(const CursorCoord &rhs) const {
  return blockNo == rhs.blockNo && lineNo == rhs.lineNo && offset == rhs.offset;
}
bool CursorCoord::operator!=(const CursorCoord &rhs) const { return !(rhs == *this); }
}  // namespace md::editor