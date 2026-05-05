//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_TEXT_H
#define QTMARKDOWN_TEXT_H
#include "Node.h"
#include "ParseContext.h"
#include "PieceTable.h"
namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Text : public Node {
  using PieceTableItemList = std::vector<PieceTableItem>;
 public:
  Text(SizeType offset, SizeType length) {
    m_type = NodeType::text;
    m_items.emplace_back(PieceTableItem{g_parseContext.bufferType,
                                        g_parseContext.baseOffset + offset,
                                        length});
  }
  Text(PieceTableItem::BufferType type, SizeType offset, SizeType length) {
    m_type = NodeType::text;
    m_items.emplace_back(PieceTableItem{type, offset, length});
  }
  bool empty() const;
  [[nodiscard]] String toString(const IBufferProvider& doc) const;
  void insert(SizeType totalOffset, PieceTableItem item);
  void remove(SizeType totalOffset, SizeType length);
  std::pair<std::unique_ptr<Text>, std::unique_ptr<Text>> split(SizeType totalOffset);
  auto begin() { return m_items.begin(); }
  auto end() { return m_items.end(); }
  void merge(Text& text);
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType contentLength(const IBufferProvider& doc) const override;
  bool calcMarkdownOffset(const IBufferProvider& doc, SizeType contentPos, SizeType& mdPos) const override;

 private:
  Text() { m_type = NodeType::text; }
  [[nodiscard]] std::pair<SizeType, SizeType> findItem(SizeType totalOffset, bool includeRight = true) const;
  void insertItem(SizeType index, PieceTableItem item);
  void removeItemAt(SizeType index);
 private:
  PieceTableItemList m_items;
};
}  // namespace md::parser
#endif  // QTMARKDOWN_TEXT_H
