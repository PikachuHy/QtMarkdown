//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_LF_H
#define MD_PARSER_LF_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Lf : public Node {
 public:
  Lf() { m_type = NodeType::lf; }
  void accept(NodeVisitor* v) override { v->visit(this); }
};
}  // namespace md::parser

#endif  // MD_PARSER_LF_H
