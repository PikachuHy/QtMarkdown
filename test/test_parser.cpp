//
// Created by pikachu on 2021/5/9.
//

#include <iostream>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "parser/Document.h"
#include "parser/Parser.h"
#include "parser/Token.h"
using namespace md::parser;

TEST_CASE("ParseUnorderedListTest,  OnlyText") {
  auto nodes = Parser::parse("- hhh");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::ul);
  auto ul = (UnorderedList*)node;
  CHECK(ul->children().size() == 1);
  auto item = ul->children().at(0);
  CHECK(item->type() == NodeType::ul_item);
  auto ul_item = (UnorderedListItem*)item;
  CHECK(ul_item->children().size() == 1);
  auto text = ul_item->children().at(0);
  CHECK(text->type() == NodeType::text);
}

TEST_CASE("ParseUnorderedListTest,  OnlyLink") {
  auto nodes = Parser::parse("- [666](http://www.666.com)");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::ul);
  auto ul = (UnorderedList*)node;
  CHECK(ul->children().size() == 1);
  auto item = ul->children().at(0);
  CHECK(item->type() == NodeType::ul_item);
  auto ul_item = (UnorderedListItem*)item;
  CHECK(ul_item->children().size() == 1);
  auto link = ul_item->children().at(0);
  CHECK(link->type() == NodeType::link);
}

TEST_CASE("ParseUnorderedListTest,  TextAndLink") {
  auto nodes = Parser::parse("- sdfg[666](http://www.666.com)sdfg");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::ul);
  auto ul = (UnorderedList*)node;
  CHECK(ul->children().size() == 1);
  auto item = ul->children().at(0);
  CHECK(item->type() == NodeType::ul_item);
  auto ul_item = (UnorderedListItem*)item;
  CHECK(ul_item->children().size() == 3);
  {
    auto text = ul_item->children().at(0);
    CHECK(text->type() == NodeType::text);
  }
  auto link = ul_item->children().at(1);
  CHECK(link->type() == NodeType::link);
  {
    auto text = ul_item->children().at(2);
    CHECK(text->type() == NodeType::text);
  }
}

TEST_CASE("ParseOrderedListTest,  OnlyText") {
  auto nodes = Parser::parse("1. hhh");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::ol);
  auto ol = (OrderedList*)node;
  CHECK(ol->children().size() == 1);
  auto item = ol->children().at(0);
  CHECK(item->type() == NodeType::ol_item);
  auto ul_item = (OrderedListItem*)item;
  CHECK(ul_item->children().size() == 1);
  auto text = ul_item->children().at(0);
  CHECK(text->type() == NodeType::text);
}

TEST_CASE("ParseOrderedListTest,  OnlyLink") {
  Parser parser;
  auto nodes = parser.parse("1. [666](http://www.666.com)");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::ol);
  auto ol = (OrderedList*)node;
  CHECK(ol->children().size() == 1);
  auto item = ol->children().at(0);
  CHECK(item->type() == NodeType::ol_item);
  auto ol_item = (OrderedListItem*)item;
  CHECK(ol_item->children().size() == 1);
  auto link = ol_item->children().at(0);
  CHECK(link->type() == NodeType::link);
}

TEST_CASE("ParseOrderedListTest,  TextAndLink") {
  Parser parser;
  auto nodes = parser.parse("1. sdfg[666](http://www.666.com)sdfg");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::ol);
  auto ul = (OrderedList*)node;
  CHECK(ul->children().size() == 1);
  auto item = ul->children().at(0);
  CHECK(item->type() == NodeType::ol_item);
  auto ol_item = (OrderedListItem*)item;
  CHECK(ol_item->children().size() == 3);
  {
    auto text = ol_item->children().at(0);
    CHECK(text->type() == NodeType::text);
  }
  auto link = ol_item->children().at(1);
  CHECK(link->type() == NodeType::link);
  {
    auto text = ol_item->children().at(2);
    CHECK(text->type() == NodeType::text);
  }
}

TEST_CASE("ParseCheckboxListTest,  OnlyText") {
  Parser parser;
  auto nodes = parser.parse("- [ ] hhh");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::checkbox);
  auto ol = (OrderedList*)node;
  CHECK(ol->children().size() == 1);
  auto item = ol->children().at(0);
  CHECK(item->type() == NodeType::checkbox_item);
  auto ul_item = (OrderedListItem*)item;
  CHECK(ul_item->children().size() == 1);
  auto text = ul_item->children().at(0);
  CHECK(text->type() == NodeType::text);
}

TEST_CASE("ParseCheckboxListTest,  OnlyLink") {
  Parser parser;
  auto nodes = parser.parse("- [ ] [666](http://www.666.com)");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::checkbox);
  auto ol = (OrderedList*)node;
  CHECK(ol->children().size() == 1);
  auto item = ol->children().at(0);
  CHECK(item->type() == NodeType::checkbox_item);
  auto ol_item = (OrderedListItem*)item;
  CHECK(ol_item->children().size() == 1);
  auto link = ol_item->children().at(0);
  CHECK(link->type() == NodeType::link);
}

TEST_CASE("ParseCheckboxListTest,  TextAndLink") {
  Parser parser;
  auto nodes = parser.parse("- [ ] sdfg[666](http://www.666.com)sdfg");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::checkbox);
  auto ul = (OrderedList*)node;
  CHECK(ul->children().size() == 1);
  auto item = ul->children().at(0);
  CHECK(item->type() == NodeType::checkbox_item);
  auto ol_item = (OrderedListItem*)item;
  CHECK(ol_item->children().size() == 3);
  {
    auto text = ol_item->children().at(0);
    CHECK(text->type() == NodeType::text);
  }
  auto link = ol_item->children().at(1);
  CHECK(link->type() == NodeType::link);
  {
    auto text = ol_item->children().at(2);
    CHECK(text->type() == NodeType::text);
  }
}

