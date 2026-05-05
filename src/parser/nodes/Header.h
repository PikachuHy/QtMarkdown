//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_HEADER_H
#define MD_PARSER_HEADER_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Header : public Container {
 public:
  explicit Header(int level);
  [[nodiscard]] int level() const { return m_level; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;
  SizeType serializedLength(const IBufferProvider& doc) const override {
    return m_level + 1 + Container::serializedLength(doc) + 2;
  }
  bool calcMarkdownOffset(const IBufferProvider& doc, SizeType contentPos, SizeType& mdPos) const override {
    mdPos += m_level + 1;  // "### "
    return Container::calcMarkdownOffset(doc, contentPos, mdPos);
  }

 private:
  int m_level;
};
}  // namespace md::parser

#endif  // MD_PARSER_HEADER_H
