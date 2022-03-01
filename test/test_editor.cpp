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
// 让GTest的QString输出更加友好
void PrintTo(const QString& s, std::ostream* os) { *os << s.toStdString(); }

#include "gtest/gtest.h"
TEST(ParagraphEditTest, EmptyParagraphInsertText) {
  Editor editor;
  editor.loadText("");
  editor.insertText("a");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto p = doc->m_root->childAt(0);

  ASSERT_EQ(p->type(), NodeType::paragraph);
  auto paragraphNode = (md::parser::Paragraph*)p;
  ASSERT_EQ(paragraphNode->size(), 1);
  auto child = paragraphNode->childAt(0);
  ASSERT_EQ(child->type(), NodeType::text);
  auto textNode = (md::parser::Text*)child;
  auto s = textNode->toString(doc.get());
  ASSERT_EQ(s, "a");
}
TEST(ParagraphEditTest, EmptyParagraphInsertReturn) {
  Editor editor;
  editor.loadText("");
  auto doc = editor.document();
  doc->insertReturn(*editor.m_cursor);
  auto& blocks = doc->m_blocks;
  ASSERT_EQ(blocks.size(), 2);
  ASSERT_EQ(doc->m_root->size(), 2);
  {
    auto p = doc->m_root->childAt(0);

    ASSERT_EQ(p->type(), NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)p;
    ASSERT_EQ(paragraphNode->size(), 0);
  }
  {
    auto p = doc->m_root->childAt(1);

    ASSERT_EQ(p->type(), NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)p;
    ASSERT_EQ(paragraphNode->size(), 0);
  }
}
TEST(ParagraphEditTest, EmptyParagraphInsertTextAndRemoveText) {
  Editor editor;
  editor.loadText("");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  editor.insertText("a");
  editor.insertText("c");
  auto coord = doc->moveCursorToLeft(cursor.coord());
  doc->updateCursor(cursor, coord);
  editor.insertText("b");
  {
    auto p = doc->m_root->childAt(0);
    ASSERT_EQ(p->type(), NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)p;
    ASSERT_EQ(paragraphNode->size(), 1);
    auto node = paragraphNode->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)node;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, "abc");
  }
  doc->removeText(cursor);
  {
    auto p = doc->m_root->childAt(0);
    ASSERT_EQ(p->type(), NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)p;
    ASSERT_EQ(paragraphNode->size(), 1);
    auto node = paragraphNode->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)node;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("ac"));
  }
}
TEST(ParagraphEditTest, RemoveInStartOfPargraph) {
  Editor editor;
  editor.loadText(R"(
a

b
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 2);
  ASSERT_EQ(doc->m_root->size(), 2);
  auto coord = cursor.coord();
  coord.blockNo = 1;
  coord.lineNo = 0;
  coord.offset = 0;
  cursor.setCoord(coord);
  doc->removeText(cursor);

  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  {
    auto p = doc->m_root->childAt(0);
    ASSERT_EQ(p->type(), NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)p;
    ASSERT_EQ(paragraphNode->size(), 1);
    auto node = paragraphNode->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)node;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("ab"));
  }
}
TEST(ParagraphEditTest, RemoveEmoji) {
  Editor editor;
  editor.loadText(R"(
a😊b
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  ASSERT_EQ(cursor.coord().offset, 4);
  {
    auto& line = blocks[0].logicalLineAt(0);
    ASSERT_EQ(line.length(), 4);
  }
  doc->removeText(cursor);
  ASSERT_EQ(cursor.coord().offset, 3);
  {
    auto& line = blocks[0].logicalLineAt(0);
    ASSERT_EQ(line.length(), 3);
  }
  doc->removeText(cursor);
  ASSERT_EQ(cursor.coord().offset, 1);
  {
    auto& line = blocks[0].logicalLineAt(0);
    ASSERT_EQ(line.length(), 1);
  }
  doc->removeText(cursor);
  ASSERT_EQ(cursor.coord().offset, 0);
  {
    auto& line = blocks[0].logicalLineAt(0);
    ASSERT_EQ(line.length(), 0);
  }
}
TEST(ParagraphEditTest, RemoveEmoji2) {
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
    ASSERT_EQ(line.length(), 10);
  }
  doc->removeText(cursor);
  {
    auto& line = blocks[0].logicalLineAt(0);
    ASSERT_EQ(line.length(), 8);
  }
}

TEST(ParagraphEditTest, RemoveText) {
  Editor editor;
  QString s;
  editor.loadText(R"(
6

ab
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 2);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  coord = doc->moveCursorToLeft(coord);
  doc->updateCursor(cursor, coord);
  doc->removeText(cursor);
  ASSERT_EQ(blocks.size(), 2);
}

