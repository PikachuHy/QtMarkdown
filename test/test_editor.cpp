//
// Created by PikachuHy on 2021/11/13.
//
#define private public
#define protected public
#include "editor/Cursor.h"
#include "editor/Document.h"
#include "editor/Editor.h"
#include "parser/Document.h"
#include "parser/Text.h"
#undef protected
#undef private
#include <QGuiApplication>

#include "debug.h"
using namespace md::editor;
using md::parser::NodeType;
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>
TEST_CASE("ParagraphEditTest == EmptyParagraphInsertText") {
  Editor editor;
  editor.loadText("");
  editor.insertText("a");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto p = doc->m_root->childAt(0);

  CHECK(p->type() == NodeType::paragraph);
  auto paragraphNode = (md::parser::Paragraph*)p;
  CHECK(paragraphNode->size() == 1);
  auto child = paragraphNode->childAt(0);
  CHECK(child->type() == NodeType::text);
  auto textNode = (md::parser::Text*)child;
  auto s = textNode->toString(doc.get());
  CHECK(s == "a");
}
TEST_CASE("ParagraphEditTest == EmptyParagraphInsertReturn") {
  Editor editor;
  editor.loadText("");
  auto doc = editor.document();
  doc->insertReturn(*editor.m_cursor);
  auto& blocks = doc->m_blocks;
  CHECK(blocks.size() == 2);
  CHECK(doc->m_root->size() == 2);
  {
    auto p = doc->m_root->childAt(0);

    CHECK(p->type() == NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)p;
    CHECK(paragraphNode->size() == 0);
  }
  {
    auto p = doc->m_root->childAt(1);

    CHECK(p->type() == NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)p;
    CHECK(paragraphNode->size() == 0);
  }
}
TEST_CASE("ParagraphEditTest == EmptyParagraphInsertTextAndRemoveText") {
  Editor editor;
  editor.loadText("");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  editor.insertText("a");
  editor.insertText("c");
  auto coord = doc->moveCursorToLeft(cursor.coord());
  doc->updateCursor(cursor, coord);
  editor.insertText("b");
  {
    auto p = doc->m_root->childAt(0);
    CHECK(p->type() == NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)p;
    CHECK(paragraphNode->size() == 1);
    auto node = paragraphNode->childAt(0);
    CHECK(node->type() == md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)node;
    auto s = textNode->toString(doc.get());
    CHECK(s == "abc");
  }
  doc->removeText(cursor);
  {
    auto p = doc->m_root->childAt(0);
    CHECK(p->type() == NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)p;
    CHECK(paragraphNode->size() == 1);
    auto node = paragraphNode->childAt(0);
    CHECK(node->type() == md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)node;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("ac"));
  }
}
TEST_CASE("ParagraphEditTest == RemoveInStartOfPargraph") {
  Editor editor;
  editor.loadText(R"(
a

b
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 2);
  CHECK(doc->m_root->size() == 2);
  auto coord = cursor.coord();
  coord.blockNo = 1;
  coord.lineNo = 0;
  coord.offset = 0;
  cursor.setCoord(coord);
  doc->removeText(cursor);

  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  {
    auto p = doc->m_root->childAt(0);
    CHECK(p->type() == NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)p;
    CHECK(paragraphNode->size() == 1);
    auto node = paragraphNode->childAt(0);
    CHECK(node->type() == md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)node;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("ab"));
  }
}
TEST_CASE("ParagraphEditTest == RemoveEmoji") {
  Editor editor;
  editor.loadText(R"(
a😊b
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  CHECK(cursor.coord().offset == 4);
  {
    auto& line = blocks[0].logicalLineAt(0);
    CHECK(line.length() == 4);
  }
  doc->removeText(cursor);
  CHECK(cursor.coord().offset == 3);
  {
    auto& line = blocks[0].logicalLineAt(0);
    CHECK(line.length() == 3);
  }
  doc->removeText(cursor);
  CHECK(cursor.coord().offset == 1);
  {
    auto& line = blocks[0].logicalLineAt(0);
    CHECK(line.length() == 1);
  }
  doc->removeText(cursor);
  CHECK(cursor.coord().offset == 0);
  {
    auto& line = blocks[0].logicalLineAt(0);
    CHECK(line.length() == 0);
  }
}
TEST_CASE("ParagraphEditTest == RemoveEmoji2") {
  Editor editor;
  QString s;
  editor.loadText(R"(
a [666](www.baidu.com) b 😊
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  {
    auto& line = blocks[0].logicalLineAt(0);
    CHECK(line.length() == 10);
  }
  doc->removeText(cursor);
  {
    auto& line = blocks[0].logicalLineAt(0);
    CHECK(line.length() == 8);
  }
}

TEST_CASE("ParagraphEditTest == RemoveText") {
  Editor editor;
  QString s;
  editor.loadText(R"(
6

ab
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 2);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  coord = doc->moveCursorToLeft(coord);
  doc->updateCursor(cursor, coord);
  doc->removeText(cursor);
  CHECK(blocks.size() == 2);
}

TEST_CASE("ParagraphEditTest == RemoveEmptyLink") {
  Editor editor;
  QString s;
  editor.loadText(R"(
a[b](c)d
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  doc->removeText(cursor);
  CHECK(blocks.size() == 1);
  doc->removeText(cursor);
  CHECK(blocks.size() == 1);
  doc->removeText(cursor);
  CHECK(blocks.size() == 1);
}

TEST_CASE("ParagraphEditTest == UpgradeToHeader") {
  Editor editor;
  editor.loadText("#");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  editor.insertText(" ");
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto node = doc->m_root->childAt(0);
  CHECK(node->type() == NodeType::header);
  auto headerNode = (md::parser::Header*)node;
  CHECK(headerNode->size() == 0);
}

TEST_CASE("ParagraphEditTest == UpgradeToHeaderAndDegradeToParagraphAndRemoveText") {
  Editor editor;
  QString s;
  editor.loadText(R"(
a
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  doc->insertText(cursor, "#");
  CHECK(blocks.size() == 1);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    CHECK(paragraphNode->size() == 1);
    auto child = paragraphNode->childAt(0);
    CHECK(child->type() == NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("#a"));
  }
  doc->insertText(cursor, " ");
  CHECK(blocks.size() == 1);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::header);
    auto headerNode = (md::parser::Header*)node;
    CHECK(headerNode->size() == 1);
    auto child = headerNode->childAt(0);
    CHECK(child->type() == NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("a"));
    CursorCoord _coord;
    _coord.blockNo = 0;
    _coord.lineNo = 0;
    _coord.offset = 0;
    CHECK(cursor.coord() == _coord);
  }
  doc->removeText(cursor);
  CHECK(blocks.size() == 1);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    CHECK(paragraphNode->size() == 1);
    auto child = paragraphNode->childAt(0);
    CHECK(child->type() == NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("a"));
  }
  doc->removeText(cursor);
  CHECK(blocks.size() == 1);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    CHECK(paragraphNode->size() == 1);
    auto child = paragraphNode->childAt(0);
    CHECK(child->type() == NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("a"));
  }
}
TEST_CASE("ParagraphEditTest == UpgradeToUl") {
  Editor editor;
  editor.loadText("-");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  editor.insertText(" ");
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto node = doc->m_root->childAt(0);
  CHECK(node->type() == NodeType::ul);
  auto ulNode = (md::parser::UnorderedList*)node;
  CHECK(ulNode->size() == 1);
  auto ulItem = ulNode->childAt(0);
  CHECK(ulItem->type() == md::parser::NodeType::ul_item);
  auto ulItemNode = (md::parser::UnorderedListItem*)ulItem;
  CHECK(ulItemNode->size() == 0);
}
TEST_CASE("ParagraphEditTest == UpgradeToCodeBlock") {
  Editor editor;
  editor.loadText("```");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);

  doc->insertReturn(cursor);
  {
    CHECK(blocks.size() == 1);
    CHECK(doc->m_root->size() == 1);
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    CHECK(codeBlockNode->size() == 0);
  }
}
TEST_CASE("ParagraphEditTest == UpgradeToCodeBlockWithOtherText") {
  Editor editor;
  editor.loadText("```asdfasdf");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto coord = doc->moveCursorToBol(cursor.coord());
  doc->updateCursor(cursor, coord);
  coord = doc->moveCursorToRight(cursor.coord());
  doc->updateCursor(cursor, coord);
  coord = doc->moveCursorToRight(cursor.coord());
  doc->updateCursor(cursor, coord);
  coord = doc->moveCursorToRight(cursor.coord());
  doc->updateCursor(cursor, coord);
  CHECK(cursor.coord().offset == 3);
  doc->insertReturn(cursor);
  CHECK(blocks.size() == 2);
  CHECK(doc->m_root->size() == 2);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    CHECK(codeBlockNode->size() == 0);
  }
  {
    auto node = doc->m_root->childAt(1);
    CHECK(node->type() == NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    CHECK(paragraphNode->size() == 1);
    auto child = paragraphNode->childAt(0);
    CHECK(child->type() == NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("asdfasdf"));
  }
}
TEST_CASE("UlEditTest == DegradeToParagraph") {
  Editor editor;
  editor.loadText("- ");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  doc->removeText(*editor.m_cursor);
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto node = doc->m_root->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto paragraphNode = (md::parser::Paragraph*)node;
  CHECK(paragraphNode->size() == 0);
}
TEST_CASE("UlEditTest == InsertSpace") {
  Editor editor;
  editor.loadText("- []");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  coord = doc->moveCursorToLeft(coord);
  doc->updateCursor(cursor, coord);
  doc->insertText(cursor, " ");

  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::ul);
    auto ulNode = (md::parser::UnorderedList*)node;
    CHECK(ulNode->size() == 1);
    auto ulItem = ulNode->childAt(0);
    CHECK(ulItem->type() == md::parser::NodeType::ul_item);
    auto ulItemNode = (md::parser::UnorderedListItem*)ulItem;
    CHECK(ulItemNode->size() == 1);
    auto child = ulItemNode->childAt(0);
    CHECK(child->type() == NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("[ ]"));
  }
}
TEST_CASE("UlEditTest == UpgradeToCheckbox") {
  Editor editor;
  editor.loadText("- ");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  editor.insertText("[ ]");
  editor.insertText(" ");
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto node = doc->m_root->childAt(0);
  CHECK(node->type() == NodeType::checkbox);
  auto ulNode = (md::parser::UnorderedList*)node;
  CHECK(ulNode->size() == 1);
  auto ulItem = ulNode->childAt(0);
  CHECK(ulItem->type() == md::parser::NodeType::checkbox_item);
  auto ulItemNode = (md::parser::UnorderedListItem*)ulItem;
  CHECK(ulItemNode->size() == 0);
}
TEST_CASE("UlEditTest == InsertReturn") {
  Editor editor;
  editor.loadText(R"(
- GIF啊
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  {
    CHECK(blocks.size() == 1);
    CHECK(doc->m_root->size() == 1);
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::ul);
    auto ulNode = (md::parser::UnorderedList*)node;
    CHECK(ulNode->size() == 1);
    auto ulItem = ulNode->childAt(0);
    CHECK(ulItem->type() == NodeType::ul_item);
    auto ulItemNode = (md::parser::UnorderedListItem*)ulItem;
    CHECK(ulItemNode->size() == 1);
    auto text = ulItemNode->childAt(0);
    CHECK(text->type() == NodeType::text);
    auto textNode = (md::parser::Text*)text;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("GIF啊"));
    auto& line = blocks[0].logicalLineAt(0);
    CHECK(line.length() == 4);
  }
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  doc->insertReturn(cursor);
  {
    CHECK(blocks.size() == 1);
    CHECK(doc->m_root->size() == 1);
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::ul);
    auto ulNode = (md::parser::UnorderedList*)node;
    CHECK(ulNode->size() == 2);
    auto ulItem = ulNode->childAt(0);
    CHECK(ulItem->type() == NodeType::ul_item);
    auto ulItemNode = (md::parser::UnorderedListItem*)ulItem;
    CHECK(ulItemNode->size() == 1);
    auto text = ulItemNode->childAt(0);
    CHECK(text->type() == NodeType::text);
    auto textNode = (md::parser::Text*)text;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("GIF啊"));
    auto& line = blocks[0].logicalLineAt(0);
    CHECK(line.length() == 4);
  }
}
TEST_CASE("CheckboxEditTest == DegradeToParagraph") {
  Editor editor;
  editor.loadText("- [ ] ");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  CHECK(blocks[0].countOfLogicalLine() == 1);
  CHECK(blocks[0].logicalLineAt(0).m_cells.size() == 0);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  doc->removeText(*editor.m_cursor);
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto node = doc->m_root->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto paragraphNode = (md::parser::Paragraph*)node;
  CHECK(paragraphNode->size() == 0);
}
TEST_CASE("CheckboxEditTest == DegradeToParagraph2") {
  Editor editor;
  editor.loadText("- [ ] 666");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  CHECK(blocks[0].countOfLogicalLine() == 1);
  CHECK(blocks[0].logicalLineAt(0).m_cells.size() == 1);
  auto coord = doc->moveCursorToBeginOfDocument();
  doc->updateCursor(cursor, coord);
  doc->removeText(*editor.m_cursor);
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  auto node = doc->m_root->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto paragraphNode = (md::parser::Paragraph*)node;
  CHECK(paragraphNode->size() == 1);
}

