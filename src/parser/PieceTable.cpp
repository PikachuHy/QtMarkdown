//
// Created by PikachuHy on 2021/11/2.
//

#include "PieceTable.h"

#include "Document.h"
#include "debug.h"
namespace md::parser {
String PieceTableItem::toString(const DocPtr& doc) const {
  auto s = bufferType == original ? doc->m_originalBuffer.mid(offset, length) : doc->m_addBuffer.mid(offset, length);
  if (s.endsWith("\n")) {
    DEBUG << "换行";
  }
  return s;
}
QDebug operator<<(QDebug debug, const PieceTableItem& item) {
  QDebugStateSaver saver(debug);
  debug.nospace() << '(' << (item.bufferType == PieceTableItem::original ? "original" : "add") << ", " << item.offset
                  << ", " << item.length << ')';

  return debug;
}
}  // namespace md::parser