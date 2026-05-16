//
// Created by PikachuHy on 2021/11/19.
//

#ifndef QTMARKDOWN_CURSORCOORD_H
#define QTMARKDOWN_CURSORCOORD_H
#include "QtMarkdown_global.h"
#include "render/mddef.h"
namespace md::editor {
class QTMARKDOWNEDITORCORE_EXPORT CursorCoord {
 public:
  // Block 下标
  SizeType blockNo = 0;
  // 逻辑行 下标
  SizeType lineNo = 0;
  // 逻辑行里到列
  SizeType offset = 0;
  bool operator<(const CursorCoord& rhs) const;
  bool operator>(const CursorCoord& rhs) const;
  bool operator<=(const CursorCoord& rhs) const;
  bool operator>=(const CursorCoord& rhs) const;
  bool operator==(const CursorCoord& rhs) const;
  bool operator!=(const CursorCoord& rhs) const;
};
QTMARKDOWNEDITORCORE_EXPORT std::ostream& operator<<(std::ostream& os, const CursorCoord& c);
}  // namespace md::editor

#endif  // QTMARKDOWN_CURSORCOORD_H
