//
// Created by pikachu on 5/22/2021.
//

#ifndef QTMARKDOWN_RENDER_H
#define QTMARKDOWN_RENDER_H
#include <QDir>
#include <QFile>
#include <QFont>
#include <QFontMetrics>
#include <QMargins>
#include <QPainter>
#include <QProcess>
#include <QStack>

#include "Document.h"
#include "QtMarkdown_global.h"
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
    : public md::parser::MultipleVisitor<
          md::parser::Header, md::parser::Text, md::parser::ItalicText,
          md::parser::BoldText, md::parser::ItalicBoldText,
          md::parser::StrickoutText, md::parser::Image, md::parser::Link,
          md::parser::CodeBlock, md::parser::InlineCode, md::parser::Paragraph,
          md::parser::CheckboxList, md::parser::CheckboxItem,
          md::parser::UnorderedList, md::parser::UnorderedListItem,
          md::parser::OrderedList, md::parser::OrderedListItem,
          md::parser::LatexBlock, md::parser::InlineLatex, md::parser::Hr,
          md::parser::QuoteBlock, md::parser::Table, md::parser::Lf> {
 public:
  explicit Render(QString filePath, EditorDocument *doc,
                  RenderSetting = RenderSetting());
  ~Render();

  void setJustCalculate(bool flag);

  [[nodiscard]] int realHeight() const;

  [[nodiscard]] int realWidth() const;

  void reset(QPainter *painter);
  void visit(md::parser::Header *node) override;
  void visit(md::parser::Text *node) override;
  void visit(md::parser::ItalicText *node) override;
  void visit(md::parser::BoldText *node) override;
  void visit(md::parser::ItalicBoldText *node) override;
  void visit(md::parser::StrickoutText *node) override;
  void visit(md::parser::Image *node) override;
  void visit(md::parser::Link *node) override;
  void visit(md::parser::CodeBlock *node) override;
  void visit(md::parser::InlineCode *node) override;
  void visit(md::parser::LatexBlock *node) override;
  void visit(md::parser::InlineLatex *node) override;
  void visit(md::parser::Paragraph *node) override;
  void visit(md::parser::CheckboxList *node) override;
  void visit(md::parser::CheckboxItem *node) override;
  void visit(md::parser::UnorderedList *node) override;
  void visit(md::parser::UnorderedListItem *node) override;
  void visit(md::parser::OrderedList *node) override;
  void visit(md::parser::OrderedListItem *node) override;
  void visit(md::parser::Hr *node) override;
  void visit(md::parser::QuoteBlock *node) override;
  void visit(md::parser::Table *node) override;
  void visit(md::parser::Lf *node) override;
  void highlight(Cursor *cursor);

 private:
  RenderPrivate *d;
  friend class LineData;
};

#endif  // QTMARKDOWN_RENDER_H