TEST(ParagraphEditTest, RemoveEmptyLink) {
  Editor editor;
  QString s;
  editor.loadText(R"(
a[b](c)d
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  doc->removeText(cursor);
  ASSERT_EQ(blocks.size(), 1);
  doc->removeText(cursor);
  ASSERT_EQ(blocks.size(), 1);
  doc->removeText(cursor);
  ASSERT_EQ(blocks.size(), 1);
}

TEST(ParagraphEditTest, UpgradeToHeader) {
  Editor editor;
  editor.loadText("#");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  editor.insertText(" ");
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto node = doc->m_root->childAt(0);
  ASSERT_EQ(node->type(), NodeType::header);
  auto headerNode = (md::parser::Header*)node;
  ASSERT_EQ(headerNode->size(), 0);
}

TEST(ParagraphEditTest, UpgradeToHeaderAndDegradeToParagraphAndRemoveText) {
  Editor editor;
  QString s;
  editor.loadText(R"(
a
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  doc->insertText(cursor, "#");
  ASSERT_EQ(blocks.size(), 1);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    ASSERT_EQ(paragraphNode->size(), 1);
    auto child = paragraphNode->childAt(0);
    ASSERT_EQ(child->type(), NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("#a"));
  }
  doc->insertText(cursor, " ");
  ASSERT_EQ(blocks.size(), 1);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::header);
    auto headerNode = (md::parser::Header*)node;
    ASSERT_EQ(headerNode->size(), 1);
    auto child = headerNode->childAt(0);
    ASSERT_EQ(child->type(), NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("a"));
    CursorCoord _coord;
    _coord.blockNo = 0;
    _coord.lineNo = 0;
    _coord.offset = 0;
    ASSERT_EQ(cursor.coord(), _coord);
  }
  doc->removeText(cursor);
  ASSERT_EQ(blocks.size(), 1);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    ASSERT_EQ(paragraphNode->size(), 1);
    auto child = paragraphNode->childAt(0);
    ASSERT_EQ(child->type(), NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("a"));
  }
  doc->removeText(cursor);
  ASSERT_EQ(blocks.size(), 1);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    ASSERT_EQ(paragraphNode->size(), 1);
    auto child = paragraphNode->childAt(0);
    ASSERT_EQ(child->type(), NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("a"));
  }
}
TEST(ParagraphEditTest, UpgradeToUl) {
  Editor editor;
  editor.loadText("-");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  editor.insertText(" ");
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto node = doc->m_root->childAt(0);
  ASSERT_EQ(node->type(), NodeType::ul);
  auto ulNode = (md::parser::UnorderedList*)node;
  ASSERT_EQ(ulNode->size(), 1);
  auto ulItem = ulNode->childAt(0);
  ASSERT_EQ(ulItem->type(), md::parser::NodeType::ul_item);
  auto ulItemNode = (md::parser::UnorderedListItem*)ulItem;
  ASSERT_EQ(ulItemNode->size(), 0);
}
TEST(ParagraphEditTest, UpgradeToCodeBlock) {
  Editor editor;
  editor.loadText("```");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);

  doc->insertReturn(cursor);
  {
    ASSERT_EQ(blocks.size(), 1);
    ASSERT_EQ(doc->m_root->size(), 1);
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    ASSERT_EQ(codeBlockNode->size(), 0);
  }
}
TEST(ParagraphEditTest, UpgradeToCodeBlockWithOtherText) {
  Editor editor;
  editor.loadText("```asdfasdf");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto coord = doc->moveCursorToBol(cursor.coord());
  doc->updateCursor(cursor, coord);
  coord = doc->moveCursorToRight(cursor.coord());
  doc->updateCursor(cursor, coord);
  coord = doc->moveCursorToRight(cursor.coord());
  doc->updateCursor(cursor, coord);
  coord = doc->moveCursorToRight(cursor.coord());
  doc->updateCursor(cursor, coord);
  ASSERT_EQ(cursor.coord().offset, 3);
  doc->insertReturn(cursor);
  ASSERT_EQ(blocks.size(), 2);
  ASSERT_EQ(doc->m_root->size(), 2);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    ASSERT_EQ(codeBlockNode->size(), 0);
  }
  {
    auto node = doc->m_root->childAt(1);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    ASSERT_EQ(paragraphNode->size(), 1);
    auto child = paragraphNode->childAt(0);
    ASSERT_EQ(child->type(), NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("asdfasdf"));
  }
}
TEST(UlEditTest, DegradeToParagraph) {
  Editor editor;
  editor.loadText("- ");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  doc->removeText(*editor.m_cursor);
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto node = doc->m_root->childAt(0);
  ASSERT_EQ(node->type(), NodeType::paragraph);
  auto paragraphNode = (md::parser::Paragraph*)node;
  ASSERT_EQ(paragraphNode->size(), 0);
}
TEST(UlEditTest, InsertSpace) {
  Editor editor;
  editor.loadText("- []");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  coord = doc->moveCursorToLeft(coord);
  doc->updateCursor(cursor, coord);
  doc->insertText(cursor, " ");

  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::ul);
    auto ulNode = (md::parser::UnorderedList*)node;
    ASSERT_EQ(ulNode->size(), 1);
    auto ulItem = ulNode->childAt(0);
    ASSERT_EQ(ulItem->type(), md::parser::NodeType::ul_item);
    auto ulItemNode = (md::parser::UnorderedListItem*)ulItem;
    ASSERT_EQ(ulItemNode->size(), 1);
    auto child = ulItemNode->childAt(0);
    ASSERT_EQ(child->type(), NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("[ ]"));
  }
}
TEST(UlEditTest, UpgradeToCheckbox) {
  Editor editor;
  editor.loadText("- ");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  editor.insertText("[ ]");
  editor.insertText(" ");
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto node = doc->m_root->childAt(0);
  ASSERT_EQ(node->type(), NodeType::checkbox);
  auto ulNode = (md::parser::UnorderedList*)node;
  ASSERT_EQ(ulNode->size(), 1);
  auto ulItem = ulNode->childAt(0);
  ASSERT_EQ(ulItem->type(), md::parser::NodeType::checkbox_item);
  auto ulItemNode = (md::parser::UnorderedListItem*)ulItem;
  ASSERT_EQ(ulItemNode->size(), 0);
}
TEST(UlEditTest, InsertReturn) {
  Editor editor;
  editor.loadText(R"(
- GIF啊
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  {
    ASSERT_EQ(blocks.size(), 1);
    ASSERT_EQ(doc->m_root->size(), 1);
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::ul);
    auto ulNode = (md::parser::UnorderedList*)node;
    ASSERT_EQ(ulNode->size(), 1);
    auto ulItem = ulNode->childAt(0);
    ASSERT_EQ(ulItem->type(), NodeType::ul_item);
    auto ulItemNode = (md::parser::UnorderedListItem*)ulItem;
    ASSERT_EQ(ulItemNode->size(), 1);
    auto text = ulItemNode->childAt(0);
    ASSERT_EQ(text->type(), NodeType::text);
    auto textNode = (md::parser::Text*)text;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("GIF啊"));
    auto& line = blocks[0].logicalLineAt(0);
    ASSERT_EQ(line.length(), 4);
  }
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  doc->insertReturn(cursor);
  {
    ASSERT_EQ(blocks.size(), 1);
    ASSERT_EQ(doc->m_root->size(), 1);
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::ul);
    auto ulNode = (md::parser::UnorderedList*)node;
    ASSERT_EQ(ulNode->size(), 2);
    auto ulItem = ulNode->childAt(0);
    ASSERT_EQ(ulItem->type(), NodeType::ul_item);
    auto ulItemNode = (md::parser::UnorderedListItem*)ulItem;
    ASSERT_EQ(ulItemNode->size(), 1);
    auto text = ulItemNode->childAt(0);
    ASSERT_EQ(text->type(), NodeType::text);
    auto textNode = (md::parser::Text*)text;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("GIF啊"));
    auto& line = blocks[0].logicalLineAt(0);
    ASSERT_EQ(line.length(), 4);
  }
}
TEST(CheckboxEditTest, DegradeToParagraph) {
  Editor editor;
  editor.loadText("- [ ] ");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  ASSERT_EQ(blocks[0].countOfLogicalLine(), 1);
  ASSERT_EQ(blocks[0].logicalLineAt(0).m_cells.size(), 0);
  auto coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  doc->removeText(*editor.m_cursor);
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto node = doc->m_root->childAt(0);
  ASSERT_EQ(node->type(), NodeType::paragraph);
  auto paragraphNode = (md::parser::Paragraph*)node;
  ASSERT_EQ(paragraphNode->size(), 0);
}
TEST(CheckboxEditTest, DegradeToParagraph2) {
  Editor editor;
  editor.loadText("- [ ] 666");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  ASSERT_EQ(blocks[0].countOfLogicalLine(), 1);
  ASSERT_EQ(blocks[0].logicalLineAt(0).m_cells.size(), 1);
  auto coord = doc->moveCursorToBeginOfDocument();
  doc->updateCursor(cursor, coord);
  doc->removeText(*editor.m_cursor);
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto node = doc->m_root->childAt(0);
  ASSERT_EQ(node->type(), NodeType::paragraph);
  auto paragraphNode = (md::parser::Paragraph*)node;
  ASSERT_EQ(paragraphNode->size(), 1);
}

