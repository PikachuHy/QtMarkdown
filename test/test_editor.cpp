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
  doc->moveCursorToLeft(cursor);
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
  doc->moveCursorToEndOfDocument(cursor);
  ASSERT_EQ(cursor.coord().offset, 4);
  doc->removeText(cursor);
  ASSERT_EQ(cursor.coord().offset, 3);
  doc->removeText(cursor);
  ASSERT_EQ(cursor.coord().offset, 1);
  doc->removeText(cursor);
  ASSERT_EQ(cursor.coord().offset, 0);
}

TEST(ParagraphEditTest, UpgradeToHeader) {
  Editor editor;
  editor.loadText("#");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  doc->moveCursorToEndOfDocument(*editor.m_cursor);
  editor.insertText(" ");
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto node = doc->m_root->childAt(0);
  ASSERT_EQ(node->type(), NodeType::header);
  auto headerNode = (md::parser::Header*)node;
  ASSERT_EQ(headerNode->size(), 0);
}
TEST(ParagraphEditTest, UpgradeToUl) {
  Editor editor;
  editor.loadText("-");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  doc->moveCursorToEndOfDocument(*editor.m_cursor);
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
  doc->moveCursorToEndOfDocument(*editor.m_cursor);
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
  doc->moveCursorToBol(cursor);
  doc->moveCursorToRight(cursor);
  doc->moveCursorToRight(cursor);
  doc->moveCursorToRight(cursor);
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
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  doc->moveCursorToEndOfDocument(*editor.m_cursor);
  doc->removeText(*editor.m_cursor);
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto node = doc->m_root->childAt(0);
  ASSERT_EQ(node->type(), NodeType::paragraph);
  auto paragraphNode = (md::parser::Paragraph*)node;
  ASSERT_EQ(paragraphNode->size(), 0);
}
TEST(UlEditTest, UpgradeToCheckbox) {
  Editor editor;
  editor.loadText("- ");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  doc->moveCursorToEndOfDocument(*editor.m_cursor);
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
TEST(CheckboxEditTest, DegradeToParagraph) {
  Editor editor;
  editor.loadText("- [ ] ");
  auto doc = editor.document();
  auto& blocks = doc->m_blocks;
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  ASSERT_EQ(blocks[0].countOfLogicalLine(), 1);
  ASSERT_EQ(blocks[0].logicalLineAt(0).m_cells.size(), 0);
  doc->moveCursorToEndOfDocument(*editor.m_cursor);
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
  doc->moveCursorToBeginOfDocument(cursor);
  doc->removeText(*editor.m_cursor);
  ASSERT_EQ(blocks.size(), 1);
  ASSERT_EQ(doc->m_root->size(), 1);
  auto node = doc->m_root->childAt(0);
  ASSERT_EQ(node->type(), NodeType::paragraph);
  auto paragraphNode = (md::parser::Paragraph*)node;
  ASSERT_EQ(paragraphNode->size(), 1);
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
  doc->moveCursorToRight(cursor);
  ASSERT_EQ(cursor.coord().offset, 1);
  doc->moveCursorToRight(cursor);
  ASSERT_EQ(cursor.coord().offset, 3);
  doc->moveCursorToRight(cursor);
  ASSERT_EQ(cursor.coord().offset, 4);
  doc->moveCursorToLeft(cursor);
  ASSERT_EQ(cursor.coord().offset, 3);
  doc->moveCursorToLeft(cursor);
  ASSERT_EQ(cursor.coord().offset, 1);
  doc->moveCursorToLeft(cursor);
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
int main(int argc, char** argv) {
  // 必须加这一句
  // 不然调用字体(QFontMetric)时会崩溃
  QGuiApplication app(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}