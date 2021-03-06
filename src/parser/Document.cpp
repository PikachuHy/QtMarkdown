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
std::ostream &operator<<(std::ostream &os, const NodeType &type) {
  os << magic_enum::enum_name(type);
  return os;
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
  ASSERT(index >= 0 && index <= m_children.size());
  node->setParent(this);
  m_children.insert(m_children.begin() + index, node);
}
void Container::appendChildren(QList<Text *> &children) {
  if (children.empty()) return;
  m_children.reserve(m_children.size() + children.size());
  for (auto &node : children) {
    node->setParent(this);
    m_children.push_back(node);
  }
}
void Container::appendChildren(NodePtrList &children) {
  if (children.empty()) return;
  m_children.reserve(m_children.size() + children.size());
  for (auto &node : children) {
    node->setParent(this);
    m_children.push_back(node);
  }
}
NodePtr Container::childAt(SizeType index) const {
  ASSERT(index >= 0 && index < m_children.size());
  return m_children[index];
}
NodePtr &Container::operator[](SizeType index) {
  ASSERT(index >= 0 && index < m_children.size());
  return m_children[index];
}
const NodePtr &Container::operator[](SizeType index) const {
  ASSERT(index >= 0 && index < m_children.size());
  return m_children[index];
}
void Container::removeChildAt(SizeType index) {
  ASSERT(index >= 0 && index < m_children.size());
  m_children.erase(m_children.begin() + index);
}
SizeType Container::indexOf(NodePtr child) const {
  for (int i = 0; i < m_children.size(); ++i) {
    if (m_children[i] == child) {
      return i;
    }
  }
  DEBUG << child->type() << child;
  ASSERT(false && "no child");
}
void Container::removeChild(NodePtr node) {
  auto it = std::find(m_children.begin(), m_children.end(), node);
  if (it == m_children.end()) {
    DEBUG << node->type();
    ASSERT(false && "no child");
  }
  m_children.erase(it);
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