//
// Created by PikachuHy on 2021/11/2.
//

#ifndef QTMARKDOWN_PIECETABLE_H
#define QTMARKDOWN_PIECETABLE_H
#include "QtMarkdown_global.h"
#include "IBufferProvider.h"
#include "mddef.h"
namespace md::parser {
class QTMARKDOWNSHARED_EXPORT PieceTableItem {
 public:
  enum BufferType { original, add };
  BufferType bufferType;
  SizeType offset;
  SizeType length;
  [[nodiscard]] String toString(const IBufferProvider& doc) const;
};
std::ostream& operator<<(std::ostream& os, const PieceTableItem& item);
}  // namespace md::parser
#endif  // QTMARKDOWN_PIECETABLE_H
