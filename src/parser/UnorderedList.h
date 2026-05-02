//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_UNORDEREDLIST_H
#define MD_PARSER_UNORDEREDLIST_H

#include "Node.h"
#include "ListNode.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT UnorderedList : public ListNode, public ContainerVisitable<UnorderedList> {
 public:
  UnorderedList() { m_type = NodeType::ul; }
};

class QTMARKDOWNSHARED_EXPORT UnorderedListItem : public ListItemNode, public ContainerVisitable<UnorderedListItem> {
 public:
  UnorderedListItem() { m_type = NodeType::ul_item; }
};
}  // namespace md::parser

#endif  // MD_PARSER_UNORDEREDLIST_H
