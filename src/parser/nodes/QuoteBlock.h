//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_QUOTEBLOCK_H
#define MD_PARSER_QUOTEBLOCK_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNPARSER_EXPORT QuoteBlock : public Container {
 public:
  QuoteBlock() { m_type = NodeType::quote_block; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
};
}  // namespace md::parser

#endif  // MD_PARSER_QUOTEBLOCK_H
