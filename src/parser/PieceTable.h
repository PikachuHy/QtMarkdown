//
// Created by PikachuHy on 2021/11/2.
//

#ifndef QTMARKDOWN_PIECETABLE_H
#define QTMARKDOWN_PIECETABLE_H

#include "mddef.h"
namespace md::parser {
class PieceTableItem {
 public:
  enum BufferType { original, add };
  BufferType bufferType;
  SizeType offset;
  SizeType length;
  [[nodiscard]] String toString(const DocPtr& doc) const;
};
class PieceTable;
class PieceTableIterator {
 public:
  PieceTableIterator(PieceTable* table, SizeType index) : m_table(table), m_index(index) {}
  PieceTableIterator& operator++() {
    m_index++;
    return *this;
  }
  bool operator!=(const PieceTableIterator& other) const { return this->m_index != other.m_index; }
  String operator*();

 private:
  SizeType m_index;
  PieceTable* m_table;
};
class EditorDocument;
class PieceTable {
  using iterator = PieceTableIterator;

 public:
  explicit PieceTable(EditorDocument& doc, qsizetype offset, qsizetype length);
  void insert(qsizetype offset, qsizetype addOffset, qsizetype addLength);
  void remove(qsizetype offset, qsizetype length);
  iterator begin();
  iterator end();

 private:
  QString itemString(qsizetype index);

 private:
  EditorDocument& m_doc;
  QList<PieceTableItem> m_items;
  friend class PieceTableIterator;
};
}  // namespace md::parser
#endif  // QTMARKDOWN_PIECETABLE_H
