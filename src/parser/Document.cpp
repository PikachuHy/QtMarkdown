//
// Created by pikachu on 2021/1/31.
//

#include "Document.h"

#include <utility>

#include "Parser.h"
#include "Text.h"
#include "debug.h"
#include "magic_enum.hpp"
namespace md::parser {
QDebug operator<<(QDebug debug, const NodeType &type) {
  QDebugStateSaver saver(debug);
  auto str = std::string(magic_enum::enum_name(type));
  debug.nospace() << QString::fromStdString(str);
  return debug;
}
Header::Header(int level) : m_level(level) { m_type = NodeType::header; }

Document::Document(const String &str) : m_originalBuffer(str), m_root(Parser::parse(str)) {}

String Document::toHtml() {
#if 0
  auto visitor = new SimpleHtmlVisitor(m_originalBuffer);
  for (auto it : m_root->children()) {
    it->accept(visitor);
  }
  auto ret = visitor->html();
  delete visitor;
  return ret;
#endif
  return {};
}

void Document::accept(VisitorNode *visitor) {
  for (auto it : m_root->children()) {
    it->accept(visitor);
  }
}
void Container::setChild(SizeType index, NodePtr node) {
  ASSERT(index >= 0 && index < m_children.size());
  node->setParent(this);
  m_children[index] = node;
}
void Container::insertChild(SizeType index, NodePtr node) {
  ASSERT(index >= 0 && index < m_children.size());
  node->setParent(this);
  m_children.insert(index, node);
}
void Container::appendChildren(QList<Text *> &children) {
  if (children.empty()) return;
  m_children.reserve(m_children.size() + children.size());
  for (auto &node : children) {
    node->setParent(this);
    m_children.append(node);
  }
}
void Container::appendChildren(NodePtrList &children) {
  if (children.empty()) return;
  m_children.reserve(m_children.size() + children.size());
  for (auto &node : children) {
    node->setParent(this);
    m_children.append(node);
  }
}
ItalicText::ItalicText(Text *text) : m_text(text) {
  m_type = NodeType::italic;
  m_text->setParent(this);
}
BoldText::BoldText(Text *text) : m_text(text) {
  m_type = NodeType::bold;
  m_text->setParent(this);
}
ItalicBoldText::ItalicBoldText(Text *text) : m_text(text) {
  m_type = NodeType::italic_bold;
  m_text->setParent(this);
}
StrickoutText::StrickoutText(Text *text) : m_text(text) {
  m_type = NodeType::strickout;
  m_text->setParent(this);
}
Link::Link(Text *content, Text *href) : m_content(content), m_href(href) {
  m_type = NodeType::link;
  m_content->setParent(this);
  m_href->setParent(this);
}
}  // namespace md::parser