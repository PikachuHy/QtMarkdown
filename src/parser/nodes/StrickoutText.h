//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_STRICKOUTTEXT_H
#define MD_PARSER_STRICKOUTTEXT_H

#include "../Node.h"
#include "../Text.h"

namespace md::parser {
class QTMARKDOWNPARSER_EXPORT StrickoutText : public Node {
 public:
  explicit StrickoutText(std::unique_ptr<Text> text);
  ~StrickoutText();
  [[nodiscard]] Text* text() const { return m_text.get(); }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType contentLength(const IBufferProvider& doc) const override { return m_text->contentLength(doc); }

 private:
  std::unique_ptr<Text> m_text;
};
}  // namespace md::parser

#endif  // MD_PARSER_STRICKOUTTEXT_H
