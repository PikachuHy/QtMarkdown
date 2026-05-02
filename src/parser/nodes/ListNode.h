//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_LISTNODE_H
#define MD_PARSER_LISTNODE_H

#include "../Node.h"

namespace md::parser {

class QTMARKDOWNSHARED_EXPORT ListNode : public Container {
 public:
  ListNode() = default;
  ~ListNode() override = default;
  void accept(VisitorNode* v) override {
    if (auto p = dynamic_cast<Visitor<ListNode>*>(v); p) {
      p->visit(this);
    }
  }
};

class QTMARKDOWNSHARED_EXPORT ListItemNode : public Container {
 public:
  ListItemNode() = default;
  ~ListItemNode() override = default;
  void accept(VisitorNode* v) override {
    if (auto p = dynamic_cast<Visitor<ListItemNode>*>(v); p) {
      p->visit(this);
    }
  }
};

}  // namespace md::parser
#endif  // MD_PARSER_LISTNODE_H