TEST_CASE("CheckboxEditTest == EmptyCheckBoxInsertReturn") {
  Editor editor;
  editor.loadText(R"(
- [ ] a
- [ ] b
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == md::parser::NodeType::checkbox);
  }
  CursorCoord coord;
  coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  doc->removeText(cursor);
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == md::parser::NodeType::checkbox);
  }
  doc->insertReturn(cursor);
  CHECK(blocks.size() == 2);
  CHECK(doc->m_root->size() == 2);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == md::parser::NodeType::checkbox);
  }
  {
    auto node = doc->m_root->childAt(1);
    CHECK(node->type() == md::parser::NodeType::paragraph);
  }
}
TEST_CASE("EditorTest == HeaderReturn") {
  Editor editor;
  editor.loadText("");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  editor.insertText("#");
  editor.insertText(" ");
  editor.insertText("asdfljsaf");
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == md::parser::NodeType::header);
  }
  doc->insertReturn(cursor);
  CHECK(blocks.size() == 2);
  CHECK(doc->m_root->size() == 2);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == md::parser::NodeType::header);
  }
  {
    auto node = doc->m_root->childAt(1);
    CHECK(node->type() == md::parser::NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    CHECK(paragraphNode->size() == 0);
  }
}

TEST_CASE("CodeBlockEditTest == InsertText") {
  Editor editor;
  editor.loadText(R"(
```
```
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == md::parser::NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    CHECK(codeBlockNode->size() == 0);
  }
  doc->insertText(cursor, "a");
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == md::parser::NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    CHECK(codeBlockNode->size() == 1);
    auto child = codeBlockNode->childAt(0);
    CHECK(child->type() == md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("a"));
  }
  doc->insertReturn(cursor);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == md::parser::NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    CHECK(codeBlockNode->size() == 2);
    auto child = codeBlockNode->childAt(0);
    CHECK(child->type() == md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("a"));
    CHECK(blocks[0].countOfLogicalLine() == 2);
  }
  doc->insertText(cursor, "b");
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == md::parser::NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    CHECK(codeBlockNode->size() == 2);
    CHECK(blocks[0].countOfLogicalLine() == 2);
    {
      auto child = codeBlockNode->childAt(0);
      CHECK(child->type() == md::parser::NodeType::text);
      auto textNode = (md::parser::Text*)child;
      auto s = textNode->toString(doc.get());
      CHECK(s == QString("a"));
    }
    {
      auto child = codeBlockNode->childAt(1);
      CHECK(child->type() == md::parser::NodeType::text);
      auto textNode = (md::parser::Text*)child;
      auto s = textNode->toString(doc.get());
      CHECK(s == QString("b"));
    }
  }
  doc->removeText(cursor);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == md::parser::NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    CHECK(codeBlockNode->size() == 2);
    auto child = codeBlockNode->childAt(0);
    CHECK(child->type() == md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("a"));
    CHECK(blocks[0].countOfLogicalLine() == 2);
  }
  doc->removeText(cursor);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == md::parser::NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    CHECK(codeBlockNode->size() == 1);
    auto child = codeBlockNode->childAt(0);
    CHECK(child->type() == md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("a"));
    CHECK(blocks[0].countOfLogicalLine() == 1);
  }
}

