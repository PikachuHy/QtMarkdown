//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_INLINELATEX_H
#define MD_PARSER_INLINELATEX_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT InlineLatex : public Node {
 public:
  explicit InlineLatex(std::unique_ptr<Text> code);
  ~InlineLatex();
  Text* code() { return m_code.get(); }
  void accept(NodeVisitor* v) override { v->visit(this); }

 private:
  std::unique_ptr<Text> m_code;
};
}  // namespace md::parser

#endif  // MD_PARSER_INLINELATEX_H
