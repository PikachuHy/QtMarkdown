//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_ORDEREDLIST_H
#define MD_PARSER_ORDEREDLIST_H

#include "Node.h"
#include "ListNode.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT OrderedList : public ListNode, public ContainerVisitable<OrderedList> {
 public:
  OrderedList() { m_type = NodeType::ol; }
};

class QTMARKDOWNSHARED_EXPORT OrderedListItem : public ListItemNode, public ContainerVisitable<OrderedListItem> {
 public:
  OrderedListItem() { m_type = NodeType::ol_item; }
};
}  // namespace md::parser

#endif  // MD_PARSER_ORDEREDLIST_H
