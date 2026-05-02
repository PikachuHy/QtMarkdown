//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_HEADER_H
#define MD_PARSER_HEADER_H

#include "Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Header : public ContainerVisitable<Header> {
 public:
  explicit Header(int level);
  [[nodiscard]] int level() const { return m_level; }

 private:
  int m_level;
};
}  // namespace md::parser

#endif  // MD_PARSER_HEADER_H
