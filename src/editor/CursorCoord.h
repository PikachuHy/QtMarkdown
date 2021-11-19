//
// Created by PikachuHy on 2021/11/19.
//

#ifndef QTMARKDOWN_CURSORCOORD_H
#define QTMARKDOWN_CURSORCOORD_H
#include "CursorCoord.h"
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
}  // namespace md::editor

#endif  // QTMARKDOWN_CURSORCOORD_H
