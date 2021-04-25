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
#include <QPainter>
#include <QScrollArea>

struct DefaultEditorVisitor: MultipleVisitor<Header,
        Text, ItalicText, BoldText, ItalicBoldText,
        Image, Link, CodeBlock, InlineCode, Paragraph,
        UnorderedList, OrderedList,
        Hr, QuoteBlock, Table> {
    explicit DefaultEditorVisitor(QPainter& painter, int w, int rightMargin):
            m_painter(painter), m_maxWidth(w - rightMargin) {
        qDebug() << "width: " << m_maxWidth;
        m_curX = 0;
        m_curY = 0;
        m_lastMaxHeight = 0;
        m_lastMaxWidth = w;
    }
    QRect textRect(const QString& text) {
        QFontMetrics metrics = m_painter.fontMetrics();
        QRect textBoundingRect = metrics.boundingRect(QRect(m_curX, m_curY, m_maxWidth, 0), Qt::TextWordWrap, text);
        return textBoundingRect;
    }
    int textWidth(const QString& text) {
        QFontMetrics metrics = m_painter.fontMetrics();
        int w = metrics.horizontalAdvance(text);
        return w;
    }
    int charWidth(const QChar& ch) {
        QFontMetrics metrics = m_painter.fontMetrics();
        int w = metrics.horizontalAdvance(ch);
        return w;
    }
    bool currentLineCanDrawText(const QString& text) {
        auto needWidth = textWidth(text);
        // qDebug() << "need" << needWidth << text;
        if (m_curX + needWidth < m_maxWidth) {
            return true;
        } else {
            return false;
        }
    }
    void drawText(const QString& text) {
        if (text == "\r") return;
//         qDebug() << "draw" << text;
        if (text.isEmpty()) return;
        if (currentLineCanDrawText(text)) {
            drawTextInCurrentLine(text);
        } else {
            auto ch_w = charWidth(text.at(0));
            if (m_curX + ch_w <= m_maxWidth) {
                // 计算这一行可以画多少个字符
                int left_w = m_maxWidth - m_curX;
                int may_ch_count = left_w / ch_w - 1;
                if (currentLineCanDrawText(text.left(may_ch_count + 1))) {
                    while (currentLineCanDrawText(text.left(may_ch_count + 1))) {
                        may_ch_count++;
                    }
                } else {
                    while (!currentLineCanDrawText(text.left(may_ch_count))) {
                        may_ch_count--;
                    }
                }
                drawTextInCurrentLine(text.left(may_ch_count));
                m_curY += m_lastMaxHeight;
                drawTextInNewLine(text.right(text.size() - may_ch_count));
            } else {
                // 如果一个字符都画不了，直接画矩形
                m_curX = 0;
                m_curY += m_lastMaxHeight;
                drawTextInNewLine(text);
            }
        }
    }
    void drawTextInCurrentLine(const QString& text) {
//        qDebug() << "cur";
        auto rect = textRect(text);
//        qDebug() << rect << text;
        m_painter.drawText(rect, text);
        m_curX += rect.width();
        m_lastMaxHeight = qMax(m_lastMaxHeight, rect.height());
    }
    void drawTextInNewLine(const QString& text) {
//        qDebug() << "new";
        m_curX = 0;
        auto rect = textRect(text);
//        qDebug() << rect << text;
        m_painter.drawText(rect, text);
        m_curY += rect.height();
        m_lastMaxHeight = 0;
    }
    void moveToNewLine() {
        m_curY += m_lastMaxHeight;
        m_curX = 0;
        m_lastMaxHeight = 0;
    }
    void visit(Header *node) override {
        moveToNewLine();
        QString hn = "h" + String::number(node->level());
        drawText(hn);
        m_curX += 10;
        for(auto it: node->children()) {
            it->accept(this);
        }
    }
    void visit(Text *node) override {
        drawText(node->str());
    }
    void visit(ItalicText *node) override {
        drawText(node->str());
    }
    void visit(BoldText *node) override {
        drawText(node->str());
    }
    void visit(ItalicBoldText *node) override {
        drawText(node->str());
    }
    void visit(Image *node) override {
        moveToNewLine();
        QString imgPath = node->src()->str();
        QFile file(imgPath);
        if (file.exists()) {
            QImage image(imgPath);
            int imageMaxWidth = qMin(1080, m_maxWidth);
            if (image.width() > imageMaxWidth) {
                image = image.scaledToWidth(imageMaxWidth);
            }
            QRect rect(QPoint(m_curX, m_curY), image.size());
//            qDebug() << "image rect" << rect;
            m_painter.drawImage(rect, image);
            m_lastMaxHeight = rect.height();
        } else {
            qWarning() << "image not exist." << imgPath;
        }
    }
    void visit(Link *node) override {
    }
    void visit(CodeBlock *node) override {
        moveToNewLine();
        drawText(node->code()->str());
    }
    void visit(InlineCode *node) override {
    }
    void visit(Paragraph *node) override {
        moveToNewLine();
        for(auto it: node->children()) {
            it->accept(this);
        }
    }
    void visit(UnorderedList *node) override {
        for (const auto &item : node->children()) {
            moveToNewLine();
            drawText("● ");
            item->accept(this);
        }

    }
    void visit(OrderedList *node) override {
        int i = 0;
        for (const auto &item : node->children()) {
            i++;
            moveToNewLine();
            QString numStr = QString("%1. ").arg(i);
            drawText(numStr);
            item->accept(this);
        }
    }
    void visit(Hr *node) override {
    }
    void visit(QuoteBlock *node) override {
    }
    void visit(Table *node) override {
    }
    int realHeight() const {
        return m_curY + m_lastMaxHeight;
    }
    int realWidth() const {
        return m_lastMaxWidth;
    }
private:
    QPainter& m_painter;
    int m_curX;
    int m_curY;
    int m_lastMaxHeight;
    int m_lastMaxWidth;
    int m_maxWidth;
};
Editor::Editor(QWidget *parent) : QWidget(parent), m_firstDraw(true) {
    m_rightMargin = 0;
}

