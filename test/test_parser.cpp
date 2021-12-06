//
// Created by pikachu on 2021/5/9.
//

#include <iostream>
#include <catch2/catch_test_macros.hpp>
#include "parser/Document.h"
#include "parser/Parser.h"
#include "parser/Token.h"
using namespace md::parser;
#include "common.h"
TEST_CASE( "parse paragraph", "[p]" ) {
  SECTION( "only text one sharp" ) {
    auto nodes = Parser::parse("#");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto paragraphNode = (Paragraph*)node;
    ASSERT_EQ(paragraphNode->size(), 1);
    auto text = paragraphNode->childAt(0);
    ASSERT_EQ(text->type(), NodeType::text);
  }
  SECTION( "only text three backquote" ) {
    auto nodes = Parser::parse("```");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto paragraphNode = (Paragraph*)node;
    ASSERT_EQ(paragraphNode->size(), 1);
    auto text = paragraphNode->childAt(0);
    ASSERT_EQ(text->type(), NodeType::text);
  }
}
TEST_CASE( "parse unordered list", "[ul]" ) {
  SECTION( "only text" ) {
    auto nodes = Parser::parse("- hhh");
    REQUIRE(nodes->size() == 1);
    auto node = nodes->childAt(0);
    REQUIRE(node->type() == NodeType::ul);
    auto ul = (UnorderedList*)node;
    REQUIRE(ul->size() == 1);
    auto item = ul->childAt(0);
    REQUIRE(item->type() == NodeType::ul_item);
    auto ul_item = (UnorderedListItem*)item;
    REQUIRE(ul_item->size() == 1);
    auto text = ul_item->children().at(0);
    REQUIRE(text->type() == NodeType::text);
  }
  SECTION( "only link" ) {
    auto nodes = Parser::parse("- [666](http://www.666.com)");
    REQUIRE(nodes->size() == 1);
    auto node = nodes->childAt(0);
    REQUIRE(node->type() == NodeType::ul);
    auto ul = (UnorderedList*)node;
    REQUIRE(ul->size() == 1);
    auto item = ul->children().at(0);
    REQUIRE(item->type() == NodeType::ul_item);
    auto ul_item = (UnorderedListItem*)item;
    REQUIRE(ul_item->size() == 1);
    auto link = ul_item->children().at(0);
    REQUIRE(link->type() == NodeType::link);
  }
  SECTION( "text and link" ) {
    auto nodes = Parser::parse("- sdfg[666](http://www.666.com)sdfg");
    REQUIRE(nodes->size() == 1);
    auto node = nodes->childAt(0);
    REQUIRE(node->type() == NodeType::ul);
    auto ul = (UnorderedList*)node;
    REQUIRE(ul->size() == 1);
    auto item = ul->children().at(0);
    REQUIRE(item->type() == NodeType::ul_item);
    auto ul_item = (UnorderedListItem*)item;
    REQUIRE(ul_item->size() == 3);
    {
      auto text = ul_item->children().at(0);
      REQUIRE(text->type() == NodeType::text);
    }
    auto link = ul_item->children().at(1);
    REQUIRE(link->type() == NodeType::link);
    {
      auto text = ul_item->children().at(2);
      REQUIRE(text->type() == NodeType::text);
    }
  }
}
TEST_CASE( "parse ordered list", "[ol]" ) {
  SECTION( "only text" ) {
    auto nodes = Parser::parse("1. hhh");
    REQUIRE(nodes->size() == 1);
    auto node = nodes->childAt(0);
    REQUIRE(node->type() == NodeType::ol);
    auto ol = (OrderedList*)node;
    REQUIRE(ol->size() == 1);
    auto item = ol->children().at(0);
    REQUIRE(item->type() == NodeType::ol_item);
    auto ul_item = (OrderedListItem*)item;
    REQUIRE(ul_item->size() == 1);
    auto text = ul_item->children().at(0);
    REQUIRE(text->type() == NodeType::text);
  }
  SECTION( "only link" ) {
    auto nodes = Parser::parse("1. [666](http://www.666.com)");
    REQUIRE(nodes->size() == 1);
    auto node = nodes->childAt(0);
    REQUIRE(node->type() == NodeType::ol);
    auto ol = (OrderedList*)node;
    REQUIRE(ol->size() == 1);
    auto item = ol->children().at(0);
    REQUIRE(item->type() == NodeType::ol_item);
    auto ol_item = (OrderedListItem*)item;
    REQUIRE(ol_item->size() == 1);
    auto link = ol_item->childAt(0);
    REQUIRE(link->type() == NodeType::link);
  }
  SECTION( "text and link" ) {
    auto nodes = Parser::parse("1. sdfg[666](http://www.666.com)sdfg");
    REQUIRE(nodes->size() == 1);
    auto node = nodes->childAt(0);
    REQUIRE(node->type() == NodeType::ol);
    auto ul = (OrderedList*)node;
    REQUIRE(ul->size() == 1);
    auto item = ul->children().at(0);
    REQUIRE(item->type() == NodeType::ol_item);
    auto ol_item = (OrderedListItem*)item;
    REQUIRE(ol_item->size() == 3);
    {
      auto text = ol_item->children().at(0);
      REQUIRE(text->type() == NodeType::text);
    }
    auto link = ol_item->children().at(1);
    REQUIRE(link->type() == NodeType::link);
    {
      auto text = ol_item->children().at(2);
      REQUIRE(text->type() == NodeType::text);
    }
  }
}
TEST_CASE( "parse check box", "[cb]" ) {
  SECTION( "only text" ) {
    auto nodes = Parser::parse("- [ ] hhh");
    REQUIRE(nodes->size() == 1);
    auto node = nodes->childAt(0);
    REQUIRE(node->type() == NodeType::checkbox);
    auto ol = (OrderedList*)node;
    REQUIRE(ol->size() == 1);
    auto item = ol->children().at(0);
    REQUIRE(item->type() == NodeType::checkbox_item);
    auto ul_item = (OrderedListItem*)item;
    REQUIRE(ul_item->size() == 1);
    auto text = ul_item->children().at(0);
    REQUIRE(text->type() == NodeType::text);
  }
  SECTION( "only link" ) {
    auto nodes = Parser::parse("- [ ] [666](http://www.666.com)");
    REQUIRE(nodes->size() == 1);
    auto node = nodes->childAt(0);
    REQUIRE(node->type() == NodeType::checkbox);
    auto ol = (OrderedList*)node;
    REQUIRE(ol->size() == 1);
    auto item = ol->children().at(0);
    REQUIRE(item->type() == NodeType::checkbox_item);
    auto ol_item = (OrderedListItem*)item;
    REQUIRE(ol_item->size() == 1);
    auto link = ol_item->children().at(0);
    REQUIRE(link->type() == NodeType::link);
  }
  SECTION( "only link" ) {
    auto nodes = Parser::parse("- [ ] sdfg[666](http://www.666.com)sdfg");
    REQUIRE(nodes->size() == 1);
    auto node = nodes->childAt(0);
    REQUIRE(node->type() == NodeType::checkbox);
    auto ul = (OrderedList*)node;
    REQUIRE(ul->size() == 1);
    auto item = ul->children().at(0);
    REQUIRE(item->type() == NodeType::checkbox_item);
    auto ol_item = (OrderedListItem*)item;
    REQUIRE(ol_item->size() == 3);
    {
      auto text = ol_item->children().at(0);
      REQUIRE(text->type() == NodeType::text);
    }
    auto link = ol_item->children().at(1);
    REQUIRE(link->type() == NodeType::link);
    {
      auto text = ol_item->children().at(2);
      REQUIRE(text->type() == NodeType::text);
    }
  }
}
TEST_CASE( "parse image", "[image]" ) {
  SECTION( "only image" ) {
    auto nodes = Parser::parse("![666](http://www.666.com)");
    REQUIRE(nodes->size() == 1);
    auto node = nodes->childAt(0);
    REQUIRE(node->type() == NodeType::paragraph);
    auto p = (Paragraph*)node;
    REQUIRE(p->size() == 1);
    auto link = p->children().at(0);
    REQUIRE(link->type() == NodeType::image);
  }
}
TEST_CASE( "parse link", "[link]" ) {
  SECTION( "only link" ) {
    auto nodes = Parser::parse("[666](http://www.666.com)");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::link);
  }
  SECTION( "text link text" ) {
    auto nodes = Parser::parse("before[666](http://www.666.com)after");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
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
}
TEST_CASE( "parse inline code", "[inline][code]" ) {
  SECTION( "only inline code" ) {
    auto nodes = Parser::parse("`#include`");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::inline_code);
  }
  SECTION( "text inline code text" ) {
    auto nodes = Parser::parse("before`#include`after");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
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

}
TEST_CASE( "parse inline latex", "[inline][latex]" ) {
  SECTION( "only inline latex" ) {
    Parser parser;
    auto nodes = Parser::parse("$a^2$");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::inline_latex);
  }
  SECTION( "text inline latex text" ) {
    auto nodes = Parser::parse("before$a^2$after");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
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
}
TEST_CASE( "parse semantic", "[italic][bold]" ) {
  SECTION( "only italic" ) {
    auto nodes = Parser::parse("*666*");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::italic);
  }
  SECTION( "text only italic text" ) {
    auto nodes = Parser::parse("before*666*after");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
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
  SECTION( "only bold" ) {
    auto nodes = Parser::parse("**666**");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::bold);
  }
  SECTION( "text bold text" ) {
    auto nodes = Parser::parse("before**666**after");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
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
  SECTION( "italic and bold" ) {
    auto nodes = Parser::parse("***666***");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::italic_bold);
  }
  SECTION( "text italic and bold text" ) {
    auto nodes = Parser::parse("before***666***after");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
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
  SECTION( "strickout" ) {
    auto nodes = Parser::parse("~~666~~");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto p = (Paragraph*)node;
    ASSERT_EQ(p->children().size(), 1);
    auto link = p->children().at(0);
    ASSERT_EQ(link->type(), NodeType::strickout);
  }
  SECTION( "text strickout text" ) {
    auto nodes = Parser::parse("before~~666~~after");
    ASSERT_EQ(nodes->size(), 1);
    auto node = nodes->childAt(0);
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
}
