//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_HR_H
#define MD_PARSER_HR_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Hr : public Node {
 public:
  Hr() { m_type = NodeType::hr; }
  void accept(NodeVisitor* v) override { v->visit(this); }
};
}  // namespace md::parser

#endif  // MD_PARSER_HR_H
