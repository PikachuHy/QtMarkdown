//
// Created by PikachuHy on 2021/11/5.
//

#include "MarkdownSerializer.h"
#include <string>
#include "debug.h"
#include "parser/Document.h"
#include "parser/Text.h"

using namespace md::parser;

namespace md::editor {

MarkdownSerializer::MarkdownSerializer(const parser::IBufferProvider& doc) : m_doc(doc) {}

String MarkdownSerializer::markdown() const { return m_md; }

void MarkdownSerializer::visit(Header* node) {
    for (int i = 0; i < node->level(); ++i) {
        m_md += "#";
    }
    m_md += " ";
    for (auto& it : node->children()) {
        it->accept(this);
    }
    markContentEnd();
    m_md += "\n";
    m_md += "\n";
}

void MarkdownSerializer::recordTextPositions(const String& text) {
    if (!m_recordPositions) return;
    SizeType mdStart = m_md.length();
    SizeType textLen = text.length();
    m_contentToMarkdown.reserve(m_contentToMarkdown.size() + textLen);
    for (SizeType i = 0; i < textLen; ++i) {
        m_contentToMarkdown.push_back(mdStart + i);
    }
}

void MarkdownSerializer::visit(Text* node) {
    String text = node->toString(m_doc);
    recordTextPositions(text);
    m_md += text;
}

void MarkdownSerializer::visit(ItalicText* node) {
    m_md += "*";
    node->text()->accept(this);
    m_md += "*";
}

void MarkdownSerializer::visit(BoldText* node) {
    m_md += "**";
    node->text()->accept(this);
    m_md += "**";
}

void MarkdownSerializer::visit(ItalicBoldText* node) {
    m_md += "***";
    node->text()->accept(this);
    m_md += "***";
}

void MarkdownSerializer::visit(StrickoutText* node) {
    m_md += "~~";
    node->text()->accept(this);
    m_md += "~~";
}

void MarkdownSerializer::visit(Image* node) {
    m_md += "![";
    if (node->alt()) {
        node->alt()->accept(this);
    } else {
        DEBUG << "image alt is null";
    }
    m_md += "](";
    m_recordPositions = false;
    if (node->src()) {
        node->src()->accept(this);
    } else {
        DEBUG << "image src is null";
    }
    m_recordPositions = true;
    m_md += ")";
}

void MarkdownSerializer::visit(Link* node) {
    m_md += "[";
    if (node->content()) {
        node->content()->accept(this);
    } else {
        DEBUG << "link content is null";
    }
    m_md += "](";
    m_recordPositions = false;
    if (node->href()) {
        node->href()->accept(this);
    } else {
        DEBUG << "link href is null";
    }
    m_recordPositions = true;
    m_md += ")";
}

void MarkdownSerializer::visit(CodeBlock* node) {
    m_md += "```";
    m_recordPositions = false;
    node->name()->accept(this);
    m_recordPositions = true;
    m_md += "\n";
    for (auto& child : node->children()) {
        child->accept(this);
        m_md += "\n";
    }
    // Ensure the closing "```" is always on its own line after a "\n",
    // so the content-to-markdown sentinel maps to a position where
    // inserted text produces valid fenced code block markdown.
    if (node->children().empty()) {
        markContentEnd();
        m_md += "\n";
    } else {
        markContentEnd();
    }
    m_md += "```";
    m_md += "\n";
    m_md += "\n";
}

void MarkdownSerializer::visit(InlineCode* node) {
    m_md += "`";
    if (auto code = node->code(); code) {
        code->accept(this);
    }
    m_md += "`";
}

void MarkdownSerializer::visit(Paragraph* node) {
    if (node->children().empty()) return;
    for (auto& it : node->children()) {
        it->accept(this);
    }
    markContentEnd();
    m_md += "\n";
    m_md += "\n";
}

void MarkdownSerializer::visit(CheckboxList* node) {
    for (SizeType i = 0; i < node->size(); ++i) {
        auto* child = node->childAt(i);
        ASSERT(child->type() == NodeType::checkbox_item);
        auto* item = static_cast<CheckboxItem*>(child);
        m_md += "- [";
        if (item->isChecked()) {
            m_md += "x";
        } else {
            m_md += " ";
        }
        m_md += "] ";
        item->accept(this);
        if (i + 1 < node->size()) {
            m_md += "\n";
        }
    }
    markContentEnd();
    m_md += "\n";
}

void MarkdownSerializer::visit(CheckboxItem* node) {
    for (auto& it : node->children()) {
        it->accept(this);
    }
}

void MarkdownSerializer::visit(UnorderedList* node) {
    for (SizeType i = 0; i < node->size(); ++i) {
        m_md += "- ";
        node->childAt(i)->accept(this);
        if (i + 1 < node->size()) {
            m_md += "\n";
        }
    }
    markContentEnd();
    m_md += "\n";
}

void MarkdownSerializer::visit(OrderedList* node) {
    for (SizeType i = 0; i < node->size(); ++i) {
        m_md += std::to_string(i + 1) + ". ";
        node->childAt(i)->accept(this);
        if (i + 1 < node->size()) {
            m_md += "\n";
        }
    }
    markContentEnd();
    m_md += "\n";
}

void MarkdownSerializer::visit(OrderedListItem* node) {
    for (auto& child : node->children()) {
        child->accept(this);
    }
}

void MarkdownSerializer::visit(UnorderedListItem* node) {
    for (auto& child : node->children()) {
        child->accept(this);
    }
}

void MarkdownSerializer::visit(Hr* node) { m_md += "---\n"; }

void MarkdownSerializer::visit(Lf* node) { m_md += "\n"; }

void MarkdownSerializer::visit(QuoteBlock* node) {
    m_md += "> ";
    for (auto& it : node->children()) {
        it->accept(this);
        m_md += "\n";
    }
    markContentEnd();
    m_md += "\n";
}

void MarkdownSerializer::visit(Table* node) {
    auto renderRow = [this](const StringList& cells) {
        m_md += "|";
        for (const auto& cell : cells) {
            m_md += " " + cell + " |";
        }
        m_md += "\n";
    };
    if (node->header().empty() && node->content().empty()) return;
    renderRow(node->header());
    m_md += "|";
    for (int i = 0; i < node->header().size(); ++i) {
        m_md += " --- |";
    }
    m_md += "\n";
    for (const auto& row : node->content()) {
        renderRow(row);
    }
    m_md += "\n";
}

void MarkdownSerializer::visit(LatexBlock* node) {
    m_md += "\n";
    m_md += "$$\n";
    for (auto& it : node->children()) {
        it->accept(this);
    }
    markContentEnd();
    m_md += "$$\n";
    m_md += "\n";
}

void MarkdownSerializer::visit(InlineLatex* node) {
    m_md += "$";
    node->code()->accept(this);
    m_md += "$";
}

} // namespace md::editor
