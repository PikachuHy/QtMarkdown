//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_DOCUMENT_H
#define MD_DOCUMENT_H
#include <QDebug>
#include <QRect>

#include "QtMarkdown_global.h"
#include "Token.h"
#include "Visitor.h"
#include "mddef.h"
namespace md::parser {
enum class NodeType {
  none,
  header,
  paragraph,
  text,
  image,
  link,
  code_block,
  inline_code,
  latex_block,
  inline_latex,
  checkbox,
  checkbox_item,
  ul,
  ul_item,
  ol,
  ol_item,
  hr,
  quote_block,
  italic,
  bold,
  italic_bold,
  strickout,
  table,
  lf  // 换行
};
QDebug operator<<(QDebug debug, const NodeType& type);

class QTMARKDOWNSHARED_EXPORT Node {
 public:
  explicit Node(NodeType type = NodeType::none, Node* parent = nullptr) : m_type(type), m_parent(parent) {}
  virtual void accept(VisitorNode*) = 0;
  NodeType type() { return m_type; }
  void setParent(Node* parent) { m_parent = parent; }
  [[nodiscard]] Node* parent() const { return m_parent; }

 protected:
  NodeType m_type;
  Node* m_parent;
};
template <typename T>
struct Visitable : public Node {
  void accept(VisitorNode* v) override {
    auto p = dynamic_cast<Visitor<T>*>(v);
    if (p) {
      p->visit(static_cast<T*>(this));
    }
  }
};
using NodePtrList = QList<Node*>;
class Text;
class QTMARKDOWNSHARED_EXPORT Container : public Node {
 public:
  Container() = default;
  NodePtrList& children() { return m_children; }
  void appendChild(Node* child) {
    m_children.push_back(child);
    child->setParent(this);
  }
  void appendChildren(QList<Text*>& children);
  void accept(VisitorNode* v) override {
    for (auto node : m_children) {
      node->accept(v);
    }
  }

 protected:
  NodePtrList m_children;
};

template <typename T>
struct ContainerVisitable : public Container {
  void accept(VisitorNode* v) override {
    if (auto p = dynamic_cast<Visitor<T>*>(v); p) {
      p->visit(static_cast<T*>(this));
    }
  }
};
class QTMARKDOWNSHARED_EXPORT Header : public ContainerVisitable<Header> {
 public:
  explicit Header(int level);
  [[nodiscard]] int level() const { return m_level; }

 private:
  int m_level;
};
class QTMARKDOWNSHARED_EXPORT Paragraph : public ContainerVisitable<Paragraph> {
 public:
  Paragraph() { m_type = NodeType::paragraph; }
};
class QTMARKDOWNSHARED_EXPORT CheckboxItem : public ContainerVisitable<CheckboxItem> {
 public:
  CheckboxItem() : m_checked(false) { m_type = NodeType::checkbox_item; }
  [[nodiscard]] bool isChecked() const { return m_checked; }
  void setChecked(bool flag) { m_checked = flag; }

 private:
  bool m_checked;
};
class QTMARKDOWNSHARED_EXPORT CheckboxList : public Visitable<CheckboxList> {
 public:
  CheckboxList() { m_type = NodeType::checkbox; }
  [[nodiscard]] QList<CheckboxItem*> children() const { return m_children; }
  void appendChild(CheckboxItem* item) { m_children.append(item); }

 private:
  QList<CheckboxItem*> m_children;
};
class QTMARKDOWNSHARED_EXPORT UnorderedList : public ContainerVisitable<UnorderedList> {
 public:
  UnorderedList() { m_type = NodeType::ul; }
};

class QTMARKDOWNSHARED_EXPORT UnorderedListItem : public ContainerVisitable<UnorderedListItem> {
 public:
  UnorderedListItem() { m_type = NodeType::ul_item; }
};

class QTMARKDOWNSHARED_EXPORT OrderedList : public ContainerVisitable<OrderedList> {
 public:
  OrderedList() { m_type = NodeType::ol; }
};
class QTMARKDOWNSHARED_EXPORT OrderedListItem : public ContainerVisitable<OrderedListItem> {
 public:
  OrderedListItem() { m_type = NodeType::ol_item; }
};
class QTMARKDOWNSHARED_EXPORT QuoteBlock : public ContainerVisitable<QuoteBlock> {
 public:
  QuoteBlock() { m_type = NodeType::quote_block; }
};

class QTMARKDOWNSHARED_EXPORT ItalicText : public Visitable<ItalicText> {
 public:
  explicit ItalicText(Text* text);
  [[nodiscard]] Text* text() const { return m_text; }

 private:
  Text* m_text;
};
class QTMARKDOWNSHARED_EXPORT BoldText : public Visitable<BoldText> {
 public:
  explicit BoldText(Text* text);

  [[nodiscard]] Text* text() const { return m_text; }

 private:
  Text* m_text;
};
class QTMARKDOWNSHARED_EXPORT ItalicBoldText : public Visitable<ItalicBoldText> {
 public:
  explicit ItalicBoldText(Text* text);

  [[nodiscard]] Text* text() const { return m_text; }

 private:
  Text* m_text;
};
class QTMARKDOWNSHARED_EXPORT StrickoutText : public Visitable<StrickoutText> {
 public:
  explicit StrickoutText(Text* text);

  [[nodiscard]] Text* text() const { return m_text; }

 private:
  Text* m_text;
};
class QTMARKDOWNSHARED_EXPORT Image : public Visitable<Image> {
 public:
  Image(Text* alt, Text* src) : m_alt(alt), m_src(src) { m_type = NodeType::image; }
  Text* alt() { return m_alt; }
  Text* src() { return m_src; }

 private:
  Text* m_alt;
  Text* m_src;
};
class QTMARKDOWNSHARED_EXPORT Link : public Visitable<Link> {
 public:
  Link(Text* content, Text* href);
  Text* content() { return m_content; }
  Text* href() { return m_href; }

 private:
  Text* m_content;
  Text* m_href;
};
class QTMARKDOWNSHARED_EXPORT CodeBlock : public ContainerVisitable<CodeBlock> {
 public:
  explicit CodeBlock(Text* name) : m_name(name) { m_type = NodeType::code_block; }
  Text* name() { return m_name; }

 private:
  Text* m_name;
};
class LatexBlock;
class QTMARKDOWNSHARED_EXPORT Hr : public Visitable<Hr> {
 public:
  Hr() { m_type = NodeType::hr; }
};
class QTMARKDOWNSHARED_EXPORT InlineCode : public Visitable<InlineCode> {
 public:
  explicit InlineCode(Text* code) : m_code(code) { m_type = NodeType::inline_code; }
  Text* code() { return m_code; }

 private:
  Text* m_code;
};
class QTMARKDOWNSHARED_EXPORT InlineLatex : public Visitable<InlineLatex> {
 public:
  explicit InlineLatex(Text* code) : m_code(code) { m_type = NodeType::inline_latex; }
  Text* code() { return m_code; }

 private:
  Text* m_code;
};
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
class QTMARKDOWNSHARED_EXPORT Lf : public Visitable<Lf> {
 public:
  Lf() { m_type = NodeType::lf; }
};
class QTMARKDOWNSHARED_EXPORT Document {
 public:
  explicit Document(const String& str);
  String toHtml();
  void accept(VisitorNode* visitor);

 protected:
  String m_originalBuffer;
  String m_addBuffer;
  sptr<Container> m_root;
  friend class Parser;
  friend class Text;
  friend class PieceTableItem;
};

}  // namespace md::parser
#endif  // MD_DOCUMENT_H