void Editor::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e);
    QPainter painter(this);
    if (m_firstDraw) {
        painter.setRenderHint(QPainter::Antialiasing);
        QFile mdFile("../test.md");
        if (!mdFile.exists()) {
            qDebug() << "file not exist:" << mdFile.fileName();
            return;
        }
        mdFile.open(QIODevice::ReadOnly);
        auto mdText = mdFile.readAll();
        mdFile.close();
//    qDebug().noquote().nospace() << mdText;
        Document doc(mdText);
        int w = 600;
        if (parentWidget()) {
            w = parentWidget()->width();
        }
        qDebug() << "w" << w;
        DefaultEditorVisitor visitor(painter, w, m_rightMargin);
        doc.accept(&visitor);
        int h = visitor.realHeight();
        if (h < 0) {
            h = 600;
        }
        w = qMax(w, visitor.realWidth());
        qDebug() << "set size:" << w << h;
        setFixedSize(w, h + 20);
        {
            m_buffer = QImage(w + m_rightMargin * 2, h, QImage::Format_RGB32);
            m_buffer.fill(Qt::white);
            QPainter p(&m_buffer);
            p.setRenderHint(QPainter::Antialiasing);
            DefaultEditorVisitor _visitor(p, w, m_rightMargin);
            doc.accept(&_visitor);
        }
        m_firstDraw = false;
    } else {
        auto _size = m_buffer.size();
//        qDebug() << "image:" << _size;
//        painter.drawImage(QRect(0, 0, _size.width() - m_rightMargin, _size.height()), m_buffer);
        painter.drawImage(0, 0, m_buffer);
    }
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QScrollArea w;
    w.setWidgetResizable(true);
    auto e = new Editor(&w);
    w.setWidget(e);
    w.resize(600, 400);
    w.show();
    return QApplication::exec();
}
