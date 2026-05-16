//
// Created by PikachuHy on 2021/11/5.
//

#include "Document.h"

#include "Cursor.h"
#include "core/Utf8Util.h"
#include "debug.h"
#include "parser/Parser.h"
#include "parser/Text.h"
#include "render/Render.h"
#include "Command.h"
#include "MarkdownSerializer.h"
using namespace md::parser;
using namespace md::render;
namespace md::editor {
Document::Document(const String& str, sptr<RenderSetting> setting, core::IImageProvider* imageProvider)
    : m_parserDoc(std::make_unique<parser::Document>(str)), m_setting(setting), m_commandStack(std::make_shared<CommandStack>()),
      m_imageProvider(imageProvider) {
  this->renderAllBlock();
}
void Document::assertBlocksInSync() {
  ASSERT(m_blocks.size() == m_parserDoc->root()->children().size());
}
#ifdef QT_DEBUG
static void assertBlockTextCellsValid(const render::Block& block) {
  for (const auto& logicalLine : block.lines()) {
    for (const auto* cell : logicalLine.cells()) {
      auto* textNode = cell->textNode();
      if (textNode) {
        ASSERT(textNode->type() == parser::NodeType::text);
        ASSERT(textNode->parent() != nullptr);
      }
    }
  }
}
#endif
void Document::ensureTrailingParagraph() {
  auto& children = m_parserDoc->root()->children();
  if (children.empty() || children.back()->type() != NodeType::paragraph) {
    auto paragraph = std::make_unique<Paragraph>();
    parser::Node* raw = paragraph.get();
    m_parserDoc->root()->appendChild(std::move(paragraph));
    m_blocks.push_back(Render::render(raw, m_setting, *m_parserDoc, nullptr, m_imageProvider));
  }
  assertBlocksInSync();
}
void Document::insertText(Cursor& cursor, const String& text) {
  if (text.isEmpty()) return;
  auto command = std::make_unique<InsertTextCommand>(this, cursor.coord(), text);
  command->execute(cursor);
  m_commandStack->push(std::move(command));
  ensureTrailingParagraph();
}
void Document::removeText(Cursor& cursor) {
  auto command = std::make_unique<RemoveTextCommand>(this, cursor.coord());
  command->execute(cursor);
  if (command->hasUndoAction()) {
    m_commandStack->push(std::move(command));
  }
  ensureTrailingParagraph();
}
void Document::insertReturn(Cursor& cursor) {
  auto command = std::make_unique<InsertReturnCommand>(this, cursor.coord());
  command->execute(cursor);
  m_commandStack->push(std::move(command));
  ensureTrailingParagraph();
}

void Document::renderAllBlock() {
  m_blocks.clear();
  for (auto& node : m_parserDoc->root()->children()) {
    Block block = Render::render(node.get(), m_setting, *m_parserDoc, nullptr, m_imageProvider);
    m_blocks.push_back(std::move(block));
  }
  ensureTrailingParagraph();
}
void Document::replaceBlock(SizeType blockNo, std::unique_ptr<parser::Node> node) {
  ASSERT(blockNo >= 0 && blockNo < m_parserDoc->root()->children().size());
  ASSERT(node != nullptr);
  auto* rawNode = node.get();
  m_parserDoc->root()->setChild(blockNo, std::move(node));
  m_blocks[blockNo] = Render::render(rawNode, m_setting, *m_parserDoc, nullptr, m_imageProvider);
  assertBlocksInSync();
#ifdef QT_DEBUG
  assertBlockTextCellsValid(m_blocks[blockNo]);
#endif
}
void Document::insertBlock(SizeType blockNo, std::unique_ptr<parser::Node> node) {
  ASSERT(blockNo >= 0 && blockNo <= m_parserDoc->root()->children().size());
  ASSERT(node != nullptr);
  auto* rawNode = node.get();
  m_parserDoc->root()->insertChild(blockNo, std::move(node));
  m_blocks.insert(m_blocks.begin() + blockNo, Render::render(rawNode, m_setting, *m_parserDoc, nullptr, m_imageProvider));
  assertBlocksInSync();
#ifdef QT_DEBUG
  assertBlockTextCellsValid(m_blocks[blockNo]);
#endif
}
void Document::renderBlock(SizeType blockNo) {
  ASSERT(blockNo >= 0 && blockNo < m_parserDoc->root()->children().size());
  m_blocks[blockNo] = Render::render(m_parserDoc->root()->children()[blockNo].get(), m_setting, *m_parserDoc, nullptr, m_imageProvider);
  assertBlocksInSync();
#ifdef QT_DEBUG
  assertBlockTextCellsValid(m_blocks[blockNo]);
#endif
}
void Document::mergeBlock(SizeType blockNo1, SizeType blockNo2) {
  ASSERT(blockNo1 >= 0 && blockNo1 < m_parserDoc->root()->children().size());
  ASSERT(blockNo2 >= 0 && blockNo2 < m_parserDoc->root()->children().size());
  auto node1 = m_parserDoc->root()->children()[blockNo1]->asContainer();
  auto node2 = m_parserDoc->root()->children()[blockNo2]->asContainer();
  ASSERT(node1);
  ASSERT(node2);
  if (!node1 || !node2) {
    DEBUG << "asContainer returned null -- skipping merge";
    return;
  }
  // 需要对Text结点进行合并
  for (auto& child : node2->children()) {
    if (node1->children().empty()) {
      node1->appendChild(std::move(child));
      continue;
    }
    if (node1->children().back()->type() == NodeType::text && child->type() == NodeType::text) {
      auto& node = node1->children().back();
      auto textNode1 = static_cast<Text*>(node.get());
      auto textNode2 = static_cast<Text*>(child.get());
      textNode1->merge(*textNode2);
    } else {
      node1->appendChild(std::move(child));
    }
  }
  m_parserDoc->root()->removeChildAt(blockNo2);
  m_blocks.erase(m_blocks.begin() + blockNo2);
  renderBlock(blockNo1);
  assertBlocksInSync();
}
void Document::removeBlock(SizeType blockNo) {
  ASSERT(blockNo >= 0 && blockNo < m_blocks.size());
  m_blocks.erase(m_blocks.begin() + blockNo);
  m_parserDoc->root()->children().erase(m_parserDoc->root()->children().begin() + blockNo);
  assertBlocksInSync();
}
void Document::undo(Cursor& cursor) { m_commandStack->undo(cursor);
  ensureTrailingParagraph(); }
void Document::redo(Cursor& cursor) {
  m_commandStack->redo(cursor);
  ensureTrailingParagraph();
}
void Document::upgradeToHeader(Cursor& cursor, int level) {
  ASSERT(level >= 1 && level <= 6);
  auto command = std::make_unique<UpgradeToHeaderCommand>(this, cursor.coord(), level);
  command->execute(cursor);
  m_commandStack->push(std::move(command));
  ensureTrailingParagraph();
}
void Document::removeTextRange(const CursorCoord& begin, const CursorCoord& end) {
  auto command = std::make_unique<RemoveTextRangeCommand>(this, begin, end);
  Cursor cursor;
  command->execute(cursor);
  if (command->hasUndoAction()) {
    m_commandStack->push(std::move(command));
  }
  ensureTrailingParagraph();
}
String Document::serializeBlock(SizeType blockNo) const {
  ASSERT(blockNo >= 0 && blockNo < m_parserDoc->root()->children().size());
  auto* node = m_parserDoc->root()->childAt(blockNo);
  MarkdownSerializer serializer(*m_parserDoc);
  node->accept(&serializer);
  return serializer.markdown();
}

Document::MarkdownPosition Document::cursorToMarkdownPosition(const CursorCoord& coord) const {
  MarkdownPosition result;
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());

