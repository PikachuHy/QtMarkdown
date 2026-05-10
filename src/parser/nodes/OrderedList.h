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
};

class QTMARKDOWNSHARED_EXPORT OrderedListItem : public ListItemNode {
 public:
  OrderedListItem() { m_type = NodeType::ol_item; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
};
}  // namespace md::parser

#endif  // MD_PARSER_ORDEREDLIST_H
