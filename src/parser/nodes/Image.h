//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_IMAGE_H
#define MD_PARSER_IMAGE_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Image : public Node {
 public:
  Image(std::unique_ptr<Text> alt, std::unique_ptr<Text> src);
  ~Image();
  Text* alt() { return m_alt.get(); }
  Text* src() { return m_src.get(); }
  void accept(NodeVisitor* v) override { v->visit(this); }

 private:
  std::unique_ptr<Text> m_alt;
  std::unique_ptr<Text> m_src;
};
}  // namespace md::parser

#endif  // MD_PARSER_IMAGE_H
