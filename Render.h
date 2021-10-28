//
// Created by pikachu on 5/22/2021.
//

#ifndef QTMARKDOWN_RENDER_H
#define QTMARKDOWN_RENDER_H
#include "Document.h"
#include "QtMarkdown_global.h"
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFontMetrics>
#include <QMargins>
#include <QPainter>
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
} // namespace Element
class TexRender;
class LineData;
class Cursor;
struct RenderSetting {
  bool highlightCurrentLine = false;
  int lineSpacing = 10;
  int maxWidth = 800;
  int latexFontSize = 20;
  QMargins docMargin = QMargins(20, 20, 20, 20);
  QMargins codeMargin = QMargins(10, 20, 20, 10);
  QMargins listMargin = QMargins(15, 20, 20, 10);
  QMargins checkboxMargin = QMargins(15, 20, 20, 10);
  QMargins quoteMargin = QMargins(10, 20, 20, 10);
  [[nodiscard]] int contentMaxWidth() const {
    return maxWidth - docMargin.left() - docMargin.right();
  }
};
class QTMARKDOWNSHARED_EXPORT Render
    : public MultipleVisitor<Header, Text, ItalicText, BoldText, ItalicBoldText,
                             Image, Link, CodeBlock, InlineCode, Paragraph,
                             CheckboxList, CheckboxItem, UnorderedList,
                             UnorderedListItem, OrderedList, OrderedListItem,
                             LatexBlock, InlineLatex, Hr, QuoteBlock, Table> {
public:
  explicit Render(QString filePath, RenderSetting = RenderSetting());
  ~Render();
  void reset(QPainter *painter);
  // 隔离QPainter
  void save();
  void restore();
  void setFont(const QFont &font);
  void setPen(const QColor &color);
  QFontMetrics fontMetrics();
  // 所有的绘制走自己写的绘制函数
  void drawText(const QRectF &r, const QString &text,
                const QTextOption &o = QTextOption());
  void drawImage(const QRect &r, const QImage &image);
  void drawPixmap(const QRect &r, const QPixmap &pm);
  void drawRect(const QRect &r, QColor fb = Qt::black);
  void fillRect(const QRect &rect, const QBrush &b);
  QRect textRect(const QString &text);
  int textWidth(const QString &text);
  int charWidth(const QChar &ch);
  int textHeight();
  bool currentLineCanDrawText(const QString &text);
  int countOfThisLineCanDraw(const QString &text);
  QList<QRect> drawText(QString text);
  QRect drawTextInCurrentLine(const QString &text, bool adjustX = true,
                              bool adjustY = true);
  void moveToNewLine();
  void visit(Header *node) override;
  void visit(Text *node) override;
  void visit(ItalicText *node) override;
  void visit(BoldText *node) override;
  void visit(ItalicBoldText *node) override;
  void visit(Image *node) override;
  void visit(Link *node) override;
  void drawCodeBlock(const String &code);
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
  const QList<Element::Link *> &links();
  const QList<Element::Image *> &images();
  const QList<Element::CodeBlock *> &codes();
  [[nodiscard]] inline bool justCalculate() const;
  void setJustCalculate(bool flag);
  QFont curFont();
  void setPainter(QPainter *painter);
  QFont codeFont();
  void highlight(Cursor *cursor);
  void fixCursorPos(Cursor *cursor);
  void updateCursor(Cursor *cursor);
  void moveCursorLeft(Cursor *cursor);
  void moveCursorRight(Cursor *cursor);
  void moveCursorUp(Cursor *cursor);
  void moveCursorDown(Cursor *cursor);

private:
  QPainter *m_painter;
  RenderSetting m_setting;
  int m_curX;
  int m_curY;
  int m_lastMaxHeight;
  int m_lastMaxWidth;
  QList<Element::Link *> m_links;
  QList<Element::Image *> m_images;
  QList<Element::CodeBlock *> m_codes;
  QString m_filePath;
  QMap<QString, QString> m_cacheLatexImage;
  bool m_justCalculate;
  QStack<QFont> m_fonts;
  QFont m_font;
  TexRender *m_texRender;
  QRect m_lastRect;
  QList<LineData *> m_lineData;
  friend class LineData;
};

#endif // QTMARKDOWN_RENDER_H