TEST(CheckboxEditTest, EmptyCheckBoxInsertReturn) {
  Editor editor;
  editor.loadText(R"(
- [ ] a
- [ ] b
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::checkbox);
  }
  CursorCoord coord;
  coord = doc->moveCursorToEndOfDocument();
  doc->updateCursor(cursor, coord);
  doc->removeText(cursor);
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::checkbox);
  }
  doc->insertReturn(cursor);
  ASSERT_EQ(blocks.size(), 2);
  ASSERT_EQ(doc->m_root->size(), 2);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::checkbox);
  }
  {
    auto node = doc->m_root->childAt(1);
    ASSERT_EQ(node->type(), md::parser::NodeType::paragraph);
  }
}
TEST(EditorTest, HeaderReturn) {
  Editor editor;
  editor.loadText("");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  editor.insertText("#");
  editor.insertText(" ");
  editor.insertText("asdfljsaf");
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::header);
  }
  doc->insertReturn(cursor);
  ASSERT_EQ(blocks.size(), 2);
  ASSERT_EQ(doc->m_root->size(), 2);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::header);
  }
  {
    auto node = doc->m_root->childAt(1);
    ASSERT_EQ(node->type(), md::parser::NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    ASSERT_EQ(paragraphNode->size(), 0);
  }
}

