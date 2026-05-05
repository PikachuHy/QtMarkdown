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
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;

 private:
  bool m_checked;
};

class QTMARKDOWNSHARED_EXPORT CheckboxList : public ListNode {
 public:
  CheckboxList() { m_type = NodeType::checkbox; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType serializedLength(const IBufferProvider& doc) const override {
    SizeType total = 0;
    for (auto& child : m_children) {
      total += 6 + child->serializedLength(doc) + 1;  // "- [ ] " or "- [x] " + item + "\n"
    }
    return total + 1;  // trailing "\n"
  }
  bool calcMarkdownOffset(const IBufferProvider& doc, SizeType contentPos, SizeType& mdPos) const override {
    SizeType accumulatedContent = 0;
    for (auto& child : m_children) {
      SizeType childContentLen = child->contentLength(doc);
      if (contentPos <= accumulatedContent + childContentLen) {
        mdPos += 6;  // "- [ ] " or "- [x] "
        SizeType localPos = contentPos - accumulatedContent;
        return child->calcMarkdownOffset(doc, localPos, mdPos);
      }
      mdPos += 6 + child->serializedLength(doc) + 1;
      accumulatedContent += childContentLen;
    }
    return true;
  }
};
}  // namespace md::parser

#endif  // MD_PARSER_CHECKBOXLIST_H
