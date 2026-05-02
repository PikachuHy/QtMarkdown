//
// Created by pikachu on 2021/2/1.
//

#ifndef MD_VISITOR_H
#define MD_VISITOR_H
#include "QtMarkdown_global.h"
namespace md::parser {

// Forward declarations for all concrete node types
class Header;
class Paragraph;
class Text;
class Image;
class Link;
class CodeBlock;
class InlineCode;
class LatexBlock;
class InlineLatex;
class CheckboxList;
class CheckboxItem;
class UnorderedList;
class UnorderedListItem;
class OrderedList;
class OrderedListItem;
class Hr;
class QuoteBlock;
class ItalicText;
class BoldText;
class ItalicBoldText;
class StrickoutText;
class Table;
class Lf;

struct NodeVisitor {
  virtual ~NodeVisitor() = default;

  virtual void visit(Header*) {}
  virtual void visit(Paragraph*) {}
  virtual void visit(Text*) {}
  virtual void visit(Image*) {}
  virtual void visit(Link*) {}
  virtual void visit(CodeBlock*) {}
  virtual void visit(InlineCode*) {}
  virtual void visit(LatexBlock*) {}
  virtual void visit(InlineLatex*) {}
  virtual void visit(CheckboxList*) {}
  virtual void visit(CheckboxItem*) {}
  virtual void visit(UnorderedList*) {}
  virtual void visit(UnorderedListItem*) {}
  virtual void visit(OrderedList*) {}
  virtual void visit(OrderedListItem*) {}
  virtual void visit(Hr*) {}
  virtual void visit(QuoteBlock*) {}
  virtual void visit(ItalicText*) {}
  virtual void visit(BoldText*) {}
  virtual void visit(ItalicBoldText*) {}
  virtual void visit(StrickoutText*) {}
  virtual void visit(Table*) {}
  virtual void visit(Lf*) {}
};

}  // namespace md::parser
#endif  // MD_VISITOR_H
