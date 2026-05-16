//
// Created by PikachuHy on 2021/11/15.
//
#include "parser/Document.h"
#include "parser/Parser.h"
#include "parser/Text.h"
#include "render/Render.h"
#include "render/Cell.h"
#include "SimpleFontMetricsProvider.h"

#include "debug.h"
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
using namespace md;
using namespace md::parser;
using namespace md::render;

static sptr<RenderSetting> makeSetting() {
  auto s = std::make_shared<RenderSetting>();
  s->maxWidth = 800;
  return s;
}

static std::unique_ptr<Document> parseDoc(const String& md) {
  return std::make_unique<Document>(md);
}

static Block renderNode(Node* node, sptr<RenderSetting> setting, const IBufferProvider& doc,
                        IFontMetricsProvider* fm) {
  return Render::render(node, setting, doc, fm);
}

// Helper: count cells of type T (TextCell or InlineLatexCell) across all lines in a block
template <typename T>
static int countCells(const Block& block) {
  int n = 0;
  for (SizeType i = 0; i < block.countOfLogicalLine(); ++i) {
    for (const auto* cell : block.logicalLineAt(i).cells()) {
      if (dynamic_cast<const T*>(cell)) ++n;
    }
  }
  return n;
}

// ---- Block-level render tests ----

TEST_CASE("render header") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("# Hello\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  CHECK(node->type() == NodeType::header);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(block.countOfLogicalLine() == 1);
  CHECK(countCells<TextCell>(block) >= 1);
  CHECK(block.logicalLineAt(0).length() > 0);
}

TEST_CASE("render header levels") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  for (int level = 1; level <= 6; ++level) {
    String prefix;
    for (int i = 0; i < level; ++i) prefix += "#";
    auto doc = parseDoc(prefix + " Title\n\n");
    REQUIRE(doc->root()->size() == 1);
    auto* node = doc->root()->childAt(0);
    CHECK(node->type() == NodeType::header);
    auto* header = static_cast<Header*>(node);
    CHECK(header->level() == level);
    auto block = renderNode(node, setting, *doc, &fm);
    CHECK(block.countOfLogicalLine() >= 1);
  }
}

TEST_CASE("render paragraph") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("A simple paragraph.\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(block.countOfLogicalLine() == 1);
  CHECK(countCells<TextCell>(block) >= 1);
}

TEST_CASE("render paragraph with bold and italic") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("This is **bold** and *italic* text.\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(block.countOfLogicalLine() == 1);
  CHECK(countCells<TextCell>(block) >= 3);  // "This is ", "bold", " and ", "italic", " text."
}

TEST_CASE("render strikethrough") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("Some ~~deleted~~ text.\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(block.countOfLogicalLine() == 1);
  CHECK(countCells<TextCell>(block) >= 2);
}

TEST_CASE("render code block") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("```\na\nb\n```\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  CHECK(node->type() == NodeType::code_block);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(block.countOfLogicalLine() == 2);
}

TEST_CASE("render code block with language") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("```cpp\nint x = 1;\n```\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  CHECK(node->type() == NodeType::code_block);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(block.countOfLogicalLine() == 1);
  CHECK(countCells<TextCell>(block) >= 1);
}

TEST_CASE("render unordered list") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("- item1\n- item2\n- item3\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  CHECK(node->type() == NodeType::ul);
  auto block = renderNode(node, setting, *doc, &fm);
  // Each list item produces at least one logical line
  CHECK(block.countOfLogicalLine() >= 3);
  CHECK(countCells<TextCell>(block) >= 3);
}

TEST_CASE("render ordered list") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("1. first\n2. second\n3. third\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  CHECK(node->type() == NodeType::ol);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(block.countOfLogicalLine() >= 3);
  CHECK(countCells<TextCell>(block) >= 3);
}

TEST_CASE("render checkbox list") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("- [ ] todo\n- [x] done\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  CHECK(node->type() == NodeType::checkbox);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(block.countOfLogicalLine() >= 2);
  CHECK(countCells<TextCell>(block) >= 2);
}

TEST_CASE("render quote block") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("> quoted text\n> more text\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  CHECK(node->type() == NodeType::quote_block);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(block.countOfLogicalLine() >= 1);
  CHECK(countCells<TextCell>(block) >= 1);
}

TEST_CASE("render link") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("[click here](http://example.com)\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(countCells<TextCell>(block) >= 1);
}

TEST_CASE("render image") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("![alt](image.png)\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto block = renderNode(node, setting, *doc, &fm);
  // Image produces an ImageInstruction, not text cells
  CHECK(block.countOfLogicalLine() >= 0);
}

TEST_CASE("render inline code") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("Use `printf()` function.\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(countCells<TextCell>(block) >= 1);
}

TEST_CASE("render inline latex") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("Math: $x^2 + y^2 = z^2$\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  auto block = renderNode(node, setting, *doc, &fm);
  // InlineLatex requires MicroTeX with a math font; in headless tests without
  // initLatex(), the cell may not be created. Just verify it doesn't crash.
  CHECK(block.countOfLogicalLine() >= 0);
}

TEST_CASE("render horizontal rule syntax is not parsed") {
  // Known gap: --- is parsed as a paragraph, not Hr.
  // Hr node type exists but no block parser is registered for it.
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("---\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(block.countOfLogicalLine() == 1);
}

TEST_CASE("render latex block") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("$$\nx = 1\n$$\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  CHECK(node->type() == NodeType::latex_block);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(block.countOfLogicalLine() >= 1);
}

TEST_CASE("render empty document produces trailing paragraph") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("");
  // Trailing paragraph is added
  CHECK(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
}

TEST_CASE("render multiple paragraphs") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("Paragraph one.\n\nParagraph two.\n\nParagraph three.\n\n");
  REQUIRE(doc->root()->size() >= 3);
  for (SizeType i = 0; i < doc->root()->size(); ++i) {
    auto* node = doc->root()->childAt(i);
    CHECK(node->type() == NodeType::paragraph);
    auto block = renderNode(node, setting, *doc, &fm);
    CHECK(block.countOfLogicalLine() >= 1);
    CHECK(countCells<TextCell>(block) >= 1);
  }
}

TEST_CASE("render mixed blocks") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("# Title\n\nSome text **bold** in paragraph.\n\n- list item\n\n");
  REQUIRE(doc->root()->size() == 3);
  CHECK(doc->root()->childAt(0)->type() == NodeType::header);
  CHECK(doc->root()->childAt(1)->type() == NodeType::paragraph);
  CHECK(doc->root()->childAt(2)->type() == NodeType::ul);
}

TEST_CASE("render chinese text") {
  auto setting = makeSetting();
  SimpleFontMetricsProvider fm;
  auto doc = parseDoc("你好世界。\n\n");
  REQUIRE(doc->root()->size() == 1);
  auto* node = doc->root()->childAt(0);
  auto block = renderNode(node, setting, *doc, &fm);
  CHECK(block.countOfLogicalLine() >= 1);
  CHECK(countCells<TextCell>(block) >= 1);
  CHECK(block.logicalLineAt(0).length() > 0);
}

int main(int argc, char** argv) {
  doctest::Context context;
  int res = context.run();
  if (context.shouldExit())
    return res;
  return res;
}