TEST_CASE("CursorMoveTest == Emoji") {
  Editor editor;
  editor.loadText(R"(
a😊b
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  CHECK(cursor.coord().offset == 0);
  CursorCoord coord;
  coord = doc->moveCursorToRight(cursor.coord());
  doc->updateCursor(cursor, coord);
  CHECK(cursor.coord().offset == 1);
  coord = doc->moveCursorToRight(cursor.coord());
  doc->updateCursor(cursor, coord);
  CHECK(cursor.coord().offset == 3);
  coord = doc->moveCursorToRight(cursor.coord());
  doc->updateCursor(cursor, coord);
  CHECK(cursor.coord().offset == 4);
  coord = doc->moveCursorToLeft(cursor.coord());
  doc->updateCursor(cursor, coord);
  CHECK(cursor.coord().offset == 3);
  coord = doc->moveCursorToLeft(cursor.coord());
  doc->updateCursor(cursor, coord);
  CHECK(cursor.coord().offset == 1);
  coord = doc->moveCursorToLeft(cursor.coord());
  doc->updateCursor(cursor, coord);
  CHECK(cursor.coord().offset == 0);
}
TEST_CASE("PreeditTest == ShowPreedit") {
  Editor editor;
  editor.loadText(R"()");
  editor.setPreedit("a");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  CHECK(cursor.coord().offset == 1);
  CHECK(blocks[0].logicalLineAt(0).length() == 1);
  auto node = doc->m_root->childAt(0);
  CHECK(node->type() == NodeType::paragraph);
  auto paragraphNode = (md::parser::Paragraph*)node;
  CHECK(paragraphNode->size() == 1);
  editor.setPreedit("ab");
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  CHECK(cursor.coord().offset == 2);
  CHECK(paragraphNode->size() == 1);
  CHECK(blocks[0].logicalLineAt(0).length() == 2);
  editor.setPreedit("abc");
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  CHECK(cursor.coord().offset == 3);
  CHECK(paragraphNode->size() == 1);
  CHECK(blocks[0].logicalLineAt(0).length() == 3);
}

TEST_CASE("PreeditTest == ShowPreedit2") {
  Editor editor;
  editor.loadText(R"(
# a
)");

  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  CursorCoord coord;
  coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  doc->removeText(cursor);
  CHECK(blocks.size() == 1);
  CHECK(doc->m_root->size() == 1);
  CHECK(cursor.coord().offset == 0);
  CHECK(blocks[0].logicalLineAt(0).length() == 0);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::header);
    auto header = (md::parser::Header*)node;
    CHECK(header->size() == 0);
  }
  editor.setPreedit("a");
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::header);
    auto header = (md::parser::Header*)node;
    CHECK(header->size() == 1);
    CHECK(header->childAt(0)->type() == NodeType::text);
    auto textNode = (md::parser::Text*)header->childAt(0);
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("a"));
  }
}

