//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_LF_H
#define MD_PARSER_LF_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNPARSER_EXPORT Lf : public Node {
 public:
  Lf() { m_type = NodeType::lf; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override { return std::make_unique<Lf>(); }
};
}  // namespace md::parser

#endif  // MD_PARSER_LF_H
