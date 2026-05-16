//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_CODEBLOCK_H
#define MD_PARSER_CODEBLOCK_H

#include "../Node.h"
#include "../Text.h"

namespace md::parser {
class QTMARKDOWNPARSER_EXPORT CodeBlock : public Container {
 public:
  explicit CodeBlock(std::unique_ptr<Text> name);
  ~CodeBlock();
  Text* name() { return m_name.get(); }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType contentLength(const IBufferProvider& doc) const override {
    return Container::contentLength(doc);
  }

 private:
  std::unique_ptr<Text> m_name;
};
}  // namespace md::parser

#endif  // MD_PARSER_CODEBLOCK_H
