//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_INLINECODE_H
#define MD_PARSER_INLINECODE_H

#include "../Node.h"
#include "../Text.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT InlineCode : public Node {
 public:
  explicit InlineCode(std::unique_ptr<Text> code);
  ~InlineCode();
  Text* code() { return m_code.get(); }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType contentLength(const IBufferProvider& doc) const override { return m_code->contentLength(doc); }
  SizeType serializedLength(const IBufferProvider& doc) const override { return 1 + m_code->serializedLength(doc) + 1; }
  bool calcMarkdownOffset(const IBufferProvider& doc, SizeType contentPos, SizeType& mdPos) const override {
    if (contentPos > m_code->contentLength(doc)) return false;
    mdPos += 1;
    return m_code->calcMarkdownOffset(doc, contentPos, mdPos);
  }

 private:
  std::unique_ptr<Text> m_code;
};
}  // namespace md::parser

#endif  // MD_PARSER_INLINECODE_H
