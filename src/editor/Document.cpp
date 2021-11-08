//
// Created by PikachuHy on 2021/11/5.
//

#include "Document.h"

#include "Cursor.h"
#include "debug.h"
#include "parser/Parser.h"
#include "parser/Text.h"
#include "render/Render.h"
using namespace md::parser;
using namespace md::render;
namespace md::editor {
Document::Document(const String& str) : parser::Document(str) {
  for (auto node : m_root->children()) {
    m_blocks.append(Render::render(node, this));
  }
}
void Document::updateCursor(Cursor& cursor) {
  auto coord = cursor.coord();
  DEBUG << "cursor coord:" << coord;
  int y = 0;
  for (int blockNo = 0; blockNo < coord.blockNo; ++blockNo) {
    y += m_blocks[blockNo].height();
  }
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto line = block.logicalLines()[coord.lineNo];
  if (coord.offset == 0) {
    if (!line.empty()) {
      auto cell = line.front();
      auto pos = cell->pos();
      pos.setY(pos.y() + y);
      cursor.setPos(pos);
      const QFontMetrics& fm = QFontMetrics(cell->font());
      cursor.setHeight(fm.height());
      return;
    }
    DEBUG << "not update cursor for line empty";
    return;
  }
  SizeType totalOffset = 0;
  for (int i = 0; i < coord.cellNo; ++i) {
    auto cell = line[i];
    if (!cell->isTextCell()) continue;
    auto textCell = (TextCell*)cell;
    totalOffset += textCell->length();
  }
  // 找到光标所在cell
  // 接下来计算光标到具体位置
  auto lenOfString = coord.offset - totalOffset;
  DEBUG << "left str len:" << lenOfString;
  auto cell = line[coord.cellNo];
  auto textCell = (TextCell*)cell;
  auto str = textCell->text()->toString(this);
  ASSERT(str.size() >= lenOfString);
  str = str.left(lenOfString);
  const QFontMetrics& fm = QFontMetrics(cell->font());
  auto w = fm.horizontalAdvance(str);
  // 最后光标到位置是
  auto pos = textCell->pos();
  pos.setX(pos.x() + w);
  pos.setY(pos.y() + y);
  cursor.setPos(pos);
  cursor.setHeight(fm.height());
}
void Document::moveCursorToRight(Cursor& cursor) {
  auto coord = cursor.coord();
  auto block = m_blocks[coord.blockNo];
  auto& line = block.logicalLines()[coord.lineNo];
  SizeType totalOffset = 0;
  for (auto cell : line) {
    if (!cell->isTextCell()) continue;
    auto textCell = (TextCell*)cell;
    totalOffset += textCell->length();
  }
  if (totalOffset >= coord.offset + 1) {
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
  cursor.setCoord(coord);
  updateCursorCellNo(cursor);
}
void Document::moveCursorToLeft(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  if (coord.offset > 0) {
    coord.offset--;
  } else if (coord.lineNo > 0) {
    coord.lineNo--;
    coord.offset = block.maxOffsetOfLogicalLine(coord.lineNo);
  } else if (coord.blockNo > 0) {
    coord.blockNo--;
    coord.lineNo = m_blocks[coord.blockNo].countOfLogicalLine() - 1;
    coord.offset = m_blocks[coord.blockNo].maxOffsetOfLogicalLine(coord.lineNo);
  } else {
    // do nothing
    DEBUG << "do nothing";
  }
  cursor.setCoord(coord);
  updateCursorCellNo(cursor);
}
void Document::moveCursorToUp(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto line = block.logicalLines()[coord.lineNo];
  ASSERT(coord.cellNo >= 0 && coord.cellNo < line.size());
  auto cell = line[coord.cellNo];

  if (coord.lineNo > 0) {
    coord.lineNo--;
    coord.offset = 0;
  } else if (coord.blockNo > 0) {
    coord.blockNo--;
    coord.lineNo = m_blocks[coord.blockNo].countOfLogicalLine() - 1;
    coord.offset = 0;
  } else {
    coord.blockNo = 0;
    coord.lineNo = 0;
    coord.offset = 0;
  }
  cursor.setCoord(coord);
  updateCursorCellNo(cursor);
}
void Document::moveCursorToDown(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto line = block.logicalLines()[coord.lineNo];
  ASSERT(coord.cellNo >= 0 && coord.cellNo < line.size());
  auto cell = line[coord.cellNo];
  if (coord.lineNo + 1 < block.countOfLogicalLine()) {
    coord.lineNo++;
    coord.offset = 0;
  } else if (coord.blockNo + 1 < m_blocks.size()) {
    coord.blockNo++;
    coord.lineNo = 0;
    coord.offset = 0;
  } else {
    coord.offset = block.maxOffsetOfLogicalLine(coord.lineNo);
  }
  cursor.setCoord(coord);
  updateCursorCellNo(cursor);
}
// FIXME: 计算itemNo有问题
void Document::moveCursorToPos(Cursor& cursor, Point pos) {
  int blockNo = 0;
  int y = 0;
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
    DEBUG << "not find block";
    return;
  }
  auto block = m_blocks[blockNo];
  // 从视觉行确定
  auto leftY = pos.y() - y;
  int lineNo = 0;
  bool findLine = false;
  y = 0;
  for (const auto& line : block.visualLines()) {
    int h = 0;
    for (auto item : line) {
      h = std::max(h, item->height());
    }
    if (leftY >= y && y + h >= leftY) {
      findLine = true;
      break;
    }
    lineNo++;
  }
  if (!findLine) {
    DEBUG << "not find line";
    return;
  }
  // 确定行内偏移
  auto line = block.visualLines()[lineNo];
  bool findItem = false;
  int itemNo = 0;
  for (auto item : line) {
    // 这里到pos.x应该先用offset修正
    if (item->containX(pos.x())) {
      findItem = true;
      break;
    }
    itemNo++;
  }
  if (!findItem) {
    // 没有找到分为两种情况，一个是在前面，一个是在行尾
    if (line.empty()) {
      updateCursorOffset(cursor, blockNo, lineNo, 0, 0);
    } else {
      if (pos.x() < line.front()->config().rect.x()) {
        updateCursorOffset(cursor, blockNo, lineNo, 0, 0);
      } else {
        updateCursorOffset(cursor, blockNo, lineNo, line.size() - 1, -1);
      }
    }
  } else {
    // 计算当前的坐标还有多少文本的偏移量
    ASSERT(itemNo >= 0 && itemNo < line.size());
    auto item = line[itemNo];
    auto textInstruction = static_cast<TextInstruction*>(item);
    auto str = textInstruction->textString(this);
    auto leftX = pos.x() - item->config().rect.x();
    if (leftX == 0) {
      updateCursorOffset(cursor, blockNo, lineNo, itemNo, 0);
    } else {
      QFontMetrics fm(item->config().font);
      int textOffset = 0;
      while (textOffset < str.size()) {
        auto lastW = fm.horizontalAdvance(str.left(textOffset));
        auto w = fm.horizontalAdvance(str.left(textOffset + 1));
        if (lastW <= leftX && leftX <= w) {
          // 再判定是不动，还是右移动一位
          if (lastW + (w - lastW) / 2 >= leftX) {
            textOffset++;
          }
          updateCursorOffset(cursor, blockNo, lineNo, itemNo, textOffset);
          break;
        }
        textOffset++;
      }
    }
  }
}
void Document::updateCursorOffset(Cursor& cursor, int blockNo, int lineNo, int itemNo, int textOffset) {
  DEBUG << blockNo << lineNo << itemNo << textOffset;
  auto coord = cursor.coord();
  coord.blockNo = blockNo;
  ASSERT(blockNo >= 0 && blockNo < m_blocks.size());
  auto block = m_blocks[blockNo];
  // 现在要根据视觉行算出逻辑行到偏移
  int curVisualLineNo = 0;
  SizeType offset = 0;
  for (int i = 0; i < block.logicalLines().size(); ++i) {
    const auto& line = block.logicalLines()[i];
    for (int j = 0; j < line.size(); ++j) {
      auto cell = line[j];
      // 暂时不考虑其他的文本Cell
      if (!cell->isTextCell()) continue;
      // 如果已经找到视觉行
      if (curVisualLineNo == lineNo) {
        for (int k = 0; k < itemNo; ++k) {
          auto itemCell = line[j + k];
          auto itemTextCell = static_cast<TextCell*>(itemCell);
          offset += itemTextCell->offset();
        }
        ASSERT(j + itemNo >= 0 && j + itemNo < line.size());
        auto itemCell = line[j + itemNo];
        auto itemTextCell = static_cast<TextCell*>(itemCell);
        // 如果textOffset是-1，表示最后一个
        if (textOffset == -1) {
          offset += itemTextCell->length();
        } else {
          offset += textOffset;
        }
        // 更新坐标
        coord.lineNo = i;
        coord.cellNo = j + itemNo;
        coord.offset = offset;
        cursor.setCoord(coord);
        DEBUG << "update cursor coord to" << blockNo << i << offset;
        return;
      } else {
        if (cell->eol()) {
          curVisualLineNo++;
        }
        // 更新逻辑行内的偏移
        auto textCell = static_cast<TextCell*>(cell);
        offset += textCell->offset();
      }
    }
  }
}
void Document::insertText(Cursor& cursor, String text) {
  // 找到Text结点
  // 找到当前Cell相对Text结点的偏移量
  // 修改Text结点的PieceTable
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto line = block.logicalLines()[coord.lineNo];
  ASSERT(coord.cellNo >= 0 && coord.cellNo < line.size());
  auto cell = line[coord.cellNo];
  if (!cell->isTextCell()) return;
  auto textCell = (TextCell*)cell;
  auto textNode = textCell->text();
  SizeType totalOffset = 0;
  for (int i = 0; i < coord.cellNo; ++i) {
    totalOffset += line[i]->length();
  }
  // 对按空格特殊处理
  // 如果是 # + 空格
  if (text == " ") {
    auto parentNode = textNode->parent();
    ASSERT(parentNode != nullptr);
    if (parentNode->type() == NodeType::paragraph) {
      auto ppNode = parentNode->parent();
      ASSERT(ppNode == m_root.get() && "node hierarchy error");
      auto paragraphNode = (Paragraph*)parentNode;
      if (coord.offset <= 6) {
        String s;
        for (int i = 0; i < coord.lineNo + 1; ++i) {
          if (!line[i]->isTextCell()) continue;
          s += ((TextCell*)line[i])->text()->toString(this);
        }
        auto prefix = s.left(coord.offset);
        if (prefix.count('#') == prefix.size()) {
          // 说明是header
          // 将段落转换为标题
          auto header = new Header(prefix.size());
          for (int i = coord.cellNo; i < paragraphNode->children().size(); ++i) {
            header->appendChild(paragraphNode->children()[i]);
          }
          // 对第一个结点单独处理一下，需要去掉#
          auto cell = line[coord.cellNo];
          ASSERT(cell->isTextCell());
          auto textCell = (TextCell*)cell;
          auto textNode = textCell->text();
          auto str = textNode->toString(this);
          int countOfSharp = 0;
          for (auto ch : str) {
            if (ch == '#') {
              countOfSharp++;
            } else {
              break;
            }
          }
          textNode->remove(0, countOfSharp);
          replaceBlock(coord.blockNo, header);
          m_blocks[coord.blockNo] = Render::render(header, this);
          delete paragraphNode;
          coord.offset = 0;
          coord.cellNo = 0;
          cursor.setCoord(coord);
          return;
        }
      } else {
        // 大于6的就不可能变成标题了
      }
    }
  }
  auto leftOffset = coord.offset - totalOffset;
  DEBUG << leftOffset;
  PieceTableItem item{PieceTableItem::add, m_addBuffer.size(), text.size()};
  m_addBuffer.append(text);
  textNode->insert(textCell->offset() + leftOffset, item);
  renderBlock(coord.blockNo);
  coord.offset += text.size();
  cursor.setCoord(coord);
}
void Document::removeText(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto line = block.logicalLines()[coord.lineNo];
  ASSERT(coord.cellNo >= 0 && coord.cellNo < line.size());
  auto cell = line[coord.cellNo];
  if (!cell->isTextCell()) return;
  auto textCell = (TextCell*)cell;
  auto textNode = textCell->text();
  SizeType totalOffset = 0;
  for (int i = 0; i < coord.cellNo; ++i) {
    totalOffset += line[i]->length();
  }
  auto leftOffset = coord.offset - totalOffset;
  if (leftOffset <= 0) {
    // 在行首删除，
    // 如果是标题，降级为段落
    auto parentNode = textNode->parent();
    ASSERT(parentNode != nullptr);
    if (parentNode->type() == NodeType::header) {
      auto ppNode = parentNode->parent();
      ASSERT(ppNode == m_root.get() && "node hierarchy error");
      auto header = (Header*)parentNode;
      auto newParagraph = new Paragraph();
      newParagraph->setChildren(header->children());
      replaceBlock(coord.blockNo, newParagraph);
      delete header;
      DEBUG << "degrade header to paragraph";
      return;
    }
    // 考虑段首按删除的情况
    if (coord.lineNo == 0 && coord.cellNo == 0) {
      if (coord.blockNo > 0) {
        // 合并当前block和前一个block
        // 新坐标为前一个block的最后一行，最后一列
        auto prevBlock = m_blocks[coord.blockNo - 1];
        coord.lineNo = prevBlock.countOfLogicalLine() - 1;
        coord.offset = prevBlock.maxOffsetOfLogicalLine(prevBlock.countOfLogicalLine() - 1);
        mergeBlock(coord.blockNo - 1, coord.blockNo);
        coord.blockNo--;
        cursor.setCoord(coord);
        return;
      }
    }
    DEBUG << "no text to remove";
    return;
  }
  DEBUG << "remove" << leftOffset;
  DEBUG << "before" << textNode->toString(this);
  textNode->remove(leftOffset - 1, 1);
  DEBUG << "after" << textNode->toString(this);
  coord.offset--;
  cursor.setCoord(coord);
  renderBlock(coord.blockNo);
}
void Document::insertReturn(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto line = block.logicalLines()[coord.lineNo];
  ASSERT(coord.cellNo >= 0 && coord.cellNo < line.size());
  auto cell = line[coord.cellNo];
  if (!cell->isTextCell()) return;
  auto textCell = (TextCell*)cell;
  auto textNode = textCell->text();
  SizeType totalOffset = 0;
  for (int i = 0; i < coord.cellNo; ++i) {
    totalOffset += line[i]->length();
  }
  auto leftOffset = coord.offset - totalOffset;
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_root->children().size());
  auto node = m_root->children()[coord.blockNo];
  // 从当前光标的位置，把结点切分为同样的两个子结点
  auto [leftTextNode, rightTextNode] = textNode->split(leftOffset);
  Container* originalBlock = nullptr;
  Container* oldBlock = nullptr;
  Container* newBlock = nullptr;
  if (node->type() == NodeType::header) {
    auto header = (Header*)node;
    originalBlock = (Header*)node;
    oldBlock = new Header(header->level());
    newBlock = new Header(header->level());
  } else if (node->type() == NodeType::paragraph) {
    originalBlock = (Paragraph*)node;
    oldBlock = new Paragraph();
    newBlock = new Paragraph();
  } else {
    DEBUG << "not support now";
    return;
  }
  ASSERT(originalBlock != nullptr);
  ASSERT(oldBlock != nullptr);
  ASSERT(newBlock != nullptr);
  for (int i = 0; i < coord.cellNo; ++i) {
    oldBlock->appendChild(originalBlock->children()[i]);
  }
  oldBlock->appendChild(leftTextNode);
  newBlock->appendChild(rightTextNode);
  for (SizeType i = coord.cellNo + 1; i < originalBlock->children().size(); ++i) {
    newBlock->appendChild(originalBlock->children()[i]);
  }
  replaceBlock(coord.blockNo, oldBlock);
  insertBlock(coord.blockNo + 1, newBlock);
  coord.blockNo++;
  coord.cellNo = 0;
  coord.offset = 0;
  cursor.setCoord(coord);
  delete originalBlock;
}
void Document::replaceBlock(SizeType blockNo, parser::Node* node) {
  ASSERT(blockNo >= 0 && blockNo < m_root->children().size());
  ASSERT(node != nullptr);
  m_root->setChild(blockNo, node);
  m_blocks[blockNo] = Render::render(node, this);
}
void Document::insertBlock(SizeType blockNo, parser::Node* node) {
  ASSERT(blockNo >= 0 && blockNo < m_root->children().size());
  ASSERT(node != nullptr);
  m_root->insertChild(blockNo, node);
  m_blocks.insert(blockNo, Render::render(node, this));
}
void Document::renderBlock(SizeType blockNo) {
  ASSERT(blockNo >= 0 && blockNo < m_root->children().size());
  m_blocks[blockNo] = Render::render(m_root->children()[blockNo], this);
}
void Document::mergeBlock(SizeType blockNo1, SizeType blockNo2) {
  ASSERT(blockNo1 >= 0 && blockNo1 < m_root->children().size());
  ASSERT(blockNo2 >= 0 && blockNo2 < m_root->children().size());
  auto node1 = node2container(m_root->children()[blockNo1]);
  auto node2 = node2container(m_root->children()[blockNo2]);
  node1->appendChildren(node2->children());
  m_root->children().removeAt(blockNo2);
  m_blocks.removeAt(blockNo2);
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
  ASSERT(false && "node convert not support");
  return nullptr;
}
void Document::moveCursorToBol(Cursor& cursor) {
  auto coord = cursor.coord();
  coord.offset = 0;
  cursor.setCoord(coord);
  updateCursorCellNo(cursor);
}
void Document::moveCursorToEol(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto line = block.logicalLines()[coord.lineNo];
  ASSERT(coord.cellNo >= 0 && coord.cellNo < line.size());
  auto cell = line[coord.cellNo];
  coord.offset = block.maxOffsetOfLogicalLine(coord.lineNo);
  cursor.setCoord(coord);
  updateCursorCellNo(cursor);
}
void Document::updateCursorCellNo(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto line = block.logicalLines()[coord.lineNo];
  SizeType totalOffset = 0;
  for (int i = 0; i < line.size(); ++i) {
    auto cell = line[i];
    if (!cell->isTextCell()) continue;
    auto textCell = (TextCell*)cell;
    if (totalOffset <= coord.offset && coord.offset <= totalOffset + textCell->length()) {
      coord.cellNo = i;
      cursor.setCoord(coord);
      return;
    }
    totalOffset += textCell->length();
  }
  DEBUG << "cursor:" << cursor.coord();
  ASSERT(false && "update cursor cellNo fail");
}
}  // namespace md::editor