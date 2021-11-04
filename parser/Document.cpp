//
// Created by pikachu on 2021/1/31.
//

#include "Document.h"

#include <utility>

#include "Parser.h"
#include "debug.h"
#include "magic_enum.hpp"
namespace md::parser {
QDebug operator<<(QDebug debug, const NodeType &type) {
  QDebugStateSaver saver(debug);
  auto str = std::string(magic_enum::enum_name(type));
  debug.nospace() << QString::fromStdString(str);
  return debug;
}
struct DefaultHtmlVisitor
    : MultipleVisitor<Header, Text, ItalicText, BoldText, ItalicBoldText, Image,
                      Link, CodeBlock, InlineCode, Paragraph, CheckboxList,
                      CheckboxItem, UnorderedList, OrderedList,
                      UnorderedListItem, OrderedListItem, Hr, QuoteBlock, Table,
                      Lf> {
  DefaultHtmlVisitor(const String &text) : m_text(text) {}
  void visit(Header *node) override {
    auto hn = "h" + String::number(node->level());
    m_html += "<" + hn + ">";
    for (auto it : node->children()) {
      it->accept(this);
    }
    m_html += "</" + hn + ">\n";
  }
  void visit(Text *node) override {
    m_html += m_text.mid(node->offset(), node->length());
  }
  void visit(ItalicText *node) override {
    m_html += "<em>";
    node->text()->accept(this);
    m_html += "</em>";
  }
  void visit(BoldText *node) override {
    m_html += "<strong>";
    node->text()->accept(this);
    m_html += "</strong>";
  }
  void visit(ItalicBoldText *node) override {
    m_html += "<strong><em>";
    node->text()->accept(this);
    m_html += "</strong></em>";
  }
  void visit(Image *node) override {
    m_html += R"(<img alt=")";
    if (node->alt()) {
      node->alt()->accept(this);
    } else {
      qDebug() << "image alt is null";
    }
    m_html += R"(" src=")";
    if (node->src()) {
      node->src()->accept(this);
    } else {
      qDebug() << "image src is null";
    }
    m_html += R"(" />)";
    m_html += "\n";
  }
  void visit(Link *node) override {
    m_html += R"(<a href=")";
    if (node->href()) {
      node->href()->accept(this);
    } else {
      qDebug() << "link href is null";
    }
    m_html += R"(">)";
    if (node->content()) {
      node->content()->accept(this);
    } else {
      qDebug() << "link content is null";
    }
    m_html += R"(</a>)";
    m_html += "\n";
  }
  void visit(CodeBlock *node) override {
    m_html += "<pre><code>\n";
    for (auto child : node->children()) {
      child->accept(this);
    }
    m_html += "</code></pre>\n";
  }
  void visit(InlineCode *node) override {
    m_html += "<code>";
    if (auto code = node->code(); code) {
      code->accept(this);
    }
    m_html += "</code>\n";
  }
  void visit(Paragraph *node) override {
    if (node->children().empty()) return;
    m_html += "<p>";
    for (auto it : node->children()) {
      it->accept(this);
    }
    m_html += "</p>\n";
  }
  void visit(CheckboxList *node) override {
    for (auto it : node->children()) {
      if (it->isChecked()) {
        m_html += "\t<div><input type=\"checkbox\" checked/>";
      } else {
        m_html += "\t<div><input type=\"checkbox\"/>";
      }
      it->accept(this);
      m_html += "</div>\n";
    }
  }
  void visit(CheckboxItem *node) override {
    for (auto it : node->children()) {
      it->accept(this);
    }
  }
  void visit(UnorderedList *node) override {
    m_html += "<ul>\n";
    for (auto it : node->children()) {
      m_html += "\t<li>";
      it->accept(this);
      m_html += "</li>\n";
    }
    m_html += "</ul>\n";
  }
  void visit(OrderedList *node) override {
    m_html += "<ol>\n";
    for (auto it : node->children()) {
      m_html += "\t<li>";
      it->accept(this);
      m_html += "</li>\n";
    }
    m_html += "</ol>\n";
  }
  void visit(OrderedListItem *node) override {
    for (auto child : node->children()) {
      child->accept(this);
    }
  }
  void visit(UnorderedListItem *node) override {
    for (auto child : node->children()) {
      child->accept(this);
    }
  }
  void visit(Hr *node) override { m_html += "<hr/>\n"; }
  void visit(Lf *node) override { m_html += "\n"; }
  void visit(QuoteBlock *node) override {
    m_html += "<blockquote>\n";
    for (auto it : node->children()) {
      it->accept(this);
      m_html += "\n";
    }
    m_html += "</blockquote>\n";
  }
  void visit(Table *node) override {
    m_html += "<table>\n";
    m_html += "<thead><tr>";
    for (const auto &content : node->header()) {
      m_html += "<th>";
      m_html += content;
      m_html += "</th>";
    }
    m_html += "</tr></thead>\n";
    m_html += "<tbody>\n";
    for (const auto &row : node->content()) {
      m_html += "<tr>";
      for (const auto &content : row) {
        m_html += "<th>";
        m_html += content;
        m_html += "</th>";
      }
      m_html += "</tr>";
    }
    m_html += "</tbody>";
    m_html += "</table>\n";
  }
  String html() { return m_html; }

 private:
  String m_html;
  const String &m_text;
};
Header::Header(int level) : m_level(level) { m_type = NodeType::header; }

Document::Document(String str) : m_text(std::move(str)) {}

String Document::toHtml() {
  auto visitor = new DefaultHtmlVisitor(m_text);
  for (auto it : m_root->children()) {
    it->accept(visitor);
  }
  auto ret = visitor->html();
  delete visitor;
  return ret;
}

void Document::accept(VisitorNode *visitor) {
  for (auto it : m_root->children()) {
    it->accept(visitor);
  }
}
void Container::appendChildren(QList<Text *> &children) {
  if (children.empty()) return;
  m_children.reserve(m_children.size() + children.size());
  for (auto &node : children) {
    node->setParent(this);
    m_children.append(node);
  }
}
}  // namespace md::parser