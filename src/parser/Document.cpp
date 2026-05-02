//
// Created by pikachu on 2021/1/31.
//

#include "Document.h"

#include <utility>
#include <algorithm>

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
  for (auto& it : m_root->children()) {
    it->accept(visitor);
  }
  auto ret = visitor->html();
  delete visitor;
  return ret;
#endif
  return {};
}

void Document::accept(NodeVisitor *visitor) {
  for (auto& it : m_root->children()) {
    it->accept(visitor);
  }
}
void Container::setChild(SizeType index, std::unique_ptr<Node> node) {
  ASSERT(index >= 0 && index < m_children.size());
  node->setParent(this);
  m_children[index] = std::move(node);
}
void Container::insertChild(SizeType index, std::unique_ptr<Node> node) {
  ASSERT(index >= 0 && index <= m_children.size());
  node->setParent(this);
  m_children.insert(m_children.begin() + index, std::move(node));
}
void Container::appendChildren(std::vector<std::unique_ptr<Text>> &&children) {
  if (children.empty()) return;
  m_children.reserve(m_children.size() + children.size());
  for (auto &node : children) {
    node->setParent(this);
    m_children.push_back(std::move(node));
  }
}
void Container::appendChildren(NodePtrList &&children) {
  if (children.empty()) return;
  m_children.reserve(m_children.size() + children.size());
  for (auto &node : children) {
    node->setParent(this);
    m_children.push_back(std::move(node));
  }
}
Node* Container::childAt(SizeType index) const {
  ASSERT(index >= 0 && index < m_children.size());
  return m_children[index].get();
}
std::unique_ptr<Node>& Container::operator[](SizeType index) {
  ASSERT(index >= 0 && index < m_children.size());
  return m_children[index];
}
const std::unique_ptr<Node>& Container::operator[](SizeType index) const {
  ASSERT(index >= 0 && index < m_children.size());
  return m_children[index];
}
void Container::removeChildAt(SizeType index) {
  ASSERT(index >= 0 && index < m_children.size());
  m_children.erase(m_children.begin() + index);
}
SizeType Container::indexOf(Node* child) const {
  for (int i = 0; i < m_children.size(); ++i) {
    if (m_children[i].get() == child) {
      return i;
    }
  }
  DEBUG << child->type() << child;
  ASSERT(false && "no child");
}
void Container::removeChild(Node* node) {
  auto it = std::find_if(m_children.begin(), m_children.end(),
      [node](const auto& ptr) { return ptr.get() == node; });
  if (it == m_children.end()) {
    DEBUG << node->type();
    ASSERT(false && "no child");
  }
  m_children.erase(it);
}
ItalicText::ItalicText(std::unique_ptr<Text> text) : m_text(std::move(text)) {
  m_type = NodeType::italic;
  m_text->setParent(this);
}
ItalicText::~ItalicText() = default;
BoldText::BoldText(std::unique_ptr<Text> text) : m_text(std::move(text)) {
  m_type = NodeType::bold;
  m_text->setParent(this);
}
BoldText::~BoldText() = default;
ItalicBoldText::ItalicBoldText(std::unique_ptr<Text> text) : m_text(std::move(text)) {
  m_type = NodeType::italic_bold;
  m_text->setParent(this);
}
ItalicBoldText::~ItalicBoldText() = default;
StrickoutText::StrickoutText(std::unique_ptr<Text> text) : m_text(std::move(text)) {
  m_type = NodeType::strickout;
  m_text->setParent(this);
}
StrickoutText::~StrickoutText() = default;
Image::Image(std::unique_ptr<Text> alt, std::unique_ptr<Text> src) : m_alt(std::move(alt)), m_src(std::move(src)) {
  m_type = NodeType::image;
  m_alt->setParent(this);
  m_src->setParent(this);
}
Image::~Image() = default;
Link::Link(std::unique_ptr<Text> content, std::unique_ptr<Text> href) : m_content(std::move(content)), m_href(std::move(href)) {
  m_type = NodeType::link;
  m_content->setParent(this);
  m_href->setParent(this);
}
Link::~Link() = default;
CodeBlock::CodeBlock(std::unique_ptr<Text> name) : m_name(std::move(name)) {
  m_type = NodeType::code_block;
  m_name->setParent(this);
}
CodeBlock::~CodeBlock() = default;
InlineCode::InlineCode(std::unique_ptr<Text> code) : m_code(std::move(code)) {
  m_type = NodeType::inline_code;
  m_code->setParent(this);
}
InlineCode::~InlineCode() = default;
InlineLatex::InlineLatex(std::unique_ptr<Text> code) : m_code(std::move(code)) {
  m_type = NodeType::inline_latex;
  m_code->setParent(this);
}
InlineLatex::~InlineLatex() = default;
}  // namespace md::parser
