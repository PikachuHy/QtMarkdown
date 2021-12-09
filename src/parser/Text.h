//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_TEXT_H
#define QTMARKDOWN_TEXT_H
#include "Document.h"
#include "PieceTable.h"
namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Text : public Visitable<Text> {
  using PieceTableItemList = std::vector<PieceTableItem>;
 public:
  Text(SizeType offset, SizeType length) {
    m_type = NodeType::text;
    m_items.emplace_back(PieceTableItem{PieceTableItem::original, offset, length});
  }
  Text(PieceTableItem::BufferType type, SizeType offset, SizeType length) {
    m_type = NodeType::text;
    m_items.emplace_back(PieceTableItem{type, offset, length});
  }
  bool empty() const;
  [[nodiscard]] String toString(const DocPtr& doc) const;
  void insert(SizeType totalOffset, PieceTableItem item);
  void remove(SizeType totalOffset, SizeType length);
  std::pair<Text*, Text*> split(SizeType totalOffset);
  auto begin() { return m_items.begin(); }
  auto end() { return m_items.end(); }
  void merge(Text& text);

 private:
  Text() { m_type = NodeType::text; }
  [[nodiscard]] std::pair<SizeType, SizeType> findItem(SizeType totalOffset, bool includeRight = true) const;
  void insertItem(SizeType index, PieceTableItem item);
  void removeItemAt(SizeType index);
 private:
  PieceTableItemList m_items;
};
class QTMARKDOWNSHARED_EXPORT LatexBlock : public ContainerVisitable<LatexBlock> {
 public:
  explicit LatexBlock() { m_type = NodeType::latex_block; }
  [[nodiscard]] String toString(const DocPtr& doc) const;
};
}  // namespace md::parser
#endif  // QTMARKDOWN_TEXT_H
