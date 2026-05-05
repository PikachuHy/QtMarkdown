//
// Created by PikachuHy on 2021/11/2.
//

#include "PieceTable.h"

#include "Document.h"
#include "IBufferProvider.h"
#include "debug.h"
namespace md::parser {
String PieceTableItem::toString(const IBufferProvider& doc) const {
  auto s = bufferType == original ? doc.originalBuffer().mid(offset, length) : doc.addBuffer().mid(offset, length);
  if (s.endsWith("\n")) {
    DEBUG << "换行";
  }
  return s;
}
std::ostream& operator<<(std::ostream& os, const PieceTableItem& item) {
  os << '(' << (item.bufferType == PieceTableItem::original ? "original" : "add") << ", " << item.offset
      << ", " << item.length << ')';
  return os;
}
}  // namespace md::parser