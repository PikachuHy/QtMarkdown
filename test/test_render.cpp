//
// Created by PikachuHy on 2021/11/15.
//
#include <string>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#define private public
#define protected public
#include "parser/Document.h"
#include "parser/Parser.h"
#include "parser/Text.h"
#include "render/Render.h"
#undef protected
#undef private
#include <QGuiApplication>
#include "common.h"
#include "debug.h"
using namespace md;
using namespace md::parser;
using namespace md::render;
CodeBlock* node2codeBlock(Node* node) {
  ASSERT(node->type() == NodeType::code_block);
  return (CodeBlock*)node;
}
Text* node2text(Node* node) {
  ASSERT(node->type() == NodeType::text);
  return (Text*)node;
}

TEST_CASE( "render block", "[block]" ) {
  SECTION( "two line" ) {
    auto setting = std::make_shared<RenderSetting>();
    auto doc = new Document(R"(
```
a
b
```
)");
    auto root = doc->m_root;
    ASSERT_EQ(root->size(), 1);
    {
      auto node = root->childAt(0);
      ASSERT_EQ(node->type(), NodeType::code_block);
      auto codeBlock = node2codeBlock(node);
      ASSERT_EQ(codeBlock->size(), 2);
      auto block = Render::render(codeBlock, setting, doc);
      ASSERT_EQ(block.countOfLogicalLine(), 2);
    }
  }
}


int main(int argc, char** argv) {
  // 必须加这一句
  // 不然调用字体(QFontMetric)时会崩溃
  QGuiApplication app(argc, argv);
#ifdef BUILD_STATIC
  Q_INIT_RESOURCE(md);
#endif
  int result = Catch::Session().run(argc, argv);
  return result;
}