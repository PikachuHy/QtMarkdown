//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_QUOTEBLOCK_H
#define MD_PARSER_QUOTEBLOCK_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT QuoteBlock : public Container {
 public:
  QuoteBlock() { m_type = NodeType::quote_block; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType serializedLength(const IBufferProvider& doc) const override {
    SizeType total = 2;  // "> "
    bool first = true;
    for (auto& child : m_children) {
      if (!first) total += child->serializedLength(doc) + 1;  // "\n" + child
      else { total += child->serializedLength(doc); first = false; }
    }
    return total + 1;  // trailing "\n"
  }
  bool calcMarkdownOffset(const IBufferProvider& doc, SizeType contentPos, SizeType& mdPos) const override {
    bool first = true;
    SizeType accumulatedContent = 0;
    for (auto& child : m_children) {
      SizeType childContentLen = child->contentLength(doc);
      if (contentPos <= accumulatedContent + childContentLen) {
        if (first) mdPos += 2;  // "> "
        SizeType localContentPos = contentPos - accumulatedContent;
        return child->calcMarkdownOffset(doc, localContentPos, mdPos);
      }
      if (first) { mdPos += 2 + child->serializedLength(doc); first = false; }
      else { mdPos += child->serializedLength(doc) + 1; }
      accumulatedContent += childContentLen;
    }
    return true;
  }
};
}  // namespace md::parser

#endif  // MD_PARSER_QUOTEBLOCK_H
