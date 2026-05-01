//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_DOCUMENT_H
#define MD_DOCUMENT_H
#include <QDebug>
#include <QRect>
#include <iostream>
#include <memory>

#include "QtMarkdown_global.h"
#include "Token.h"
#include "Visitor.h"
#include "mddef.h"
namespace md::parser {
enum class QTMARKDOWNSHARED_EXPORT NodeType {
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
QTMARKDOWNSHARED_EXPORT QDebug operator<<(QDebug debug, const NodeType& type);
QTMARKDOWNSHARED_EXPORT std::ostream& operator<<(std::ostream& os, const NodeType& type);
class QTMARKDOWNSHARED_EXPORT Node {
 public:
  explicit Node(NodeType type = NodeType::none, Node* parent = nullptr) : m_type(type), m_parent(parent) {}
  virtual ~Node() {}
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
using NodePtrList = std::vector<std::unique_ptr<Node>>;
class Text;
class QTMARKDOWNSHARED_EXPORT Container : public Node {
 public:
  Container() = default;
  NodePtrList& children() { return m_children; }
  void setChildren(NodePtrList&& children) {
    m_children = std::move(children);
    for (auto& child : m_children) {
      child->setParent(this);
    }
  }
  void setChild(SizeType index, std::unique_ptr<Node> node);
  void insertChild(SizeType index, std::unique_ptr<Node> node);
  void appendChild(std::unique_ptr<Node> child) {
    child->setParent(this);
    m_children.push_back(std::move(child));
  }
  void appendChildren(NodePtrList&& children);
  void appendChildren(std::vector<std::unique_ptr<Text>>&& texts);
  [[nodiscard]] Node* childAt(SizeType index) const;
  void removeChildAt(SizeType index);
  void removeChild(Node* node);
  SizeType indexOf(Node* child) const;
  std::unique_ptr<Node>& operator[](SizeType index);
  const std::unique_ptr<Node>& operator[](SizeType index) const;
  auto empty() const { return m_children.empty(); }
  [[nodiscard]] auto size() const { return m_children.size(); }
  void accept(VisitorNode* v) override {
    for (auto& node : m_children) {
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
class QTMARKDOWNSHARED_EXPORT CheckboxList : public ContainerVisitable<CheckboxList> {
 public:
  CheckboxList() { m_type = NodeType::checkbox; }
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
  explicit ItalicText(std::unique_ptr<Text> text);
  ~ItalicText();
  [[nodiscard]] Text* text() const { return m_text.get(); }

 private:
  std::unique_ptr<Text> m_text;
};
class QTMARKDOWNSHARED_EXPORT BoldText : public Visitable<BoldText> {
 public:
  explicit BoldText(std::unique_ptr<Text> text);
  ~BoldText();
  [[nodiscard]] Text* text() const { return m_text.get(); }

 private:
  std::unique_ptr<Text> m_text;
};
class QTMARKDOWNSHARED_EXPORT ItalicBoldText : public Visitable<ItalicBoldText> {
 public:
  explicit ItalicBoldText(std::unique_ptr<Text> text);
  ~ItalicBoldText();
  [[nodiscard]] Text* text() const { return m_text.get(); }

 private:
  std::unique_ptr<Text> m_text;
};
class QTMARKDOWNSHARED_EXPORT StrickoutText : public Visitable<StrickoutText> {
 public:
  explicit StrickoutText(std::unique_ptr<Text> text);
  ~StrickoutText();
  [[nodiscard]] Text* text() const { return m_text.get(); }

 private:
  std::unique_ptr<Text> m_text;
};
class QTMARKDOWNSHARED_EXPORT Image : public Visitable<Image> {
 public:
  Image(std::unique_ptr<Text> alt, std::unique_ptr<Text> src);
  ~Image();
  Text* alt() { return m_alt.get(); }
  Text* src() { return m_src.get(); }

 private:
  std::unique_ptr<Text> m_alt;
  std::unique_ptr<Text> m_src;
};
class QTMARKDOWNSHARED_EXPORT Link : public Visitable<Link> {
 public:
  Link(std::unique_ptr<Text> content, std::unique_ptr<Text> href);
  ~Link();
  Text* content() { return m_content.get(); }
  Text* href() { return m_href.get(); }

 private:
  std::unique_ptr<Text> m_content;
  std::unique_ptr<Text> m_href;
};
class QTMARKDOWNSHARED_EXPORT CodeBlock : public ContainerVisitable<CodeBlock> {
 public:
  explicit CodeBlock(std::unique_ptr<Text> name);
  ~CodeBlock();
  Text* name() { return m_name.get(); }

 private:
  std::unique_ptr<Text> m_name;
};
class LatexBlock;
class QTMARKDOWNSHARED_EXPORT Hr : public Visitable<Hr> {
 public:
  Hr() { m_type = NodeType::hr; }
};
class QTMARKDOWNSHARED_EXPORT InlineCode : public Visitable<InlineCode> {
 public:
  explicit InlineCode(std::unique_ptr<Text> code);
  ~InlineCode();
  Text* code() { return m_code.get(); }

 private:
  std::unique_ptr<Text> m_code;
};
class QTMARKDOWNSHARED_EXPORT InlineLatex : public Visitable<InlineLatex> {
 public:
  explicit InlineLatex(std::unique_ptr<Text> code);
  ~InlineLatex();
  Text* code() { return m_code.get(); }

 private:
  std::unique_ptr<Text> m_code;
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
