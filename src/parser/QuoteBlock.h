//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_QUOTEBLOCK_H
#define MD_PARSER_QUOTEBLOCK_H

#include "Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT QuoteBlock : public ContainerVisitable<QuoteBlock> {
 public:
  QuoteBlock() { m_type = NodeType::quote_block; }
};
}  // namespace md::parser

#endif  // MD_PARSER_QUOTEBLOCK_H
