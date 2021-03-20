//
// Created by pikachu on 2021/3/19.
//

#include "Editor.h"
#include <QApplication>
#include <QDate>
#include <QFile>
#include <QDebug>
#include <QTextFrame>
#include <QTextList>
#include <QTextBlock>
#include <QTextTable>
#include "QtMarkdownParser"

struct DefaultEditorVisitor: MultipleVisitor<Header,
        Text, ItalicText, BoldText, ItalicBoldText,
        Image, Link, CodeBlock, InlineCode, Paragraph,
        UnorderedList, OrderedList,
        Hr, QuoteBlock, Table> {
    explicit DefaultEditorVisitor(Editor* editor):
        m_editor(editor), m_cursor(editor->textCursor()) {

    }
    void visit(Header *node) override {
        QTextBlockFormat format;
        auto font = m_editor->font();
        font.setPixelSize(16);
        m_cursor.insertBlock(format);
        auto hn = "h" + String::number(node->level());
        m_cursor.insertText(hn);
        for(auto it: node->children()) {
            it->accept(this);
        }
        m_cursor.insertBlock();
    }
    void visit(Text *node) override {
        m_html += node->str();
        m_cursor.insertText(node->str());
    }
    void visit(ItalicText *node) override {
        QTextCharFormat format;
        auto font = m_editor->font();
        font.setPixelSize(16);
        format.setFont(font);
        font.setItalic(true);
        m_cursor.insertText(node->str());
    }
    void visit(BoldText *node) override {
        QTextCharFormat format;
        auto font = m_editor->font();
        font.setPixelSize(16);
        format.setFont(font);
        font.setBold(true);
        m_cursor.insertText(node->str());
    }
    void visit(ItalicBoldText *node) override {
        QTextCharFormat format;
        auto font = m_editor->font();
        font.setPixelSize(16);
        format.setFont(font);
        font.setBold(true);
        font.setItalic(true);
        m_cursor.insertText(node->str());
    }
    void visit(Image *node) override {
        QTextImageFormat imageFormat;
        imageFormat.setName(node->src()->str());
        imageFormat.setWidth(600);
        m_cursor.insertImage(imageFormat);
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
        auto mainFrame = m_cursor.currentFrame();
        QTextFrameFormat blockFormat;
        blockFormat.setBorder(1);
        blockFormat.setBackground(QBrush(QColor("#f7f7f7")));
        blockFormat.setPadding(16);
        m_cursor.insertFrame(blockFormat);
        m_cursor.insertText(node->code()->str());
        m_cursor = mainFrame->lastCursorPosition();

    }
    void visit(InlineCode *node) override {
        QTextCharFormat format;
        format.setBackground(QBrush(QColor("#f7f7f7")));
        m_cursor.insertText(node->code()->str(), format);
    }
    void visit(Paragraph *node) override {
        m_cursor.insertBlock();
        for(auto it: node->children()) {
            it->accept(this);
        }
    }
    void visit(UnorderedList *node) override {
        for(auto it: node->children()) {
            QTextListFormat listFormat;
            listFormat.setStyle(QTextListFormat::ListDisc);
            m_cursor.insertList(listFormat);
            it->accept(this);
        }
    }
    void visit(OrderedList *node) override {
        QTextListFormat listFormat;
        listFormat.setStyle(QTextListFormat::ListDecimal);
        m_cursor.insertList(listFormat);
        for(auto it: node->children()) {
            m_cursor.insertBlock();
            it->accept(this);
        }
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
    Editor* m_editor;
    QTextCursor m_cursor;
};
Editor::Editor(QWidget *parent) : QTextEdit(parent) {

    QFile mdFile("../test.md");
    if (!mdFile.exists()) {
        qDebug() << "file not exist:" << mdFile.fileName();
        return;
    }
    mdFile.open(QIODevice::ReadOnly);
    auto mdText = mdFile.readAll();
    mdFile.close();
    qDebug().noquote().nospace() << mdText;
    Document doc(mdText);
    DefaultEditorVisitor visitor(this);
    doc.accept(&visitor);
    QTextCharFormat backgroundFormat;
    backgroundFormat.setBackground(QColor("lightGray"));
    QTextCursor cursor(textCursor());
    cursor.insertText(tr("Character formats"),
                      backgroundFormat);

    cursor.insertBlock();

    cursor.insertText(tr("Text can be displayed in a variety of "
                         "different character formats. "), backgroundFormat);
    cursor.insertText(tr("We can emphasize text by "));
    cursor.insertText(tr("making it italic"), backgroundFormat);
    QTextTableFormat tableFormat;
    tableFormat.setBackground(QColor("#e0e0e0"));
    tableFormat.setBorder(0.5);
    QVector<QTextLength> constraints;
    constraints << QTextLength(QTextLength::PercentageLength, 16);
    constraints << QTextLength(QTextLength::PercentageLength, 28);
    constraints << QTextLength(QTextLength::PercentageLength, 28);
    constraints << QTextLength(QTextLength::PercentageLength, 28);
    tableFormat.setColumnWidthConstraints(constraints);
    QTextTable *table = cursor.insertTable(3, 4, tableFormat);
    int rows = 3;
    int columns = 4;
    QTextCharFormat charFormat;
    for (int column = 1; column < columns; ++column) {
        auto cell = table->cellAt(0, column);
        auto cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(tr("Team %1").arg(column), charFormat);
    }

    for (int row = 1; row < rows; ++row) {
        auto cell = table->cellAt(row, 0);
        auto cellCursor = cell.firstCursorPosition();
        cellCursor.insertText(tr("%1").arg(row), charFormat);

        for (auto column = 1; column < columns; ++column) {
            if ((row-1) % 3 == column-1) {
                cell = table->cellAt(row, column);
                QTextCursor cellCursor = cell.firstCursorPosition();
                cellCursor.insertText(tr("On duty"), charFormat);
            }
        }
    }
    resize(800, 500);
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Editor w;
    w.show();
    return QApplication::exec();
}