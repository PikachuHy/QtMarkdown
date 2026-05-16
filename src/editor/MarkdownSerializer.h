//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_MARKDOWNSERIALIZER_H
#define QTMARKDOWN_MARKDOWNSERIALIZER_H

#include "QtMarkdown_global.h"
#include "render/mddef.h"
#include "parser/IBufferProvider.h"
#include "parser/Visitor.h"

namespace md::editor {

class QTMARKDOWNEDITORCORE_EXPORT MarkdownSerializer : public parser::NodeVisitor {
public:
    explicit MarkdownSerializer(const parser::IBufferProvider& doc);
    String markdown() const;
    const std::vector<SizeType>& contentToMarkdown() const { return m_contentToMarkdown; }
    SizeType contentEndMarkdownPos() const { return m_contentEndMdPos; }
    void markContentEnd() { m_contentEndMdPos = m_md.length(); }

    // Visitor overrides
    void visit(parser::Header* node) override;
    void visit(parser::Text* node) override;
    void visit(parser::ItalicText* node) override;
    void visit(parser::BoldText* node) override;
    void visit(parser::ItalicBoldText* node) override;
    void visit(parser::StrickoutText* node) override;
    void visit(parser::Image* node) override;
    void visit(parser::Link* node) override;
    void visit(parser::CodeBlock* node) override;
    void visit(parser::InlineCode* node) override;
    void visit(parser::Paragraph* node) override;
    void visit(parser::CheckboxList* node) override;
    void visit(parser::CheckboxItem* node) override;
    void visit(parser::UnorderedList* node) override;
    void visit(parser::OrderedList* node) override;
    void visit(parser::OrderedListItem* node) override;
    void visit(parser::UnorderedListItem* node) override;
    void visit(parser::Hr* node) override;
    void visit(parser::Lf* node) override;
    void visit(parser::QuoteBlock* node) override;
    void visit(parser::Table* node) override;
    void visit(parser::LatexBlock* node) override;
    void visit(parser::InlineLatex* node) override;

private:
    void recordTextPositions(const String& text);
    String m_md;
    const parser::IBufferProvider& m_doc;
    std::vector<SizeType> m_contentToMarkdown;
    SizeType m_contentEndMdPos = 0;
    bool m_recordPositions = true;
};

} // namespace md::editor
#endif // QTMARKDOWN_MARKDOWNSERIALIZER_H
