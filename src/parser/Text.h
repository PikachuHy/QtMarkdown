//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_TEXT_H
#define QTMARKDOWN_TEXT_H
#include "Document.h"
#include "PieceTable.h"
namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Text : public Visitable<Text> {
 public:
  Text(SizeType offset, SizeType length) {
    m_type = NodeType::text;
    m_items.emplace_back(PieceTableItem{PieceTableItem::original, offset, length});
  }
  [[nodiscard]] String toString(const DocPtr& doc) const;
  void insert(SizeType totalOffset, PieceTableItem item);
  void remove(SizeType totalOffset, SizeType length);
  std::pair<Text*, Text*> split(SizeType totalOffset);
  auto begin() { return m_items.begin(); }
  auto end() { return m_items.end(); }

 private:
  Text() {}
  [[nodiscard]] std::pair<SizeType, SizeType> findItem(SizeType totalOffset) const;

 private:
  QList<PieceTableItem> m_items;
};
class QTMARKDOWNSHARED_EXPORT LatexBlock : public ContainerVisitable<LatexBlock> {
 public:
  explicit LatexBlock() { m_type = NodeType::latex_block; }
  [[nodiscard]] String toString(const DocPtr& doc) const;
};
}  // namespace md::parser
#endif  // QTMARKDOWN_TEXT_H