TEST(CodeBlockEditTest, InsertText) {
  Editor editor;
  editor.loadText(R"(
```
```
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    ASSERT_EQ(codeBlockNode->size(), 0);
  }
  doc->insertText(cursor, "a");
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    ASSERT_EQ(codeBlockNode->size(), 1);
    auto child = codeBlockNode->childAt(0);
    ASSERT_EQ(child->type(), md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("a"));
  }
  doc->insertReturn(cursor);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    ASSERT_EQ(codeBlockNode->size(), 2);
    auto child = codeBlockNode->childAt(0);
    ASSERT_EQ(child->type(), md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("a"));
    ASSERT_EQ(blocks[0].countOfLogicalLine(), 2);
  }
  doc->insertText(cursor, "b");
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    ASSERT_EQ(codeBlockNode->size(), 2);
    ASSERT_EQ(blocks[0].countOfLogicalLine(), 2);
    {
      auto child = codeBlockNode->childAt(0);
      ASSERT_EQ(child->type(), md::parser::NodeType::text);
      auto textNode = (md::parser::Text*)child;
      auto s = textNode->toString(doc.get());
      ASSERT_EQ(s, QString("a"));
    }
    {
      auto child = codeBlockNode->childAt(1);
      ASSERT_EQ(child->type(), md::parser::NodeType::text);
      auto textNode = (md::parser::Text*)child;
      auto s = textNode->toString(doc.get());
      ASSERT_EQ(s, QString("b"));
    }
  }
  doc->removeText(cursor);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    ASSERT_EQ(codeBlockNode->size(), 2);
    auto child = codeBlockNode->childAt(0);
    ASSERT_EQ(child->type(), md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("a"));
    ASSERT_EQ(blocks[0].countOfLogicalLine(), 2);
  }
  doc->removeText(cursor);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), md::parser::NodeType::code_block);
    auto codeBlockNode = (md::parser::CodeBlock*)node;
    ASSERT_EQ(codeBlockNode->size(), 1);
    auto child = codeBlockNode->childAt(0);
    ASSERT_EQ(child->type(), md::parser::NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("a"));
    ASSERT_EQ(blocks[0].countOfLogicalLine(), 1);
  }
}

