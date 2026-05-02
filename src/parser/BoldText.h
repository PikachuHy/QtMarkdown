//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_BOLDTEXT_H
#define MD_PARSER_BOLDTEXT_H

#include "Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT BoldText : public Visitable<BoldText> {
 public:
  explicit BoldText(std::unique_ptr<Text> text);
  ~BoldText();
  [[nodiscard]] Text* text() const { return m_text.get(); }

 private:
  std::unique_ptr<Text> m_text;
};
}  // namespace md::parser

#endif  // MD_PARSER_BOLDTEXT_H
