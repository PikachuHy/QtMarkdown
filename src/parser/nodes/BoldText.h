//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_BOLDTEXT_H
#define MD_PARSER_BOLDTEXT_H

#include "../Node.h"
#include "../Text.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT BoldText : public Node {
 public:
  explicit BoldText(std::unique_ptr<Text> text);
  ~BoldText();
  [[nodiscard]] Text* text() const { return m_text.get(); }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType contentLength(const IBufferProvider& doc) const override { return m_text->contentLength(doc); }

 private:
  std::unique_ptr<Text> m_text;
};
}  // namespace md::parser

#endif  // MD_PARSER_BOLDTEXT_H