TEST(CursorMoveTest, Emoji) {
  Editor editor;
  editor.loadText(R"(
a😊b
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  ASSERT_EQ(cursor.coord().offset, 0);
  CursorCoord coord;
  coord = doc->moveCursorToRight(cursor.coord());
  doc->updateCursor(cursor, coord);
  ASSERT_EQ(cursor.coord().offset, 1);
  coord = doc->moveCursorToRight(cursor.coord());
  doc->updateCursor(cursor, coord);
  ASSERT_EQ(cursor.coord().offset, 3);
  coord = doc->moveCursorToRight(cursor.coord());
  doc->updateCursor(cursor, coord);
  ASSERT_EQ(cursor.coord().offset, 4);
  coord = doc->moveCursorToLeft(cursor.coord());
  doc->updateCursor(cursor, coord);
  ASSERT_EQ(cursor.coord().offset, 3);
  coord = doc->moveCursorToLeft(cursor.coord());
  doc->updateCursor(cursor, coord);
  ASSERT_EQ(cursor.coord().offset, 1);
  coord = doc->moveCursorToLeft(cursor.coord());
  doc->updateCursor(cursor, coord);
  ASSERT_EQ(cursor.coord().offset, 0);
}
TEST(PreeditTest, ShowPreedit) {
  Editor editor;
  editor.loadText(R"()");
  editor.setPreedit("a");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  ASSERT_EQ(cursor.coord().offset, 1);
  ASSERT_EQ(blocks[0].logicalLineAt(0).length(), 1);
  auto node = doc->m_root->childAt(0);
  ASSERT_EQ(node->type(), NodeType::paragraph);
  auto paragraphNode = (md::parser::Paragraph*)node;
  ASSERT_EQ(paragraphNode->size(), 1);
  editor.setPreedit("ab");
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  ASSERT_EQ(cursor.coord().offset, 2);
  ASSERT_EQ(paragraphNode->size(), 1);
  ASSERT_EQ(blocks[0].logicalLineAt(0).length(), 2);
  editor.setPreedit("abc");
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  ASSERT_EQ(cursor.coord().offset, 3);
  ASSERT_EQ(paragraphNode->size(), 1);
  ASSERT_EQ(blocks[0].logicalLineAt(0).length(), 3);
}

TEST(PreeditTest, ShowPreedit2) {
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
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  ASSERT_EQ(cursor.coord().offset, 0);
  ASSERT_EQ(blocks[0].logicalLineAt(0).length(), 0);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::header);
    auto header = (md::parser::Header*)node;
    ASSERT_EQ(header->size(), 0);
  }
  editor.setPreedit("a");
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::header);
    auto header = (md::parser::Header*)node;
    ASSERT_EQ(header->size(), 1);
    ASSERT_EQ(header->childAt(0)->type(), NodeType::text);
    auto textNode = (md::parser::Text*)header->childAt(0);
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("a"));
  }
}

TEST(MultiBlockEditTest, RemoveEmptyParagraph) {
  Editor editor;
  editor.loadText(R"(
# a
c
# b
)");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  auto& cursor = *editor.m_cursor;

  ASSERT_EQ(blocks.size(), 3);
  ASSERT_EQ(doc->m_root->size(), 3);
  auto coord = cursor.coord();
  coord.blockNo = 1;
  coord.lineNo = 0;
  coord.offset = 1;
  doc->updateCursor(cursor, coord);
  doc->removeText(cursor);
  doc->removeText(cursor);
  ASSERT_EQ(blocks.size(), 2);
  ASSERT_EQ(doc->m_root->size(), 2);
  coord = cursor.coord();
  ASSERT_EQ(coord.blockNo, 0);
  ASSERT_EQ(coord.lineNo, 0);
  ASSERT_EQ(coord.offset, 1);
}
TEST(UndoTest, InsertReturn) {
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
  ASSERT_EQ(blocks.size(), 2);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    ASSERT_EQ(paragraphNode->size(), 1);
    auto child = paragraphNode->childAt(0);
    ASSERT_EQ(child->type(), NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("a"));
  }
  {
    auto node = doc->m_root->childAt(1);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    ASSERT_EQ(paragraphNode->size(), 1);
    auto child = paragraphNode->childAt(0);
    ASSERT_EQ(child->type(), NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("b"));
  }
  doc->undo(cursor);
  ASSERT_EQ(blocks.size(), 1);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    ASSERT_EQ(paragraphNode->size(), 1);
    auto child = paragraphNode->childAt(0);
    ASSERT_EQ(child->type(), NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("ab"));
  }
  {
    auto coord = doc->moveCursorToEndOfDocument();
    doc->updateCursor(cursor, coord);
  }
  doc->insertReturn(cursor);
  doc->undo(cursor);
  ASSERT_EQ(blocks.size(), 1);
  {
    auto node = doc->m_root->childAt(0);
    ASSERT_EQ(node->type(), NodeType::paragraph);
    auto paragraphNode = (md::parser::Paragraph*)node;
    ASSERT_EQ(paragraphNode->size(), 1);
    auto child = paragraphNode->childAt(0);
    ASSERT_EQ(child->type(), NodeType::text);
    auto textNode = (md::parser::Text*)child;
    auto s = textNode->toString(doc.get());
    ASSERT_EQ(s, QString("ab"));
  }
}
int main(int argc, char** argv) {
  // 必须加这一句
  // 不然调用字体(QFontMetric)时会崩溃
  QGuiApplication app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}