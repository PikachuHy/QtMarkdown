//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_LINK_H
#define MD_PARSER_LINK_H

#include "Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Link : public Visitable<Link> {
 public:
  Link(std::unique_ptr<Text> content, std::unique_ptr<Text> href);
  ~Link();
  Text* content() { return m_content.get(); }
  Text* href() { return m_href.get(); }

 private:
  std::unique_ptr<Text> m_content;
  std::unique_ptr<Text> m_href;
};
}  // namespace md::parser

#endif  // MD_PARSER_LINK_H
