//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_PARSER_NODE_H
#define MD_PARSER_NODE_H

#include <iostream>
#include <memory>
#include <vector>

#include "QtMarkdown_global.h"
#include "Token.h"
#include "Visitor.h"
#include "mddef.h"

namespace md::parser {
class IBufferProvider;

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
  checkbox,  // Note: corresponding class is CheckboxList, not Checkbox
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
QTMARKDOWNSHARED_EXPORT std::ostream& operator<<(std::ostream& os, const NodeType& type);

class QTMARKDOWNSHARED_EXPORT Node {
 public:
  explicit Node(NodeType type = NodeType::none, Node* parent = nullptr) : m_type(type), m_parent(parent) {}
  virtual ~Node() {}
  virtual void accept(NodeVisitor*) = 0;
  virtual std::unique_ptr<Node> clone() const = 0;
  NodeType type() { return m_type; }
  void setParent(Node* parent) { m_parent = parent; }
  [[nodiscard]] Node* parent() const { return m_parent; }

  virtual SizeType contentLength(const IBufferProvider& doc) const { return 0; }

 protected:
  NodeType m_type;
  Node* m_parent;
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
  // Subclasses MUST override accept() to call v->visit(this).
  // Example: class Header : public Container {
  //   void accept(NodeVisitor* v) override { v->visit(this); }
  // };
  // The base implementation dispatches as Container* (type-erased fallback).
  // If not overridden, visitors see Container* instead of the concrete type,
  // which typically results in a silent no-op in the visitor.
  void accept(NodeVisitor* v) override {
    v->visit(this);
    for (auto& node : m_children) {
      node->accept(v);
    }
  }
  std::unique_ptr<Node> clone() const override {
    auto c = std::make_unique<Container>();
    for (auto& child : m_children) {
      c->appendChild(child->clone());
    }
    return c;
  }

  SizeType contentLength(const IBufferProvider& doc) const override {
    SizeType total = 0;
    for (auto& child : m_children) {
      total += child->contentLength(doc);
    }
    return total;
  }

 protected:
  NodePtrList m_children;
};

}  // namespace md::parser

#endif  // MD_PARSER_NODE_H
