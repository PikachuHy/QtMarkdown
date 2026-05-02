//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_INLINECODE_H
#define MD_PARSER_INLINECODE_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT InlineCode : public Visitable<InlineCode> {
 public:
  explicit InlineCode(std::unique_ptr<Text> code);
  ~InlineCode();
  Text* code() { return m_code.get(); }

 private:
  std::unique_ptr<Text> m_code;
};
}  // namespace md::parser

#endif  // MD_PARSER_INLINECODE_H
