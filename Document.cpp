//
// Created by pikachu on 2021/1/31.
//

#include "Document.h"
#include "Parser.h"
#include <QDebug>
#include "magic_enum.hpp"
QDebug operator<<(QDebug debug, const NodeType &type) {
  QDebugStateSaver saver(debug);
  QString str = QString::fromStdString(std::string(magic_enum::enum_name(type)));
  debug.nospace() << str;
  return debug;
}
struct DefaultHtmlVisitor: MultipleVisitor<Header,
        Text, ItalicText, BoldText, ItalicBoldText,
        Image, Link, CodeBlock, InlineCode, Paragraph,
        CheckboxList, UnorderedList, OrderedList,
        Hr, QuoteBlock, Table> {
    void visit(Header *node) override {
        auto hn = "h" + String::number(node->level());
        m_html += "<" + hn + ">";
        for(auto it: node->children()) {
            it->accept(this);
        }
        m_html += "</" + hn + ">\n";
    }
    void visit(Text *node) override {
        m_html += node->str();
    }
    void visit(ItalicText *node) override {
        m_html += "<em>" + node->str() + "</em>";
    }
    void visit(BoldText *node) override {
        m_html += "<strong>" + node->str() + "</strong>";
    }
    void visit(ItalicBoldText *node) override {
        m_html += "<strong><em>" + node->str() + "</strong></em>";
    }
    void visit(Image *node) override {
        m_html += R"(<img alt=")";
        if (node->alt()) {
            node->alt()->accept(this);
        } else {
            qDebug() << "image alt is null";
        }
        m_html += R"(" src=")";
        if(node->src()) {
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
        if(node->content()) {
            node->content()->accept(this);
        } else {
            qDebug() << "link content is null";
        }
        m_html += R"(</a>)";
        m_html += "\n";
    }
    void visit(CodeBlock *node) override {
        m_html += "<pre><code>\n";
        auto code = node->code()->str();
        m_html += code.toHtmlEscaped();
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
        m_html += "<p>";
        for(auto it: node->children()) {
            it->accept(this);
        }
        m_html += "</p>\n";
    }
    void visit(CheckboxList *node) override {
        for(auto it: node->children()) {
            m_html += "\t<div><input type=\"checkbox\"/>";
            it->accept(this);
            m_html += "</div>\n";
        }
    }
    void visit(UnorderedList *node) override {
        m_html += "<ul>\n";
        for(auto it: node->children()) {
            m_html += "\t<li>";
            it->accept(this);
            m_html += "</li>\n";
        }
        m_html += "</ul>\n";
    }
    void visit(OrderedList *node) override {
        m_html += "<ol>\n";
        for(auto it: node->children()) {
            m_html += "\t<li>";
            it->accept(this);
            m_html += "</li>\n";
        }
        m_html += "</ol>\n";
    }
    void visit(Hr *node) override {
        m_html += "<hr/>\n";
    }
    void visit(QuoteBlock *node) override {
        m_html += "<blockquote>\n";
        for(auto it: node->children()) {
            it->accept(this);
            m_html += "\n";
        }
        m_html += "</blockquote>\n";
    }
    void visit(Table *node) override {
        m_html += "<table>\n";
        m_html += "<thead><tr>";
        for(const auto &content: node->header()) {
            m_html += "<th>";
            m_html += content;
            m_html += "</th>";
        }
        m_html += "</tr></thead>\n";
        m_html += "<tbody>\n";
        for(const auto & row: node->content()) {
            m_html += "<tr>";
            for(const auto & content: row) {
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
};
Header::Header(int level) : m_level(level) {
    m_type = NodeType::header;
}

Document::Document(String str) {
    Parser parser;
    m_nodes = parser.parse(str);
}

String Document::toHtml() {
    auto visitor = new DefaultHtmlVisitor();
    for(auto it: m_nodes) {
        it->accept(visitor);
    }
    auto ret = visitor->html();
    delete visitor;
    return ret;
}

void Document::accept(VisitorNode* visitor) {
    for(auto it: m_nodes) {
        it->accept(visitor);
    }
}
