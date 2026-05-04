//
// Created by PikachuHy on 2021/11/15.
//
#include "parser/Document.h"
#include "parser/Parser.h"
#include "parser/Text.h"
#include "render/Render.h"
#include "SimpleFontMetricsProvider.h"

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
  SimpleFontMetricsProvider fm;
  auto doc = new Document(R"(
```
a
b
```
)");
  auto root = doc->root();
  REQUIRE(root->size() == 1);
  {
    auto node = root->childAt(0);
    CHECK(node->type() == NodeType::code_block);
    auto codeBlock = node2codeBlock(node);
    CHECK(codeBlock->size() == 2);
    auto block = Render::render(codeBlock, setting, *doc, &fm);
    CHECK(block.countOfLogicalLine() == 2);
  }
}

int main(int argc, char** argv) {
  // No QGuiApplication needed -- font metrics are provided by SimpleFontMetricsProvider.
  // codeFont() uses hardcoded monospace families (Menlo/monospace) instead of QFontDatabase.
  doctest::Context context;
  int res = context.run();
  if (context.shouldExit())
    return res;
  return res;
}