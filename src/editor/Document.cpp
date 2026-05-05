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
      auto* textCell = dynamic_cast<const render::TextCell*>(cell);
      if (textCell) {
        auto* textNode = textCell->text();
        ASSERT(textNode != nullptr);
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
void Document::updateCursor(Cursor& cursor, const CursorCoord& coord, bool updatePos) {
  cursor.setCoord(coord);
  if (!updatePos) return;
  auto [pos, h] = mapToScreen(coord);
  cursor.setPos(pos);
  cursor.setHeight(h);
}
std::pair<core::Point, int> Document::mapToScreen(const CursorCoord& coord) {
  int y = m_setting->docMargin.top;
  for (int blockNo = 0; blockNo < coord.blockNo; ++blockNo) {
    y += m_setting->blockSpacing;
    y += m_blocks[blockNo].height();
  }
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  auto [pos, h] = line.cursorAt(coord.offset, *m_parserDoc);
  if (h == line.height()) {
    h -= m_setting->lineSpacing;
  }
  auto resultPos = pos + core::Point(0, y);
  return {resultPos, h};
}
CursorCoord Document::moveCursorToRight(CursorCoord coord) {
  const auto& block = m_blocks[coord.blockNo];
  auto& line = block.logicalLineAt(coord.lineNo);
  SizeType totalOffset = line.length();
  if (totalOffset >= coord.offset + 1) {
    // 获取当前字符的UTF-8序列长度
    String s = line.left(coord.offset + 1, *m_parserDoc);
    auto seqLen = ::md::utf8SequenceLength(s[coord.offset]);
    coord.offset += seqLen;
  } else {
    // 去下一个逻辑行
    if (coord.lineNo + 1 < block.countOfLogicalLine()) {
      coord.offset = 0;
      coord.lineNo++;
    } else {
      // 不然只能去下一个block了
      if (coord.blockNo + 1 < m_blocks.size()) {
        coord.blockNo++;
        coord.lineNo = 0;
        coord.offset = 0;
      }
    }
  }
  return coord;
}
CursorCoord Document::moveCursorToLeft(CursorCoord coord) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  if (coord.offset > 0) {
    auto& line = block.logicalLineAt(coord.lineNo);
    String s = line.left(coord.offset, *m_parserDoc);
    auto prevStart = ::md::previousCodePointStart(s.toStdString(), coord.offset);
    coord.offset = prevStart;
  } else if (coord.lineNo > 0) {
    coord.lineNo--;
    coord.offset = block.logicalLineAt(coord.lineNo).length();
  } else if (coord.blockNo > 0) {
    coord.blockNo--;
    coord.lineNo = m_blocks[coord.blockNo].countOfLogicalLine() - 1;
    coord.offset = m_blocks[coord.blockNo].logicalLineAt(coord.lineNo).length();
  } else {
    // do nothing
    DEBUG << "do nothing";
  }
  return coord;
}
CursorCoord Document::moveCursorToUp(CursorCoord coord, core::Point pos) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  int x = pos.x;
  if (line.canMoveUp(coord.offset, *m_parserDoc)) {
    coord.offset = line.moveUp(coord.offset, x, *m_parserDoc);
  } else {
    if (coord.lineNo > 0) {
      coord.lineNo--;
      coord.offset = block.logicalLineAt(coord.lineNo).moveToX(x, *m_parserDoc, true);
    } else if (coord.blockNo > 0) {
      coord.blockNo--;
      coord.lineNo = m_blocks[coord.blockNo].countOfLogicalLine() - 1;
      coord.offset = m_blocks[coord.blockNo].logicalLineAt(coord.lineNo).moveToX(x, *m_parserDoc, true);
    } else {
      coord.blockNo = 0;
      coord.lineNo = 0;
      coord.offset = 0;
    }
  }
  return coord;
}
CursorCoord Document::moveCursorToDown(CursorCoord coord, core::Point pos) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  int x = pos.x;
  if (line.canMoveDown(coord.offset, *m_parserDoc)) {
    DEBUG << "move down";
    coord.offset = line.moveDown(coord.offset, x, *m_parserDoc);
  } else {
    if (coord.lineNo + 1 < block.countOfLogicalLine()) {
      coord.lineNo++;
      coord.offset = block.logicalLineAt(coord.lineNo).moveToX(x, *m_parserDoc);
    } else if (coord.blockNo + 1 < m_blocks.size()) {
      coord.blockNo++;
      coord.lineNo = 0;
      coord.offset = m_blocks[coord.blockNo].logicalLineAt(coord.lineNo).moveToX(x, *m_parserDoc);
    } else {
      coord.offset = line.length();
    }
  }
  return coord;
}
CursorCoord Document::moveCursorToPos(core::Point pos) {
  // 先找到哪个block
  SizeType blockNo = 0;
  int y = m_setting->docMargin.top;
  // 如果是点在文档上方的空白，也放到第一个位置
  if (pos.y <= y) {
    CursorCoord coord;
    coord.blockNo = 0;
    coord.lineNo = 0;
    coord.offset = 0;
    return coord;
  }
  bool findBlock = false;
  while (blockNo < m_blocks.size()) {
    int h = m_blocks[blockNo].height();
    if (pos.y >= y && y + h >= pos.y) {
      findBlock = true;
      break;
    }
    y += h + m_setting->blockSpacing;
    blockNo++;
  }
  if (!findBlock) {
    // 如果是在内容范围之外，直接去到最后一行，最后一列
    return moveCursorToEndOfDocument();
  }
  // 找到block以后，遍历每一个逻辑行
  const auto& block = m_blocks[blockNo];
  if (block.countOfLogicalLine() == 0) {
    // 如果没有逻辑行，就是0，0，0
    CursorCoord coord;
    coord.blockNo = blockNo;
    coord.lineNo = 0;
    coord.offset = 0;
    return coord;
  }
  auto node = m_parserDoc->root()->childAt(blockNo);
  if (node->type() == NodeType::ul) {
    for (int i = 0; i < block.countOfLogicalLine(); ++i) {
      (void)block.logicalLineAt(i);
    }
  }
  auto oldY = y;
  for (int lineNo = 0; lineNo < block.countOfLogicalLine(); ++lineNo) {
    auto& line = block.logicalLineAt(lineNo);
    if (y <= pos.y && pos.y <= y + line.height()) {
      CursorCoord coord;
      coord.blockNo = blockNo;
      coord.lineNo = lineNo;
      coord.offset = line.offsetAt(core::Point(pos.x, pos.y - oldY), *m_parserDoc, m_setting->lineSpacing);
      return coord;
    }
    y += line.height();
  }
  DEBUG << "not handle -- falling back to end of document";
  ASSERT(false && "not handle");
  return moveCursorToEndOfDocument();
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
  auto node1 = node2container(m_parserDoc->root()->children()[blockNo1].get());
  auto node2 = node2container(m_parserDoc->root()->children()[blockNo2].get());
  ASSERT(node1);
  ASSERT(node2);
  if (!node1 || !node2) {
    DEBUG << "node2container returned null -- skipping merge";
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
parser::Container* Document::node2container(parser::Node* node) {
  ASSERT(node != nullptr);
  if (node->type() == NodeType::header) {
    return static_cast<Header*>(node);
  }
  if (node->type() == NodeType::paragraph) {
    return static_cast<Paragraph*>(node);
  }
  if (node->type() == NodeType::ol) {
    return static_cast<OrderedList*>(node);
  }
  if (node->type() == NodeType::ol_item) {
    return static_cast<OrderedListItem*>(node);
  }
  if (node->type() == NodeType::ul) {
    return static_cast<UnorderedList*>(node);
  }
  if (node->type() == NodeType::ul_item) {
    return static_cast<UnorderedListItem*>(node);
  }
  if (node->type() == NodeType::checkbox_item) {
    return static_cast<CheckboxItem*>(node);
  }
  if (node->type() == NodeType::checkbox) {
    return static_cast<CheckboxList*>(node);
  }
  if (node->type() == NodeType::code_block) {
    return static_cast<CodeBlock*>(node);
  }
  if (node->type() == NodeType::quote_block) {
    return static_cast<QuoteBlock*>(node);
  }
  DEBUG << node->type();
  ASSERT(false && "node convert not support");
  return nullptr;
}
CursorCoord Document::moveCursorToBol(CursorCoord coord) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  coord.offset = line.moveToBol(coord.offset, *m_parserDoc);
  return coord;
}
std::pair<CursorCoord, int> Document::moveCursorToEol(CursorCoord coord) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  auto [offset, x] = line.moveToEol(coord.offset, *m_parserDoc);
  coord.offset = offset;
  return {coord, x};
}
void Document::removeBlock(SizeType blockNo) {
  ASSERT(blockNo >= 0 && blockNo < m_blocks.size());
  m_blocks.erase(m_blocks.begin() + blockNo);
  m_parserDoc->root()->children().erase(m_parserDoc->root()->children().begin() + blockNo);
  assertBlocksInSync();
}
CursorCoord Document::moveCursorToEndOfDocument() {
  CursorCoord coord;
  ASSERT(!m_blocks.empty());
  coord.blockNo = m_blocks.size() - 1;
  // Skip trailing empty paragraphs so cursor lands on actual content
  while (coord.blockNo > 0) {
    const auto& block = m_blocks[coord.blockNo];
    if (block.countOfLogicalLine() == 1 && block.logicalLineAt(0).length() == 0 &&
        m_parserDoc->root()->childAt(coord.blockNo)->type() == NodeType::paragraph) {
      coord.blockNo--;
    } else {
      break;
    }
  }
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(block.countOfLogicalLine() > 0);
  coord.lineNo = block.countOfLogicalLine() - 1;
  coord.offset = block.logicalLineAt(coord.lineNo).length();
  return coord;
}
CursorCoord Document::moveCursorToBeginOfDocument() {
  CursorCoord coord;
  coord.blockNo = 0;
  coord.lineNo = 0;
  coord.offset = 0;
  return coord;
}
bool Document::isBol(const CursorCoord& coord) const {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  const auto& line = block.logicalLineAt(coord.lineNo);
  return line.isBol(coord.offset, *m_parserDoc);
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
}  // namespace md::editor
