//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_CODEBLOCK_H
#define MD_PARSER_CODEBLOCK_H

#include "Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT CodeBlock : public ContainerVisitable<CodeBlock> {
 public:
  explicit CodeBlock(std::unique_ptr<Text> name);
  ~CodeBlock();
  Text* name() { return m_name.get(); }

 private:
  std::unique_ptr<Text> m_name;
};
}  // namespace md::parser

#endif  // MD_PARSER_CODEBLOCK_H
