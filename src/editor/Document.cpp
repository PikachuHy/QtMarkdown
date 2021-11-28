//
// Created by PikachuHy on 2021/11/5.
//

#include "Document.h"

#include "Cursor.h"
#include "debug.h"
#include "parser/Parser.h"
#include "parser/Text.h"
#include "render/Render.h"
#include "Command.h"
using namespace md::parser;
using namespace md::render;
namespace md::editor {
Document::Document(const String& str, sptr<RenderSetting> setting)
    : parser::Document(str), m_setting(setting), m_commandStack(std::make_shared<CommandStack>()) {
  for (auto node : m_root->children()) {
    const Block& block = Render::render(node, m_setting, this);
    m_blocks.push_back(block);
  }
}
void Document::updateCursor(Cursor& cursor, const CursorCoord& coord, bool updatePos) {
  cursor.setCoord(coord);
  if (!updatePos) return;
  auto [pos, h] = mapToScreen(coord);
  cursor.setPos(pos);
  cursor.setHeight(h);
}

std::pair<Point, int> Document::mapToScreen(const CursorCoord& coord) {
  int y = m_setting->docMargin.top();
  for (int blockNo = 0; blockNo < coord.blockNo; ++blockNo) {
    y += m_blocks[blockNo].height();
  }
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto line = block.logicalLineAt(coord.lineNo);
  auto [pos, h] = line.cursorAt(coord.offset, this);
  if (h == line.height()) {
    h -= m_setting->lineSpacing;
  }
  return {pos + Point(0, y), h};
}
CursorCoord Document::moveCursorToRight(CursorCoord coord) {
  auto block = m_blocks[coord.blockNo];
  auto& line = block.logicalLineAt(coord.lineNo);
  SizeType totalOffset = line.length();
  if (totalOffset >= coord.offset + 1) {
    // 判断emoji
    String s = line.left(coord.offset + 1, this);
    auto ch = s[coord.offset].unicode();
    // 如果是emoji的开始标志，再往后移动一位
    if (ch == 0xd83d || ch == 0xd83c) {
      coord.offset++;
    }
    coord.offset++;
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
  auto block = m_blocks[coord.blockNo];
  if (coord.offset > 0) {
    if (coord.offset >= 2) {
      // 判断emoji
      auto& line = block.logicalLineAt(coord.lineNo);
      String s = line.left(coord.offset, this);
      auto ch = s[coord.offset - 2].unicode();
      // 如果是emoji的开始标志，再往前移动一位
      if (ch == 0xd83d || ch == 0xd83c) {
        coord.offset--;
      }
    }
    coord.offset--;
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
CursorCoord Document::moveCursorToUp(CursorCoord coord, Point pos) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  int x = pos.x();
  if (line.canMoveUp(coord.offset, this)) {
    coord.offset = line.moveUp(coord.offset, x, this);
  } else {
    if (coord.lineNo > 0) {
      coord.lineNo--;
      coord.offset = block.logicalLineAt(coord.lineNo).moveToX(x, this, true);
    } else if (coord.blockNo > 0) {
      coord.blockNo--;
      coord.lineNo = m_blocks[coord.blockNo].countOfLogicalLine() - 1;
      coord.offset = m_blocks[coord.blockNo].logicalLineAt(coord.lineNo).moveToX(x, this, true);
    } else {
      coord.blockNo = 0;
      coord.lineNo = 0;
      coord.offset = 0;
    }
  }
  return coord;
}
CursorCoord Document::moveCursorToDown(CursorCoord coord, Point pos) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  int x = pos.x();
  if (line.canMoveDown(coord.offset, this)) {
    DEBUG << "move down";
    coord.offset = line.moveDown(coord.offset, x, this);
  } else {
    if (coord.lineNo + 1 < block.countOfLogicalLine()) {
      coord.lineNo++;
      coord.offset = block.logicalLineAt(coord.lineNo).moveToX(x, this);
    } else if (coord.blockNo + 1 < m_blocks.size()) {
      coord.blockNo++;
      coord.lineNo = 0;
      coord.offset = m_blocks[coord.blockNo].logicalLineAt(coord.lineNo).moveToX(x, this);
    } else {
      coord.offset = line.length();
    }
  }
  return coord;
}
CursorCoord Document::moveCursorToPos(Point pos) {
  // 先找到哪个block
  SizeType blockNo = 0;
  int y = m_setting->docMargin.top();
  // 如果是点在文档上方的空白，也放到第一个位置
  if (pos.y() <= y) {
    CursorCoord coord;
    coord.blockNo = 0;
    coord.lineNo = 0;
    coord.offset = 0;
    return coord;
  }
  bool findBlock = false;
  while (blockNo < m_blocks.size()) {
    int h = m_blocks[blockNo].height();
    if (pos.y() >= y && y + h >= pos.y()) {
      findBlock = true;
      break;
    }
    y += h;
    blockNo++;
  }
  if (!findBlock) {
    // 如果是在内容范围之外，直接去到最后一行，最后一列
    return moveCursorToEndOfDocument();
  }
  // 找到block以后，遍历每一个逻辑行
  auto block = m_blocks[blockNo];
  if (block.countOfLogicalLine() == 0) {
    // 如果没有逻辑行，就是0，0，0
    CursorCoord coord;
    coord.blockNo = blockNo;
    coord.lineNo = 0;
    coord.offset = 0;
    return coord;
  }
  auto node = m_root->childAt(blockNo);
  if (node->type() == NodeType::ul) {
    for (int i = 0; i < block.countOfLogicalLine(); ++i) {
      auto line = block.logicalLineAt(i);
      DEBUG << line.height();
    }
  }
  auto oldY = y;
  for (int lineNo = 0; lineNo < block.countOfLogicalLine(); ++lineNo) {
    auto& line = block.logicalLineAt(lineNo);
    if (y <= pos.y() && pos.y() <= y + line.height()) {
      CursorCoord coord;
      coord.blockNo = blockNo;
      coord.lineNo = lineNo;
      coord.offset = line.offsetAt(Point(pos.x(), pos.y() - oldY), this, m_setting->lineSpacing);
      return coord;
    }
    y += line.height();
  }
  DEBUG << "not handle";
  ASSERT(false && "not handle");
}
void Document::insertText(Cursor& cursor, const String& text) {
  if (text.isEmpty()) return;
  auto command = new InsertTextCommand(this, cursor.coord(), text);
  command->execute(cursor);
  m_commandStack->push(command);
}
void Document::removeText(Cursor& cursor) {
  auto command = new RemoveTextCommand(this, cursor.coord());
  command->execute(cursor);
  m_commandStack->push(command);
}
void Document::insertReturn(Cursor& cursor) {
  auto command = new InsertReturnCommand(this, cursor.coord());
  command->execute(cursor);
  m_commandStack->push(command);
}
void Document::replaceBlock(SizeType blockNo, parser::Node* node) {
  ASSERT(blockNo >= 0 && blockNo < m_root->children().size());
  ASSERT(node != nullptr);
  m_root->setChild(blockNo, node);
  m_blocks[blockNo] = Render::render(node, m_setting, this);
}
void Document::insertBlock(SizeType blockNo, parser::Node* node) {
  ASSERT(blockNo >= 0 && blockNo <= m_root->children().size());
  ASSERT(node != nullptr);
  m_root->insertChild(blockNo, node);
  m_blocks.insert(m_blocks.begin() + blockNo, Render::render(node, m_setting, this));
}
void Document::renderBlock(SizeType blockNo) {
  ASSERT(blockNo >= 0 && blockNo < m_root->children().size());
  m_blocks[blockNo] = Render::render(m_root->children()[blockNo], m_setting, this);
}
void Document::mergeBlock(SizeType blockNo1, SizeType blockNo2) {
  ASSERT(blockNo1 >= 0 && blockNo1 < m_root->children().size());
  ASSERT(blockNo2 >= 0 && blockNo2 < m_root->children().size());
  auto node1 = node2container(m_root->children()[blockNo1]);
  auto node2 = node2container(m_root->children()[blockNo2]);
  // 需要对Text结点进行合并
  for (auto child : node2->children()) {
    if (node1->children().empty()) {
      node1->appendChild(child);
      continue;
    }
    if (node1->children().back()->type() == NodeType::text && child->type() == NodeType::text) {
      auto node = node1->children().back();
      auto textNode1 = (Text*)node;
      auto textNode2 = (Text*)child;
      textNode1->merge(*textNode2);
    } else {
      node1->appendChild(child);
    }
  }
  m_root->removeChildAt(blockNo2);
  m_blocks.erase(m_blocks.begin() + blockNo2);
  replaceBlock(blockNo1, node1);
}
parser::Container* Document::node2container(parser::Node* node) {
  ASSERT(node != nullptr);
  if (node->type() == NodeType::header) {
    return (Header*)node;
  }
  if (node->type() == NodeType::paragraph) {
    return (Paragraph*)node;
  }
  if (node->type() == NodeType::ol) {
    return (OrderedList*)node;
  }
  if (node->type() == NodeType::ol_item) {
    return (OrderedListItem*)node;
  }
  if (node->type() == NodeType::ul) {
    return (UnorderedList*)node;
  }
  if (node->type() == NodeType::ul_item) {
    return (UnorderedListItem*)node;
  }
  if (node->type() == NodeType::checkbox_item) {
    return (CheckboxItem*)node;
  }
  if (node->type() == NodeType::checkbox) {
    return (CheckboxList*)node;
  }
  if (node->type() == NodeType::code_block) {
    return (CodeBlock*)node;
  }
  DEBUG << node->type();
  ASSERT(false && "node convert not support");
  return nullptr;
}
CursorCoord Document::moveCursorToBol(CursorCoord coord) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  coord.offset = line.moveToBol(coord.offset, this);
  return coord;
}
std::pair<CursorCoord, int> Document::moveCursorToEol(CursorCoord coord) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  auto [offset, x] = line.moveToEol(coord.offset, this);
  coord.offset = offset;
  return {coord, x};
}
void Document::removeBlock(SizeType blockNo) {
  ASSERT(blockNo >= 0 && blockNo < m_blocks.size());
  m_blocks.erase(m_blocks.begin() + blockNo);
  m_root->children().erase(m_root->children().begin() + blockNo);
}
CursorCoord Document::moveCursorToEndOfDocument() {
  CursorCoord coord;
  ASSERT(!m_blocks.empty());
  coord.blockNo = m_blocks.size() - 1;
  auto block = m_blocks[coord.blockNo];
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
  return line.isBol(coord.offset, (DocPtr)this);
}
void Document::undo(Cursor& cursor) { m_commandStack->undo(cursor); }
void Document::redo(Cursor& cursor) {}
void Document::upgradeToHeader(const Cursor& cursor, int level) {
  ASSERT(level >= 1 && level <= 6);
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_root->size());
  auto node = m_root->childAt(coord.blockNo);
  if (node->type() != NodeType::paragraph) return;
  auto paragraphNode = (Paragraph*)node;
  auto header = new Header(level);
  header->setChildren(paragraphNode->children());
  replaceBlock(coord.blockNo, header);
}
}  // namespace md::editor
