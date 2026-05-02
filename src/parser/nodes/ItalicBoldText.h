//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_ITALICBOLDTEXT_H
#define MD_PARSER_ITALICBOLDTEXT_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT ItalicBoldText : public Node {
 public:
  explicit ItalicBoldText(std::unique_ptr<Text> text);
  ~ItalicBoldText();
  [[nodiscard]] Text* text() const { return m_text.get(); }
  void accept(NodeVisitor* v) override { v->visit(this); }

 private:
  std::unique_ptr<Text> m_text;
};
}  // namespace md::parser

#endif  // MD_PARSER_ITALICBOLDTEXT_H
