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
  void accept(VisitorNode* v) override {
    if (auto p = dynamic_cast<Visitor<OrderedList>*>(v); p) {
      p->visit(this);
    }
  }
};

class QTMARKDOWNSHARED_EXPORT OrderedListItem : public ListItemNode {
 public:
  OrderedListItem() { m_type = NodeType::ol_item; }
  void accept(VisitorNode* v) override {
    if (auto p = dynamic_cast<Visitor<OrderedListItem>*>(v); p) {
      p->visit(this);
    }
  }
};
}  // namespace md::parser

#endif  // MD_PARSER_ORDEREDLIST_H
