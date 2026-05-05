//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_LINK_H
#define MD_PARSER_LINK_H

#include "../Node.h"
#include "../Text.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Link : public Node {
 public:
  Link(std::unique_ptr<Text> content, std::unique_ptr<Text> href);
  ~Link();
  Text* content() { return m_content.get(); }
  Text* href() { return m_href.get(); }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType contentLength(const IBufferProvider& doc) const override { return m_content->contentLength(doc); }
  SizeType serializedLength(const IBufferProvider& doc) const override {
    return 1 + m_content->serializedLength(doc) + 2 + m_href->serializedLength(doc) + 1;  // "[" + content + "](" + href + ")"
  }
  bool calcMarkdownOffset(const IBufferProvider& doc, SizeType contentPos, SizeType& mdPos) const override {
    mdPos += 1;  // "["
    return m_content->calcMarkdownOffset(doc, contentPos, mdPos);
  }

 private:
  std::unique_ptr<Text> m_content;
  std::unique_ptr<Text> m_href;
};
}  // namespace md::parser

#endif  // MD_PARSER_LINK_H
