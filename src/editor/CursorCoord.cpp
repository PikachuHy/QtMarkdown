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
}  // namespace md::editor