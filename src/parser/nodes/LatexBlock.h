//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_LATEXBLOCK_H
#define MD_PARSER_LATEXBLOCK_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT LatexBlock : public ContainerVisitable<LatexBlock> {
 public:
  LatexBlock() { m_type = NodeType::latex_block; }
  [[nodiscard]] String toString(DocPtr const& doc) const;
};
}  // namespace md::parser

#endif  // MD_PARSER_LATEXBLOCK_H
