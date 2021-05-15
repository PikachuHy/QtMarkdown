//
// Created by pikachu on 2021/5/9.
//

#include <iostream>

#include "Parser.h"
#include "Token.h"
#include "gtest/gtest.h"


TEST(ParseUnorderedListTest, OnlyText) {
    Parser parser;
    auto nodes = parser.parse("- hhh");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::ul);
    auto ul = (UnorderedList*)node;
    ASSERT_EQ(ul->children().size(), 1);
    auto item = ul->children().at(0);
    ASSERT_EQ(item->type(), NodeType::ul_item);
    auto ul_item = (UnorderedListItem*)item;
    ASSERT_EQ(ul_item->children().size(), 1);
    auto text = ul_item->children().at(0);
    ASSERT_EQ(text->type(), NodeType::text);
}

TEST(ParseUnorderedListTest, OnlyLink) {
    Parser parser;
    auto nodes = parser.parse("- [666](http://www.666.com)");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::ul);
    auto ul = (UnorderedList*)node;
    ASSERT_EQ(ul->children().size(), 1);
    auto item = ul->children().at(0);
    ASSERT_EQ(item->type(), NodeType::ul_item);
    auto ul_item = (UnorderedListItem*)item;
    ASSERT_EQ(ul_item->children().size(), 1);
    auto link = ul_item->children().at(0);
    ASSERT_EQ(link->type(), NodeType::link);
}


TEST(ParseUnorderedListTest, TextAndLink) {
    Parser parser;
    auto nodes = parser.parse("- sdfg[666](http://www.666.com)sdfg");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::ul);
    auto ul = (UnorderedList*)node;
    ASSERT_EQ(ul->children().size(), 1);
    auto item = ul->children().at(0);
    ASSERT_EQ(item->type(), NodeType::ul_item);
    auto ul_item = (UnorderedListItem*)item;
    ASSERT_EQ(ul_item->children().size(), 3);
    {
        auto text = ul_item->children().at(0);
        ASSERT_EQ(text->type(), NodeType::text);
    }
    auto link = ul_item->children().at(1);
    ASSERT_EQ(link->type(), NodeType::link);
    {
        auto text = ul_item->children().at(2);
        ASSERT_EQ(text->type(), NodeType::text);
    }
}


TEST(ParseOrderedListTest, OnlyText) {
    Parser parser;
    auto nodes = parser.parse("1. hhh");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::ol);
    auto ol = (OrderedList*)node;
    ASSERT_EQ(ol->children().size(), 1);
    auto item = ol->children().at(0);
    ASSERT_EQ(item->type(), NodeType::ol_item);
    auto ul_item = (OrderedListItem*)item;
    ASSERT_EQ(ul_item->children().size(), 1);
    auto text = ul_item->children().at(0);
    ASSERT_EQ(text->type(), NodeType::text);
}

TEST(ParseOrderedListTest, OnlyLink) {
    Parser parser;
    auto nodes = parser.parse("1. [666](http://www.666.com)");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::ol);
    auto ol = (OrderedList*)node;
    ASSERT_EQ(ol->children().size(), 1);
    auto item = ol->children().at(0);
    ASSERT_EQ(item->type(), NodeType::ol_item);
    auto ol_item = (OrderedListItem*)item;
    ASSERT_EQ(ol_item->children().size(), 1);
    auto link = ol_item->children().at(0);
    ASSERT_EQ(link->type(), NodeType::link);
}


TEST(ParseOrderedListTest, TextAndLink) {
    Parser parser;
    auto nodes = parser.parse("1. sdfg[666](http://www.666.com)sdfg");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::ol);
    auto ul = (OrderedList*)node;
    ASSERT_EQ(ul->children().size(), 1);
    auto item = ul->children().at(0);
    ASSERT_EQ(item->type(), NodeType::ol_item);
    auto ol_item = (OrderedListItem*)item;
    ASSERT_EQ(ol_item->children().size(), 3);
    {
        auto text = ol_item->children().at(0);
        ASSERT_EQ(text->type(), NodeType::text);
    }
    auto link = ol_item->children().at(1);
    ASSERT_EQ(link->type(), NodeType::link);
    {
        auto text = ol_item->children().at(2);
        ASSERT_EQ(text->type(), NodeType::text);
    }
}


TEST(ParseCheckboxListTest, OnlyText) {
    Parser parser;
    auto nodes = parser.parse("- [ ] hhh");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::checkbox);
    auto ol = (OrderedList*)node;
    ASSERT_EQ(ol->children().size(), 1);
    auto item = ol->children().at(0);
    ASSERT_EQ(item->type(), NodeType::checkbox_item);
    auto ul_item = (OrderedListItem*)item;
    ASSERT_EQ(ul_item->children().size(), 1);
    auto text = ul_item->children().at(0);
    ASSERT_EQ(text->type(), NodeType::text);
}

TEST(ParseCheckboxListTest, OnlyLink) {
    Parser parser;
    auto nodes = parser.parse("- [ ] [666](http://www.666.com)");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::checkbox);
    auto ol = (OrderedList*)node;
    ASSERT_EQ(ol->children().size(), 1);
    auto item = ol->children().at(0);
    ASSERT_EQ(item->type(), NodeType::checkbox_item);
    auto ol_item = (OrderedListItem*)item;
    ASSERT_EQ(ol_item->children().size(), 1);
    auto link = ol_item->children().at(0);
    ASSERT_EQ(link->type(), NodeType::link);
}


