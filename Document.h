//
// Created by pikachu on 2021/1/31.
//

#ifndef MD_DOCUMENT_H
#define MD_DOCUMENT_H

#include "Token.h"
#include "Visitor.h"
#include <vector>
enum class NodeType {
    none,
    header,
    paragraph,
    text,
    image,
    link,
    code_block, inline_code,
    ul,
    ol,
    hr,
    quote_block,
    italic, bold, italic_bold,
    table
};
class Node {
public:
    Node(NodeType type = NodeType::none): m_type(type) {}
    virtual void accept(VisitorNode*) = 0;
protected:
    NodeType m_type;
};
template<typename T>
struct Visitable: public Node {
    void accept(VisitorNode* v) override {
        auto p = dynamic_cast<Visitor<T>*>(v);
        if (p) {
            p->visit(static_cast<T*>(this));
        }
    }
};
using NodePtrList = QList<Node*>;
class Container : public Node {
public:
    Container() { }
    std::vector<Node*>& children() { return m_children; }
    void appendChild(Node* child) { m_children.push_back(child); }
protected:
    std::vector<Node*> m_children;
};

template<typename T>
struct ContainerVisitable: public Container {
    void accept(VisitorNode* v) override {
        if (auto p = dynamic_cast<Visitor<T>*>(v); p) {
            p->visit(static_cast<T*>(this));
        }
    }
};
class Header: public ContainerVisitable<Header> {
public:
    explicit Header(int level, String content);
    int level() { return m_level; }
private:
    int m_level;
};
class Paragraph: public ContainerVisitable<Paragraph> {
public:
    Paragraph() { m_type = NodeType::paragraph; }
};
class UnorderedList: public ContainerVisitable<UnorderedList> {
public:
    UnorderedList() { m_type = NodeType::ul; }
};

class OrderedList: public ContainerVisitable<OrderedList> {
public:
    OrderedList() { m_type = NodeType::ol; }
};
class QuoteBlock: public ContainerVisitable<QuoteBlock> {
public:
    QuoteBlock() { m_type = NodeType::quote_block; }
};
class Text: public Visitable<Text> {
public:
    Text(String str): m_str(str) { m_type = NodeType::text; }
    String str() { return m_str; }
    void setStr(String str) { m_str = str; }
private:
    String m_str;
};
class ItalicText: public Visitable<ItalicText> {
public:
    ItalicText(String str): m_str(str) { m_type = NodeType::italic; }
    String str() { return m_str; }
    void setStr(String str) { m_str = str; }
private:
    String m_str;
};
class BoldText: public Visitable<BoldText> {
public:
    BoldText(String str): m_str(str) { m_type = NodeType::bold; }
    String str() { return m_str; }
    void setStr(String str) { m_str = str; }
private:
    String m_str;
};
class ItalicBoldText: public Visitable<ItalicBoldText> {
public:
    ItalicBoldText(String str): m_str(str) { m_type = NodeType::bold; }
    String str() { return m_str; }
    void setStr(String str) { m_str = str; }
private:
    String m_str;
};
class Image: public Visitable<Image> {
public:
    Image(Text* alt, Text* src): m_alt(alt), m_src(src) { m_type = NodeType::image; }
    Text* alt() { return m_alt; }
    Text* src() { return m_src; }
private:
    Text* m_alt;
    Text* m_src;
};
class Link: public Visitable<Link> {
public:
    Link(Text* content, Text* href): m_content(content), m_href(href) { m_type = NodeType::link; }
    Text* content() { return m_content; }
    Text* href() { return m_href; }
private:
    Text* m_content;
    Text* m_href;
};
class CodeBlock: public Visitable<CodeBlock> {
public:
    CodeBlock(Text* name, Text* code): m_name(name), m_code(code) { m_type = NodeType::code_block; }
    Text* name() { return m_name; }
    Text* code() { return m_code; }
private:
    Text* m_name;
    Text* m_code;
};
class Hr: public Visitable<Hr> {
public:
    Hr() { m_type = NodeType::hr; }
};
class InlineCode: public Visitable<InlineCode> {
public:
    InlineCode(Text* code): m_code(code) { m_type = NodeType::inline_code; }
    Text* code() { return m_code; }
private:
    Text* m_code;
};
class Table: public Visitable<Table> {
public:
    Table() { m_type = NodeType::table; }
    void appendRow(const StringList& row) {
        m_content.append(row);
    }
    void setHeader(const StringList& header) { m_header = header; }
    StringList& header() { return m_header; }
    QList<StringList>& content() { return m_content; }
private:
    StringList m_header;
    QList<StringList> m_content;
};
class Document {
public:
    explicit Document(String str);
    String toHtml();
private:
    NodePtrList m_nodes;
};


#endif //MD_DOCUMENT_H
