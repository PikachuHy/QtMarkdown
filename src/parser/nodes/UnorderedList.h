//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_UNORDEREDLIST_H
#define MD_PARSER_UNORDEREDLIST_H

#include "../Node.h"
#include "ListNode.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT UnorderedList : public ListNode {
 public:
  UnorderedList() { m_type = NodeType::ul; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType serializedLength(const IBufferProvider& doc) const override {
    SizeType total = 0;
    for (auto& child : m_children) {
      total += 2 + child->serializedLength(doc) + 1;  // "- " + item + "\n"
    }
    return total + 1;  // trailing "\n"
  }
  bool calcMarkdownOffset(const IBufferProvider& doc, SizeType contentPos, SizeType& mdPos) const override {
    SizeType accumulatedContent = 0;
    for (auto& child : m_children) {
      SizeType childContentLen = child->contentLength(doc);
      if (contentPos <= accumulatedContent + childContentLen) {
        mdPos += 2;  // "- "
        SizeType localPos = contentPos - accumulatedContent;
        return child->calcMarkdownOffset(doc, localPos, mdPos);
      }
      mdPos += 2 + child->serializedLength(doc) + 1;
      accumulatedContent += childContentLen;
    }
    return true;
  }
};

class QTMARKDOWNSHARED_EXPORT UnorderedListItem : public ListItemNode {
 public:
  UnorderedListItem() { m_type = NodeType::ul_item; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
};
}  // namespace md::parser

#endif  // MD_PARSER_UNORDEREDLIST_H
