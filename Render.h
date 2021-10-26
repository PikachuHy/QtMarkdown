//
// Created by pikachu on 5/22/2021.
//

#ifndef QTMARKDOWN_RENDER_H
#define QTMARKDOWN_RENDER_H
#include "QtMarkdown_global.h"
#include "Document.h"
#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QDir>
#include <QFile>
#include <QProcess>
#include <QStack>
#include <QTemporaryFile>

namespace Element {
    struct Link {
        QString text;
        QString url;
        QList<QRect> rects;
    };
    struct Image {
        QString path;
        QRect rect;
    };
    struct CodeBlock {
        QString code;
        QRect rect;
    };
}
class TexRender;
class QTMARKDOWNSHARED_EXPORT Render: public MultipleVisitor<Header,
        Text, ItalicText, BoldText, ItalicBoldText,
        Image, Link, CodeBlock, InlineCode, Paragraph,
        CheckboxList, CheckboxItem,
        UnorderedList, UnorderedListItem,
        OrderedList, OrderedListItem,
        LatexBlock, InlineLatex,
        Hr, QuoteBlock, Table> {
public:
    explicit Render(int w, int rightMargin, const QString& filePath);
    ~Render();
    void reset(QPainter* painter);
    // 隔离QPainter
    void save();
    void restore();
    void setFont(const QFont& font);
    void setPen(const QColor &color);
    QFontMetrics fontMetrics();
    // 所有的绘制走自己写的绘制函数
    void drawText(const QRectF &r, const QString &text, const QTextOption &o = QTextOption());
    void drawImage(const QRect &r, const QImage &image);
    void drawPixmap(const QRect &r, const QPixmap &pm);
    void fillRect(const QRect &rect, const QBrush &b);
    QRect textRect(const QString& text);
    int textWidth(const QString& text);
    int charWidth(const QChar& ch);
    int textHeight();
    bool currentLineCanDrawText(const QString& text);
    int countOfThisLineCanDraw(const QString& text);
    QList<QRect> drawText(QString text);
    QRect drawTextInCurrentLine(const QString& text, bool adjustX = true, bool adjustY = true);
    void moveToNewLine();
    void visit(Header *node) override;
    void visit(Text *node) override;
    void visit(ItalicText *node) override;
    void visit(BoldText *node) override;
    void visit(ItalicBoldText *node) override;
    void visit(Image *node) override;
    void visit(Link *node) override;
    void drawCodeBlock(const String& code);
    void visit(CodeBlock *node) override;
    void visit(InlineCode *node) override;
    void visit(LatexBlock *node) override;
    void visit(InlineLatex *node) override;
    void visit(Paragraph *node) override;
    void visit(CheckboxList *node) override;
    void visit(CheckboxItem *node) override;
    void visit(UnorderedList *node) override;
    void visit(UnorderedListItem *node) override;
    void visit(OrderedList *node) override;
    void visit(OrderedListItem *node) override;
    void visit(Hr *node) override;
    void visit(QuoteBlock *node) override;
    void visit(Table *node) override;
    int realHeight() const;
    int realWidth() const;
    const QList<Element::Link*>& links();
    const QList<Element::Image*>& images();
    const QList<Element::CodeBlock*>& codes();
    [[nodiscard]] inline bool justCalculate() const;
    void setJustCalculate(bool flag);
    QFont curFont();
    void setPainter(QPainter* painter);
    QFont codeFont();
private:
    QPainter* m_painter;
    int m_curX;
    int m_curY;
    int m_lastMaxHeight;
    int m_lastMaxWidth;
    int m_maxWidth;
    QList<Element::Link*> m_links;
    QList<Element::Image*> m_images;
    QList<Element::CodeBlock*> m_codes;
    const QString& m_filePath;
    QMap<QString, QString> m_cacheLatexImage;
    bool m_justCalculate;
    QStack<QFont> m_fonts;
    QFont m_font;
    TexRender* m_texRender;
};

#endif //QTMARKDOWN_RENDER_H
