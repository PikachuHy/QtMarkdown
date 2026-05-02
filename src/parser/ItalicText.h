//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_ITALICTEXT_H
#define MD_PARSER_ITALICTEXT_H

#include "Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT ItalicText : public Visitable<ItalicText> {
 public:
  explicit ItalicText(std::unique_ptr<Text> text);
  ~ItalicText();
  [[nodiscard]] Text* text() const { return m_text.get(); }

 private:
  std::unique_ptr<Text> m_text;
};
}  // namespace md::parser

#endif  // MD_PARSER_ITALICTEXT_H
