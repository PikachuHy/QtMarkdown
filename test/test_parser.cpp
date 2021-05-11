//
// Created by pikachu on 2021/5/9.
//

#include <iostream>

#include "Parser.h"
#include "Token.h"
#include "gtest/gtest.h"

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