  SizeType contentPos = 0;
  for (SizeType i = 0; i < coord.lineNo; ++i) {
    contentPos += block.logicalLineAt(i).length();
  }
  contentPos += coord.offset;

  auto* blockNode = m_parserDoc->root()->childAt(coord.blockNo);
  MarkdownSerializer serializer(*m_parserDoc);
  blockNode->accept(&serializer);
  result.text = serializer.markdown();
  const auto& posMap = serializer.contentToMarkdown();

  if (contentPos < posMap.size()) {
    result.pos = posMap[contentPos];
  } else if (!posMap.empty()) {
    result.pos = posMap.back() + 1;
  } else {
    result.pos = result.text.length();
    if (result.text.endsWith("\n\n")) result.pos -= 2;
    else if (result.text.endsWith("\n")) result.pos -= 1;
    auto nodeType = blockNode->type();
    if (nodeType == NodeType::code_block) {
      if (result.pos >= 3 && result.text.mid(result.pos - 3, 3) == "```") result.pos -= 3;
    } else if (nodeType == NodeType::latex_block) {
      if (result.pos >= 2 && result.text.mid(result.pos - 2, 2) == "$$") result.pos -= 2;
    }
  }
  return result;
}

CursorCoord Document::findCursorFromContentPosition(SizeType blockNo, SizeType contentPos) const {
  ASSERT(blockNo >= 0 && blockNo < m_blocks.size());
  const auto& block = m_blocks[blockNo];
  SizeType remaining = contentPos;
  for (SizeType i = 0; i < block.countOfLogicalLine(); ++i) {
    SizeType lineLen = block.logicalLineAt(i).length();
    if (remaining <= lineLen)
      return {blockNo, i, remaining};
    remaining -= lineLen;
  }
  SizeType lastLine = block.countOfLogicalLine() - 1;
  return {blockNo, lastLine, block.logicalLineAt(lastLine).length()};
}

void Document::replaceBlocksFromText(SizeType startBlockNo, SizeType endBlockNo,
                                     const String& editedMD, SizeType addOffset, SizeType addLength) {
  auto newRoot = Parser::parse(editedMD, PieceTableItem::add, addOffset);
  auto& newChildren = newRoot->children();
  SizeType newBlockCount = newChildren.size();

  if (newBlockCount == 1 && newChildren[0]->type() == NodeType::paragraph) {
    auto* p = static_cast<Paragraph*>(newChildren[0].get());
    if (p->children().empty() && (endBlockNo - startBlockNo > 1 || m_blocks.size() > 1)) {
      removeBlock(startBlockNo);
      return;
    }
  }

  auto& oldChildren = m_parserDoc->root()->children();
  SizeType removeCount = endBlockNo - startBlockNo;
  for (SizeType i = 0; i < removeCount && startBlockNo < oldChildren.size(); ++i) {
    oldChildren.erase(oldChildren.begin() + startBlockNo);
    m_blocks.erase(m_blocks.begin() + startBlockNo);
  }

  for (SizeType i = 0; i < newBlockCount; ++i) {
    auto* raw = newChildren[i].get();
    m_parserDoc->root()->insertChild(startBlockNo + i, std::move(newChildren[i]));
    m_blocks.insert(m_blocks.begin() + startBlockNo + i,
                    Render::render(raw, m_setting, *m_parserDoc, nullptr, m_imageProvider));
  }
  assertBlocksInSync();
}

}  // namespace md::editor
