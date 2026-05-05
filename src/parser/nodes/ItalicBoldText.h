//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_ITALICBOLDTEXT_H
#define MD_PARSER_ITALICBOLDTEXT_H

#include "../Node.h"
#include "../Text.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT ItalicBoldText : public Node {
 public:
  explicit ItalicBoldText(std::unique_ptr<Text> text);
  ~ItalicBoldText();
  [[nodiscard]] Text* text() const { return m_text.get(); }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType contentLength(const IBufferProvider& doc) const override { return m_text->contentLength(doc); }
  SizeType serializedLength(const IBufferProvider& doc) const override { return 3 + m_text->serializedLength(doc) + 3; }
  bool calcMarkdownOffset(const IBufferProvider& doc, SizeType contentPos, SizeType& mdPos) const override {
    if (contentPos > m_text->contentLength(doc)) return false;
    mdPos += 3;
    return m_text->calcMarkdownOffset(doc, contentPos, mdPos);
  }

 private:
  std::unique_ptr<Text> m_text;
};
}  // namespace md::parser

#endif  // MD_PARSER_ITALICBOLDTEXT_H
