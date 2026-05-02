//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_HR_H
#define MD_PARSER_HR_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Hr : public Visitable<Hr> {
 public:
  Hr() { m_type = NodeType::hr; }
};
}  // namespace md::parser

#endif  // MD_PARSER_HR_H