TEST_CASE("MultiBlockEditTest == RemoveEmptyParagraph") {
  Editor editor;
  editor.loadText(R"(
# a
c
# b
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;

  CHECK(blocks.size() == 3);
  CHECK(doc->m_root->size() == 3);
  auto coord = cursor.coord();
  coord.blockNo = 1;
  coord.lineNo = 0;
  coord.offset = 1;
  doc->updateCursor(cursor, coord);
  doc->removeText(cursor);
  doc->removeText(cursor);
  CHECK(blocks.size() == 2);
  CHECK(doc->m_root->size() == 2);
  coord = cursor.coord();
  CHECK(coord.blockNo == 0);
  CHECK(coord.lineNo == 0);
  CHECK(coord.offset == 1);
}
TEST_CASE("UndoTest == InsertReturn") {
  Editor editor;
  editor.loadText(R"(
ab
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  {
    auto coord = doc->moveCursorToEndOfDocument();
    doc->updateCursor(cursor, coord);
    coord = doc->moveCursorToLeft(coord);
    doc->updateCursor(cursor, coord);
  }
  doc->insertReturn(cursor);
  CHECK(blocks.size() == 2);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    CHECK(paragraphNode->size() == 1);
    auto child = paragraphNode->childAt(0);
    CHECK(child->type() == NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("a"));
  }
  {
    auto node = doc->m_root->childAt(1);
    CHECK(node->type() == NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    CHECK(paragraphNode->size() == 1);
    auto child = paragraphNode->childAt(0);
    CHECK(child->type() == NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("b"));
  }
  doc->undo(cursor);
  CHECK(blocks.size() == 1);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    CHECK(paragraphNode->size() == 1);
    auto child = paragraphNode->childAt(0);
    CHECK(child->type() == NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("ab"));
  }
  {
    auto coord = doc->moveCursorToEndOfDocument();
    doc->updateCursor(cursor, coord);
  }
  doc->insertReturn(cursor);
  doc->undo(cursor);
  CHECK(blocks.size() == 1);
  {
    auto node = doc->m_root->childAt(0);
    CHECK(node->type() == NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    CHECK(paragraphNode->size() == 1);
    auto child = paragraphNode->childAt(0);
    CHECK(child->type() == NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    CHECK(s == QString("ab"));
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