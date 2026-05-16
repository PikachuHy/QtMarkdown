//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_HEADER_H
#define MD_PARSER_HEADER_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNPARSER_EXPORT Header : public Container {
 public:
  explicit Header(int level);
  [[nodiscard]] int level() const { return m_level; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;

 private:
  int m_level;
};
}  // namespace md::parser

#endif  // MD_PARSER_HEADER_H
