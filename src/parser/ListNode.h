//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_LISTNODE_H
#define MD_PARSER_LISTNODE_H

#include "QtMarkdown_global.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT ListNode {
 public:
  virtual ~ListNode() = default;
};
class QTMARKDOWNSHARED_EXPORT ListItemNode {
 public:
  virtual ~ListItemNode() = default;
};
}  // namespace md::parser

#endif  // MD_PARSER_LISTNODE_H
