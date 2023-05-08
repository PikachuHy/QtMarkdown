//
// Created by PikachuHy on 2021/11/15.
//
#define private public
#define protected public
#include "parser/Document.h"
#include "parser/Parser.h"
#include "parser/Text.h"
#include "render/Render.h"
#undef protected
#undef private
#include <QGuiApplication>

#include "debug.h"
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
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
TEST_CASE("testing render code block, two line") {
  auto setting = std::make_shared<RenderSetting>();
  auto doc = new Document(R"(
```
a
b
```
)");
  auto root = doc->m_root;
  REQUIRE(root->size() == 1);
  {
    auto node = root->childAt(0);
    CHECK(node->type() == NodeType::code_block);
    auto codeBlock = node2codeBlock(node);
    CHECK(codeBlock->size() == 2);
    auto block = Render::render(codeBlock, setting, doc);
    CHECK(block.countOfLogicalLine() == 2);
  }
}

int main(int argc, char** argv) {
  // 必须加这一句
  // 不然调用字体(QFontMetric)时会崩溃
  QGuiApplication app(argc, argv);
  doctest::Context context;

  int res = context.run(); // run

  if (context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
    return res;             // propagate the result of the tests

  int client_stuff_return_code = 0;
  // your program - if the testing framework is integrated in your production code

  return res + client_stuff_return_code; // the result from doctest is propagated here as well
}