//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_CHECKBOXLIST_H
#define MD_PARSER_CHECKBOXLIST_H

#include "../Node.h"
#include "ListNode.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT CheckboxItem : public ListItemNode {
 public:
  CheckboxItem() : m_checked(false) { m_type = NodeType::checkbox_item; }
  [[nodiscard]] bool isChecked() const { return m_checked; }
  void setChecked(bool flag) { m_checked = flag; }
  void accept(VisitorNode* v) override {
    if (auto p = dynamic_cast<Visitor<CheckboxItem>*>(v); p) {
      p->visit(this);
    }
  }

 private:
  bool m_checked;
};

class QTMARKDOWNSHARED_EXPORT CheckboxList : public ListNode {
 public:
  CheckboxList() { m_type = NodeType::checkbox; }
  void accept(VisitorNode* v) override {
    if (auto p = dynamic_cast<Visitor<CheckboxList>*>(v); p) {
      p->visit(this);
    }
  }
};
}  // namespace md::parser

#endif  // MD_PARSER_CHECKBOXLIST_H
