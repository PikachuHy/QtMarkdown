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
};

class QTMARKDOWNSHARED_EXPORT UnorderedListItem : public ListItemNode {
 public:
  UnorderedListItem() { m_type = NodeType::ul_item; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
};
}  // namespace md::parser

#endif  // MD_PARSER_UNORDEREDLIST_H
