//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_ORDEREDLIST_H
#define MD_PARSER_ORDEREDLIST_H

#include "../Node.h"
#include "ListNode.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT OrderedList : public ListNode {
 public:
  OrderedList() { m_type = NodeType::ol; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType serializedLength(const IBufferProvider& doc) const override {
    SizeType total = 0;
    for (SizeType i = 0; i < m_children.size(); ++i) {
      SizeType prefixLen = std::to_string(i + 1).size() + 2;  // "N. "
      total += prefixLen + m_children[i]->serializedLength(doc) + 1;
    }
    return total + 1;  // trailing "\n"
  }
  bool calcMarkdownOffset(const IBufferProvider& doc, SizeType contentPos, SizeType& mdPos) const override {
    SizeType accumulatedContent = 0;
    for (SizeType i = 0; i < m_children.size(); ++i) {
      auto& child = m_children[i];
      SizeType childContentLen = child->contentLength(doc);
      if (contentPos <= accumulatedContent + childContentLen) {
        mdPos += std::to_string(i + 1).size() + 2;  // "N. "
        SizeType localPos = contentPos - accumulatedContent;
        return child->calcMarkdownOffset(doc, localPos, mdPos);
      }
      mdPos += std::to_string(i + 1).size() + 2 + child->serializedLength(doc) + 1;
      accumulatedContent += childContentLen;
    }
    return true;
  }
};

class QTMARKDOWNSHARED_EXPORT OrderedListItem : public ListItemNode {
 public:
  OrderedListItem() { m_type = NodeType::ol_item; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
};
}  // namespace md::parser

#endif  // MD_PARSER_ORDEREDLIST_H
