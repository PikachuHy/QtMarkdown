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
std::ostream& operator<<(std::ostream& os, const NodeType& type) {
  os << magic_enum::enum_name(type);
  return os;
}
Header::Header(int level) : m_level(level) { m_type = NodeType::header; }

Document::Document(const String &str) : m_originalBuffer(str), m_root(Parser::parse(str)) {}

String Document::toHtml() {
  // HTML export not yet implemented
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
  if (auto& child = m_children[index]) {
    child->setParent(nullptr);
  }
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
  return -1;
}
void Container::removeChild(Node* node) {
  auto it = std::find_if(m_children.begin(), m_children.end(),
      [node](const auto& ptr) { return ptr.get() == node; });
  if (it == m_children.end()) {
    DEBUG << node->type();
    ASSERT(false && "no child");
    return;
  }
  node->setParent(nullptr);
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

// clone() implementations

std::unique_ptr<Node> ItalicText::clone() const {
  auto text = std::unique_ptr<Text>(static_cast<Text*>(m_text->clone().release()));
  return std::make_unique<ItalicText>(std::move(text));
}
std::unique_ptr<Node> BoldText::clone() const {
  auto text = std::unique_ptr<Text>(static_cast<Text*>(m_text->clone().release()));
  return std::make_unique<BoldText>(std::move(text));
}
std::unique_ptr<Node> ItalicBoldText::clone() const {
  auto text = std::unique_ptr<Text>(static_cast<Text*>(m_text->clone().release()));
  return std::make_unique<ItalicBoldText>(std::move(text));
}
std::unique_ptr<Node> StrickoutText::clone() const {
  auto text = std::unique_ptr<Text>(static_cast<Text*>(m_text->clone().release()));
  return std::make_unique<StrickoutText>(std::move(text));
}
std::unique_ptr<Node> Image::clone() const {
  auto alt = std::unique_ptr<Text>(static_cast<Text*>(m_alt->clone().release()));
  auto src = std::unique_ptr<Text>(static_cast<Text*>(m_src->clone().release()));
  return std::make_unique<Image>(std::move(alt), std::move(src));
}
std::unique_ptr<Node> Link::clone() const {
  auto content = std::unique_ptr<Text>(static_cast<Text*>(m_content->clone().release()));
  auto href = std::unique_ptr<Text>(static_cast<Text*>(m_href->clone().release()));
  return std::make_unique<Link>(std::move(content), std::move(href));
}
std::unique_ptr<Node> CodeBlock::clone() const {
  auto name = std::unique_ptr<Text>(static_cast<Text*>(m_name->clone().release()));
  auto cb = std::make_unique<CodeBlock>(std::move(name));
  for (auto& child : m_children) {
    cb->appendChild(child->clone());
  }
  return cb;
}
std::unique_ptr<Node> InlineCode::clone() const {
  auto code = std::unique_ptr<Text>(static_cast<Text*>(m_code->clone().release()));
  return std::make_unique<InlineCode>(std::move(code));
}
std::unique_ptr<Node> InlineLatex::clone() const {
  auto code = std::unique_ptr<Text>(static_cast<Text*>(m_code->clone().release()));
  return std::make_unique<InlineLatex>(std::move(code));
}
std::unique_ptr<Node> Header::clone() const {
  auto h = std::make_unique<Header>(m_level);
  for (auto& child : m_children) {
    h->appendChild(child->clone());
  }
  return h;
}
std::unique_ptr<Node> Paragraph::clone() const {
  auto p = std::make_unique<Paragraph>();
  for (auto& child : m_children) {
    p->appendChild(child->clone());
  }
  return p;
}
std::unique_ptr<Node> QuoteBlock::clone() const {
  auto q = std::make_unique<QuoteBlock>();
  for (auto& child : m_children) {
    q->appendChild(child->clone());
  }
  return q;
}
std::unique_ptr<Node> UnorderedList::clone() const {
  auto ul = std::make_unique<UnorderedList>();
  for (auto& child : m_children) {
    ul->appendChild(child->clone());
  }
  return ul;
}
std::unique_ptr<Node> UnorderedListItem::clone() const {
  auto item = std::make_unique<UnorderedListItem>();
  for (auto& child : m_children) {
    item->appendChild(child->clone());
  }
  return item;
}
std::unique_ptr<Node> OrderedList::clone() const {
  auto ol = std::make_unique<OrderedList>();
  for (auto& child : m_children) {
    ol->appendChild(child->clone());
  }
  return ol;
}
std::unique_ptr<Node> OrderedListItem::clone() const {
  auto item = std::make_unique<OrderedListItem>();
  for (auto& child : m_children) {
    item->appendChild(child->clone());
  }
  return item;
}
std::unique_ptr<Node> CheckboxList::clone() const {
  auto cl = std::make_unique<CheckboxList>();
  for (auto& child : m_children) {
    cl->appendChild(child->clone());
  }
  return cl;
}
std::unique_ptr<Node> CheckboxItem::clone() const {
  auto item = std::make_unique<CheckboxItem>();
  item->setChecked(m_checked);
  for (auto& child : m_children) {
    item->appendChild(child->clone());
  }
  return item;
}
std::unique_ptr<Node> Table::clone() const {
  auto t = std::make_unique<Table>();
  t->setHeader(m_header);
  for (auto& row : m_content) {
    t->appendRow(row);
  }
  return t;
}
std::unique_ptr<Node> LatexBlock::clone() const {
  auto lb = std::make_unique<LatexBlock>();
  for (auto& child : m_children) {
    lb->appendChild(child->clone());
  }
  return lb;
}
}  // namespace md::parser
