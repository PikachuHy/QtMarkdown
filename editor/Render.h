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
class TexRender;
class LineData;
class Cursor;
class EditorDocument;
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
class RenderPrivate;
class QTMARKDOWNSHARED_EXPORT Render
    : public MultipleVisitor<Header, Text, ItalicText, BoldText, ItalicBoldText,
                             Image, Link, CodeBlock, InlineCode, Paragraph,
                             CheckboxList, CheckboxItem, UnorderedList,
                             UnorderedListItem, OrderedList, OrderedListItem,
                             LatexBlock, InlineLatex, Hr, QuoteBlock, Table> {
public:
  explicit Render(QString filePath, EditorDocument *doc,
                  RenderSetting = RenderSetting());
  ~Render();

  void setJustCalculate(bool flag);

  [[nodiscard]] int realHeight() const;

  [[nodiscard]] int realWidth() const;

  void reset(QPainter *painter);
  void visit(Header *node) override;
  void visit(Text *node) override;
  void visit(ItalicText *node) override;
  void visit(BoldText *node) override;
  void visit(ItalicBoldText *node) override;
  void visit(Image *node) override;
  void visit(Link *node) override;
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
  void highlight(Cursor *cursor);

private:
  RenderPrivate *d;
  friend class LineData;
};

#endif // QTMARKDOWN_RENDER_H
