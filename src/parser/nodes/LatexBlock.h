//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_LATEXBLOCK_H
#define MD_PARSER_LATEXBLOCK_H

#include "../IBufferProvider.h"
#include "../Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT LatexBlock : public Container {
 public:
  LatexBlock() { m_type = NodeType::latex_block; }
  [[nodiscard]] String toString(const IBufferProvider& doc) const;
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType serializedLength(const IBufferProvider& doc) const override {
    return 4 + Container::serializedLength(doc) + 5;  // "\n$$\n" + children + "\n$$\n\n"
  }
  bool calcMarkdownOffset(const IBufferProvider& doc, SizeType contentPos, SizeType& mdPos) const override {
    mdPos += 4;  // "\n$$\n"
    return Container::calcMarkdownOffset(doc, contentPos, mdPos);
  }
};
}  // namespace md::parser

#endif  // MD_PARSER_LATEXBLOCK_H
