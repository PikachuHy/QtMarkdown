//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_LISTNODE_H
#define MD_PARSER_LISTNODE_H

#include "../Node.h"

namespace md::parser {

class QTMARKDOWNSHARED_EXPORT ListNode : public Container {
 public:
  ListNode() = default;
  ~ListNode() override = default;
};

class QTMARKDOWNSHARED_EXPORT ListItemNode : public Container {
 public:
  ListItemNode() = default;
  ~ListItemNode() override = default;
};

}  // namespace md::parser
#endif  // MD_PARSER_LISTNODE_H
