//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_LF_H
#define MD_PARSER_LF_H

#include "Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Lf : public Visitable<Lf> {
 public:
  Lf() { m_type = NodeType::lf; }
};
}  // namespace md::parser

#endif  // MD_PARSER_LF_H
