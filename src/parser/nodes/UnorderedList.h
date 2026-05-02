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
  void accept(VisitorNode* v) override {
    if (auto p = dynamic_cast<Visitor<UnorderedList>*>(v); p) {
      p->visit(this);
    } else {
      ListNode::accept(v);
    }
  }
};

class QTMARKDOWNSHARED_EXPORT UnorderedListItem : public ListItemNode {
 public:
  UnorderedListItem() { m_type = NodeType::ul_item; }
  void accept(VisitorNode* v) override {
    if (auto p = dynamic_cast<Visitor<UnorderedListItem>*>(v); p) {
      p->visit(this);
    } else {
      ListItemNode::accept(v);
    }
  }
};
}  // namespace md::parser

#endif  // MD_PARSER_UNORDEREDLIST_H
