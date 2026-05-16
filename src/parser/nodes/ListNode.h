//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_LISTNODE_H
#define MD_PARSER_LISTNODE_H

#include "../Node.h"

namespace md::parser {

class QTMARKDOWNPARSER_EXPORT ListNode : public Container {
 public:
  ListNode() = default;
  ~ListNode() override = default;
  std::unique_ptr<Node> clone() const override = 0;
};

class QTMARKDOWNPARSER_EXPORT ListItemNode : public Container {
 public:
  ListItemNode() = default;
  ~ListItemNode() override = default;
  std::unique_ptr<Node> clone() const override = 0;
};

}  // namespace md::parser
#endif  // MD_PARSER_LISTNODE_H