TEST_CASE("ParseImageTest,  Only") {
  Parser parser;
  auto nodes = parser.parse("![666](http://www.666.com)");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 1);
  auto link = p->children().at(0);
  CHECK(link->type() == NodeType::image);
}

TEST_CASE("ParseLinkTest,  Only") {
  Parser parser;
  auto nodes = parser.parse("[666](http://www.666.com)");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 1);
  auto link = p->children().at(0);
  CHECK(link->type() == NodeType::link);
}
TEST_CASE("ParseInlineCodeTest,  Only") {
  Parser parser;
  auto nodes = parser.parse("`#include`");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 1);
  auto link = p->children().at(0);
  CHECK(link->type() == NodeType::inline_code);
}
TEST_CASE("ParseInlineLatexTest,  Only") {
  Parser parser;
  auto nodes = parser.parse("$a^2$");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 1);
  auto link = p->children().at(0);
  CHECK(link->type() == NodeType::inline_latex);
}
TEST_CASE("ParseItalicTest,  Only") {
  Parser parser;
  auto nodes = parser.parse("*666*");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 1);
  auto link = p->children().at(0);
  CHECK(link->type() == NodeType::italic);
}
TEST_CASE("ParseBoldTest,  Only") {
  Parser parser;
  auto nodes = parser.parse("**666**");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 1);
  auto link = p->children().at(0);
  CHECK(link->type() == NodeType::bold);
}

TEST_CASE("ParseItalicBoldBoldTest,  Only") {
  Parser parser;
  auto nodes = parser.parse("***666***");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 1);
  auto link = p->children().at(0);
  CHECK(link->type() == NodeType::italic_bold);
}

TEST_CASE("ParseStrickoutTest,  Only") {
  Parser parser;
  auto nodes = parser.parse("~~666~~");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 1);
  auto link = p->children().at(0);
  CHECK(link->type() == NodeType::strickout);
}

TEST_CASE("ParseLinkTest,  Middle") {
  Parser parser;
  auto nodes = parser.parse("before[666](http://www.666.com)after");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 3);
  auto before = p->children().at(0);
  CHECK(before->type() == NodeType::text);
  auto link = p->children().at(1);
  CHECK(link->type() == NodeType::link);
  auto after = p->children().at(2);
  CHECK(after->type() == NodeType::text);
}
TEST_CASE("ParseInlineCodeTest,  Middle") {
  Parser parser;
  auto nodes = parser.parse("before`#include`after");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 3);
  auto before = p->children().at(0);
  CHECK(before->type() == NodeType::text);
  auto ic = p->children().at(1);
  CHECK(ic->type() == NodeType::inline_code);
  auto after = p->children().at(2);
  CHECK(after->type() == NodeType::text);
}
TEST_CASE("ParseInlineLatexTest,  Middle") {
  Parser parser;
  auto nodes = parser.parse("before$a^2$after");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 3);
  auto before = p->children().at(0);
  CHECK(before->type() == NodeType::text);
  auto il = p->children().at(1);
  CHECK(il->type() == NodeType::inline_latex);
  auto after = p->children().at(2);
  CHECK(after->type() == NodeType::text);
}
TEST_CASE("ParseItalicTest,  Middle") {
  Parser parser;
  auto nodes = parser.parse("before*666*after");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 3);
  auto before = p->children().at(0);
  CHECK(before->type() == NodeType::text);
  auto i = p->children().at(1);
  CHECK(i->type() == NodeType::italic);
  auto after = p->children().at(2);
  CHECK(after->type() == NodeType::text);
}
TEST_CASE("ParseBoldTest,  Middle") {
  Parser parser;
  auto nodes = parser.parse("before**666**after");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 3);
  auto before = p->children().at(0);
  CHECK(before->type() == NodeType::text);
  auto b = p->children().at(1);
  CHECK(b->type() == NodeType::bold);
  auto after = p->children().at(2);
  CHECK(after->type() == NodeType::text);
}

TEST_CASE("ParseItalicBoldBoldTest,  Middle") {
  Parser parser;
  auto nodes = parser.parse("before***666***after");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 3);
  auto before = p->children().at(0);
  CHECK(before->type() == NodeType::text);
  auto it = p->children().at(1);
  CHECK(it->type() == NodeType::italic_bold);
  auto after = p->children().at(2);
  CHECK(after->type() == NodeType::text);
}

TEST_CASE("ParseStrickoutTest,  Middle") {
  Parser parser;
  auto nodes = parser.parse("before~~666~~after");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto p = (Paragraph*)node;
  CHECK(p->children().size() == 3);
  auto before = p->children().at(0);
  CHECK(before->type() == NodeType::text);
  auto strickout = p->children().at(1);
  CHECK(strickout->type() == NodeType::strickout);
  auto after = p->children().at(2);
  CHECK(after->type() == NodeType::text);
}

TEST_CASE("ParseParagraphTest,  OnlyTextOneSharp") {
  auto nodes = Parser::parse("#");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto paragraphNode = (Paragraph*)node;
  CHECK(paragraphNode->size() == 1);
  auto text = paragraphNode->childAt(0);
  CHECK(text->type() == NodeType::text);
}

TEST_CASE("ParseParagraphTest,  OnlyTextThreeBackquote") {
  auto nodes = Parser::parse("```");
  CHECK(nodes->size() == 1);
  auto node = nodes->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto paragraphNode = (Paragraph*)node;
  CHECK(paragraphNode->size() == 1);
  auto text = paragraphNode->childAt(0);
  CHECK(text->type() == NodeType::text);
}
