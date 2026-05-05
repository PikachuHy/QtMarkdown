//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_PARAGRAPH_H
#define MD_PARSER_PARAGRAPH_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Paragraph : public Container {
 public:
  Paragraph() { m_type = NodeType::paragraph; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType serializedLength(const IBufferProvider& doc) const override {
    return Container::serializedLength(doc) + 2;  // trailing "\n\n"
  }
};
}  // namespace md::parser

#endif  // MD_PARSER_PARAGRAPH_H
