//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_STRICKOUTTEXT_H
#define MD_PARSER_STRICKOUTTEXT_H

#include "Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT StrickoutText : public Visitable<StrickoutText> {
 public:
  explicit StrickoutText(std::unique_ptr<Text> text);
  ~StrickoutText();
  [[nodiscard]] Text* text() const { return m_text.get(); }

 private:
  std::unique_ptr<Text> m_text;
};
}  // namespace md::parser

#endif  // MD_PARSER_STRICKOUTTEXT_H