TEST(ParseCheckboxListTest, TextAndLink) {
    Parser parser;
    auto nodes = parser.parse("- [ ] sdfg[666](http://www.666.com)sdfg");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::checkbox);
    auto ul = (OrderedList*)node;
    ASSERT_EQ(ul->children().size(), 1);
    auto item = ul->children().at(0);
    ASSERT_EQ(item->type(), NodeType::checkbox_item);
    auto ol_item = (OrderedListItem*)item;
    ASSERT_EQ(ol_item->children().size(), 3);
    {
        auto text = ol_item->children().at(0);
        ASSERT_EQ(text->type(), NodeType::text);
    }
    auto link = ol_item->children().at(1);
    ASSERT_EQ(link->type(), NodeType::link);
    {
        auto text = ol_item->children().at(2);
        ASSERT_EQ(text->type(), NodeType::text);
    }
}


TEST(ParseImageTest, Only) {
    Parser parser;
    auto nodes = parser.parse("![666](http://www.666.com)");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::image);
}


TEST(ParseLinkTest, Only) {
    Parser parser;
    auto nodes = parser.parse("[666](http://www.666.com)");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::link);
}
TEST(ParseInlineCodeTest, Only) {
    Parser parser;
    auto nodes = parser.parse("`#include`");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::inline_code);
}
TEST(ParseInlineLatexTest, Only) {
    Parser parser;
    auto nodes = parser.parse("$a^2$");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::inline_latex);
}
TEST(ParseItalicTest, Only) {
    Parser parser;
    auto nodes = parser.parse("*666*");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::italic);
}
TEST(ParseBoldTest, Only) {
    Parser parser;
    auto nodes = parser.parse("**666**");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::bold);
}

TEST(ParseItalicBoldBoldTest, Only) {
    Parser parser;
    auto nodes = parser.parse("***666***");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::italic_bold);
}

TEST(ParseStrickoutTest, Only) {
    Parser parser;
    auto nodes = parser.parse("~~666~~");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::strickout);
}

TEST(ParseLinkTest, Middle) {
    Parser parser;
    auto nodes = parser.parse("before[666](http://www.666.com)after");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 3);
    auto before = p->children().at(0);
    ASSERT_EQ(before->type(), NodeType::text);
    auto link = p->children().at(1);
    ASSERT_EQ(link->type(), NodeType::link);
    auto after = p->children().at(2);
    ASSERT_EQ(after->type(), NodeType::text);
}
TEST(ParseInlineCodeTest, Middle) {
    Parser parser;
    auto nodes = parser.parse("before`#include`after");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 3);
    auto before = p->children().at(0);
    ASSERT_EQ(before->type(), NodeType::text);
    auto ic = p->children().at(1);
    ASSERT_EQ(ic->type(), NodeType::inline_code);
    auto after = p->children().at(2);
    ASSERT_EQ(after->type(), NodeType::text);
}
TEST(ParseInlineLatexTest, Middle) {
    Parser parser;
    auto nodes = parser.parse("before$a^2$after");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 3);
    auto before = p->children().at(0);
    ASSERT_EQ(before->type(), NodeType::text);
    auto il = p->children().at(1);
    ASSERT_EQ(il->type(), NodeType::inline_latex);
    auto after = p->children().at(2);
    ASSERT_EQ(after->type(), NodeType::text);
}
TEST(ParseItalicTest, Middle) {
    Parser parser;
    auto nodes = parser.parse("before*666*after");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 3);
    auto before = p->children().at(0);
    ASSERT_EQ(before->type(), NodeType::text);
    auto i = p->children().at(1);
    ASSERT_EQ(i->type(), NodeType::italic);
    auto after = p->children().at(2);
    ASSERT_EQ(after->type(), NodeType::text);
}
TEST(ParseBoldTest, Middle) {
    Parser parser;
    auto nodes = parser.parse("before**666**after");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 3);
    auto before = p->children().at(0);
    ASSERT_EQ(before->type(), NodeType::text);
    auto b = p->children().at(1);
    ASSERT_EQ(b->type(), NodeType::bold);
    auto after = p->children().at(2);
    ASSERT_EQ(after->type(), NodeType::text);
}

TEST(ParseItalicBoldBoldTest, Middle) {
    Parser parser;
    auto nodes = parser.parse("before***666***after");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 3);
    auto before = p->children().at(0);
    ASSERT_EQ(before->type(), NodeType::text);
    auto it = p->children().at(1);
    ASSERT_EQ(it->type(), NodeType::italic_bold);
    auto after = p->children().at(2);
    ASSERT_EQ(after->type(), NodeType::text);
}


TEST(ParseStrickoutTest, Middle) {
    Parser parser;
    auto nodes = parser.parse("before~~666~~after");
    ASSERT_EQ(nodes.size(), 1);
    auto node = nodes[0];
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 3);
    auto before = p->children().at(0);
    ASSERT_EQ(before->type(), NodeType::text);
    auto strickout = p->children().at(1);
    ASSERT_EQ(strickout->type(), NodeType::strickout);
    auto after = p->children().at(2);
    ASSERT_EQ(after->type(), NodeType::text);
}

