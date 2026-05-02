//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_TABLE_H
#define MD_PARSER_TABLE_H

#include "Node.h"

namespace md::parser {
class QTMARKDOWNSHARED_EXPORT Table : public Visitable<Table> {
 public:
  Table() { m_type = NodeType::table; }
  void appendRow(const StringList& row) { m_content.append(row); }
  void setHeader(const StringList& header) { m_header = header; }
  StringList& header() { return m_header; }
  QList<StringList>& content() { return m_content; }

 private:
  StringList m_header;
  QList<StringList> m_content;
};
}  // namespace md::parser

#endif  // MD_PARSER_TABLE_H
