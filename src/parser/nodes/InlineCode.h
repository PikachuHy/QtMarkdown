//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_INLINECODE_H
#define MD_PARSER_INLINECODE_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT InlineCode : public Node {
 public:
  explicit InlineCode(std::unique_ptr<Text> code);
  ~InlineCode();
  Text* code() { return m_code.get(); }
  void accept(NodeVisitor* v) override { v->visit(this); }

 private:
  std::unique_ptr<Text> m_code;
};
}  // namespace md::parser

#endif  // MD_PARSER_INLINECODE_H
