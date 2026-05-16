//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_TABLE_H
#define MD_PARSER_TABLE_H

#include "../Node.h"

namespace md::parser {
class QTMARKDOWNPARSER_EXPORT Table : public Node {
 public:
  Table() { m_type = NodeType::table; }
  void appendRow(const std::vector<String>& row) { m_content.push_back(row); }
  void setHeader(const std::vector<String>& header) { m_header = header; }
  std::vector<String>& header() { return m_header; }
  std::vector<std::vector<String>>& content() { return m_content; }
  void accept(NodeVisitor* v) override { v->visit(this); }
  std::unique_ptr<Node> clone() const override;

 private:
  std::vector<String> m_header;
  std::vector<std::vector<String>> m_content;
};
}  // namespace md::parser

#endif  // MD_PARSER_TABLE_H
