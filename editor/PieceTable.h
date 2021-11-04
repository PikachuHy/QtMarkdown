//
// Created by PikachuHy on 2021/11/2.
//

#ifndef QTMARKDOWN_PIECETABLE_H
#define QTMARKDOWN_PIECETABLE_H

#include <QList>
#include <QString>
struct PieceTableItem {
  enum BufferType { original, add };
  BufferType bufferType;
  qsizetype offset;
  qsizetype length;
};
class PieceTable;
class PieceTableIterator {
 public:
  PieceTableIterator(PieceTable* table, qsizetype index)
      : m_table(table), m_index(index) {}
  PieceTableIterator& operator++() {
    m_index++;
    return *this;
  }
  bool operator!=(const PieceTableIterator& other) const {
    return this->m_index != other.m_index;
  }
  QString operator*();

 private:
  qsizetype m_index;
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

#endif  // QTMARKDOWN_PIECETABLE_H
