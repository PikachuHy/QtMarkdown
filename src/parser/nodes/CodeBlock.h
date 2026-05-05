//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_CODEBLOCK_H
#define MD_PARSER_CODEBLOCK_H

#include "../Node.h"
#include "../Text.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT CodeBlock : public Container {
 public:
  explicit CodeBlock(std::unique_ptr<Text> name);
  ~CodeBlock();
  Text* name() { return m_name.get(); }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType contentLength(const IBufferProvider& doc) const override {
    return Container::contentLength(doc);
  }
  SizeType serializedLength(const IBufferProvider& doc) const override {
    SizeType total = 3 + m_name->serializedLength(doc) + 1;  // "```" + name + "\n"
    for (auto& child : m_children) {
      total += child->serializedLength(doc) + 1;  // child + "\n"
    }
    return total + 5;  // "```\n\n"
  }
  bool calcMarkdownOffset(const IBufferProvider& doc, SizeType contentPos, SizeType& mdPos) const override {
    mdPos += 3 + m_name->serializedLength(doc) + 1;  // "```" + name + "\n"
    return Container::calcMarkdownOffset(doc, contentPos, mdPos);
  }

 private:
  std::unique_ptr<Text> m_name;
};
}  // namespace md::parser

#endif  // MD_PARSER_CODEBLOCK_H
