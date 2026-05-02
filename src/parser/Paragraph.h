//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_PARAGRAPH_H
#define MD_PARSER_PARAGRAPH_H

#include "Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Paragraph : public ContainerVisitable<Paragraph> {
 public:
  Paragraph() { m_type = NodeType::paragraph; }
};
}  // namespace md::parser

#endif  // MD_PARSER_PARAGRAPH_H
