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
Document::Document(const String& str, sptr<RenderSetting> setting) : parser::Document(str), m_setting(setting) {
  for (auto node : m_root->children()) {
    m_blocks.append(Render::render(node, m_setting, this));
  }
}
void Document::updateCursor(Cursor& cursor) {
  auto coord = cursor.coord();
  int y = m_setting->docMargin.top();
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
      if (cell->isStaticTextCell()) {
        int x = cell->pos().x();
        auto staticCell = (StaticTextCell*)cell;
        pos.setX(x + staticCell->width());
      }
      pos.setY(pos.y() + y);
      cursor.setPos(pos);
      const QFontMetrics& fm = QFontMetrics(cell->font());
      cursor.setHeight(fm.height());
      return;
    }
    auto pos = cursor.pos();
    auto x = line.x();
    // 当逻辑行为空时，必须累加前面都逻辑行的高度
    for (int i = 0; i < coord.lineNo; ++i) {
      ASSERT(i >= 0 && i < block.logicalLines().size());
      y += block.logicalLines()[i].height();
    }
    pos.setX(x);
    pos.setY(y);
    cursor.setPos(pos);
    cursor.setHeight(line.height());
    return;
  }
  SizeType totalOffset = 0;
  for (int i = 0; i < coord.cellNo; ++i) {
    auto cell = line[i];
    if (!cell->isTextCell()) continue;
    auto textCell = (TextCell*)cell;
    totalOffset += textCell->length();
  }
  // 如果逻辑行为空的话，往往是没有文本可以画的
  //

  // 找到光标所在cell
  // 接下来计算光标到具体位置
  ASSERT(coord.cellNo >= 0 && coord.cellNo < line.size());
  auto cell = line[coord.cellNo];
  auto textCell = (TextCell*)cell;
  auto str = textCell->text()->toString(this).mid(textCell->offset());
  auto lenOfString = coord.offset - totalOffset;
  DEBUG << str << lenOfString;
  ASSERT(str.size() >= lenOfString);
  if (lenOfString < str.size()) {
    auto ch = str[lenOfString].unicode();
    // emoji检测
    // /\uD83C[\uDF00-\uDFFF]|\uD83D[\uDC00-\uDE4F]/g;
    if (ch == 0xd83d || ch == 0xd83c) {
    }
    if ((ch >= 0xdf00 && ch <= 0xdfff) || (ch >= 0xdc00 && ch <= 0xde4f)) {
      lenOfString++;
    }
  }
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
    // 判断emoji
    String s;
    for (auto cell : line) {
      if (!cell->isTextCell()) continue;
      auto textCell = (TextCell*)cell;
      s += textCell->text()->toString(this);
      if (s.size() > coord.offset + 1) break;
    }
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
  cursor.setCoord(coord);
  updateCursorCellNo(cursor);
}
void Document::moveCursorToLeft(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  if (coord.offset > 0) {
    if (coord.offset >= 2) {
      // 判断emoji
      String s;
      auto& line = block.logicalLines()[coord.lineNo];
      for (auto cell : line) {
        if (!cell->isTextCell()) continue;
        auto textCell = (TextCell*)cell;
        s += textCell->text()->toString(this);
        if (s.size() > coord.offset) break;
      }
      auto ch = s[coord.offset - 2].unicode();
      // 如果是emoji的开始标志，再往前移动一位
      if (ch == 0xd83d || ch == 0xd83c) {
        coord.offset--;
      }
    }
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
void Document::moveCursorToPos(Cursor& cursor, Point pos) {
  DEBUG << pos;
  // 如果没有block，就是第一个位置
  if (m_blocks.empty()) {
    auto coord = cursor.coord();
    coord.blockNo = 0;
    coord.lineNo = 0;
    coord.cellNo = 0;
    coord.offset = 0;
    cursor.setCoord(coord);
    return;
  }
  // 先找到哪个block
  SizeType blockNo = 0;
  int y = m_setting->docMargin.top();
  // 如果是点在文档上方的空白，也放到第一个位置
  if (pos.y() <= y) {
    auto coord = cursor.coord();
    coord.blockNo = 0;
    coord.lineNo = 0;
    coord.cellNo = 0;
    coord.offset = 0;
    cursor.setCoord(coord);
    return;
  }
  bool findBlock = false;
  while (blockNo < m_blocks.size()) {
    int h = m_blocks[blockNo].height();
    if (pos.y() >= y && y + h >= pos.y()) {
      DEBUG << y << "~" << y + h << pos;
      findBlock = true;
      break;
    }
    y += h;
    blockNo++;
  }
  if (!findBlock) {
    // 如果是在内容范围之外，直接去到最后一行，最后一列
    moveCursorToEndOfDocument(cursor);
    return;
  }
  // 找到block以后，遍历每一个逻辑行
  auto block = m_blocks[blockNo];
  if (block.countOfLogicalLine() == 0) {
    // 如果没有逻辑行，就是0，0，0
    auto coord = cursor.coord();
    coord.blockNo = blockNo;
    coord.lineNo = 0;
    coord.cellNo = 0;
    coord.offset = 0;
    cursor.setCoord(coord);
    return;
  }
  for (int lineNo = 0; lineNo < block.countOfLogicalLine(); ++lineNo) {
    auto line = block.logicalLines()[lineNo];
    // 如果当前逻辑行是空
    if (line.empty()) {
      // 判定y是不是在这个区间里
      QFontMetrics fm(m_setting->zhTextFont);
      auto h = fm.height();
      if (y <= pos.y() && pos.y() <= y + h) {
        // 在这行
        auto coord = cursor.coord();
        coord.blockNo = blockNo;
        coord.lineNo = lineNo;
        coord.cellNo = 0;
        coord.offset = 0;
        cursor.setCoord(coord);
        DEBUG << "empty logical line";
        return;
      }
      y += h;
      continue;
    }
    // 由于没有办法知道逻辑行的高度，先算出每个视觉行的高度
    std::vector<int> hs;
    std::vector<SizeType> startIndex;
    int curMaxH = 0;
    for (SizeType i = 0; i < line.size(); ++i) {
      auto cell = line[i];
      curMaxH = std::max(curMaxH, cell->height());
      if (cell->eol()) {
        hs.push_back(curMaxH);
        curMaxH = 0;
      }
      if (cell->bol()) {
        startIndex.push_back(i);
      }
    }
    ASSERT(!line.empty());
    if (!line.back()->eol()) {
      hs.push_back(curMaxH);
    }
    DEBUG << hs.size() << startIndex.size();
    {
      int totalH = 0;
      for (auto h : hs) {
        totalH += h;
      }
      DEBUG << y << "~" << y + totalH << pos;
    }
    ASSERT(hs.size() == startIndex.size());
    for (SizeType visualLineNo = 0; visualLineNo < hs.size(); ++visualLineNo) {
      DEBUG << y << "~" << y + hs[visualLineNo] << pos;
      if (y <= pos.y() && pos.y() <= y + hs[visualLineNo]) {
        // 找到所在视觉行
        ASSERT(visualLineNo >= 0 && visualLineNo < startIndex.size());
        SizeType start = startIndex[visualLineNo];
        SizeType end = visualLineNo + 1 < startIndex.size() ? startIndex[visualLineNo + 1] : line.size();
        for (SizeType cellNo = start; cellNo < end; ++cellNo) {
          // 确定x所在cell
          auto cell = line[cellNo];
          auto textCell = (TextCell*)cell;
          auto x = cell->pos().x();
          DEBUG << pos << x;
          // 如果比第一个cell的x都小，则是第一个cell的偏移量
          if (cellNo == start && pos.x() <= x) {
            auto coord = cursor.coord();
            coord.blockNo = blockNo;
            coord.lineNo = lineNo;
            coord.cellNo = cellNo;
            coord.offset = textCell->offset();
            cursor.setCoord(coord);
            return;
          }
          auto w = cell->width(this);
          if (x <= pos.x() && pos.x() <= x + w) {
            // 确定了是哪个cell
            // 接下来确定偏移量
            auto needW = pos.x() - x;
            auto s = textCell->toString(this);
            DEBUG << s;
            QFontMetrics fm(textCell->font());
            for (int k = 0; k < s.size() - 1; ++k) {
              auto w1 = fm.horizontalAdvance(s.left(k));
              auto w2 = fm.horizontalAdvance(s.left(k + 1));
              if (w1 <= needW && needW <= w2) {
                // 找到了字符偏移量
                auto coord = cursor.coord();
                coord.blockNo = blockNo;
                coord.lineNo = lineNo;
                coord.cellNo = cellNo;
                coord.offset = textCell->offset() + k;
                // 如果点击在中间靠后的部分，则偏移加1
                if (needW >= w1 + (w2 - w1) / 2) {
                  coord.offset++;
                }
                if (coord.offset - textCell->offset() > 0) {
                  auto index = coord.offset - textCell->offset() - 1;
                  // 如果当前偏移量切分了emoji，往后移动一位
                  auto ch = s[index].unicode();
                  if (ch == 0xd83d || ch == 0xd83c) {
                    coord.offset++;
                  }
                }
                cursor.setCoord(coord);
                return;
              }
            }
          }
          // 如果x比最后一个还大，则取最后一个
          if (cellNo == end - 1 && pos.x() >= x + w) {
            auto coord = cursor.coord();
            coord.blockNo = blockNo;
            coord.lineNo = lineNo;
            coord.cellNo = cellNo;
            coord.offset = textCell->offset() + textCell->length();
            cursor.setCoord(coord);
            return;
          }
        }
      }
      y += hs[visualLineNo];
    }
  }
  DEBUG << "fail data" << y << pos;
  ASSERT(false && "update coord fail");
}
void Document::insertText(Cursor& cursor, const String& text) {
  if (text.isEmpty()) return;
  String textToInsert = text;
  if (text == "(") {
    textToInsert = "()";
  } else if (text == "[") {
    textToInsert = "[]";
  } else if (text == "{") {
    textToInsert = "{}";
  }
  // 找到Text结点
  // 找到当前Cell相对Text结点的偏移量
  // 修改Text结点的PieceTable
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto line = block.logicalLines()[coord.lineNo];
  // 对cell个数为0的情况特殊处理
  // 新建的文档（空文本）为这个情况
  if (line.empty()) {
    auto node = m_root->children()[coord.blockNo];
    auto c = node2container(node);
    auto offset = m_addBuffer.size();
    auto length = textToInsert.size();
    m_addBuffer.append(textToInsert);
    Text* newTextNode = new Text(PieceTableItem::add, offset, length);
    // ol特殊处理
    if (node->type() == NodeType::ol || node->type() == NodeType::ul || node->type() == NodeType::checkbox) {
      auto listNode = node2container(node);
      ASSERT(coord.lineNo >= 0 && coord.lineNo < listNode->children().size());
      auto listItem = listNode->children()[coord.lineNo];
      auto listItemNode = node2container(listItem);
      listItemNode->appendChild(newTextNode);
    } else if (node->type() == NodeType::code_block) {
      DEBUG << coord;
      if (c->size() > coord.lineNo) {
        c->removeChildAt(coord.lineNo);
      }
      c->insertChild(coord.lineNo, newTextNode);
    } else {
      c->appendChild(newTextNode);
    }
    coord.offset += text.size();
    cursor.setCoord(coord);
    renderBlock(coord.blockNo);
    return;
  }
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
        if (!prefix.isEmpty() && prefix.count('#') == prefix.size()) {
          // 说明是header
          // 将段落转换为标题
          auto header = new Header(prefix.size());
          for (auto i = coord.cellNo; i < paragraphNode->children().size(); ++i) {
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
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            auto parentNode = textNode->parent();
            ASSERT(parentNode == header);
            bool ok = header->children().removeOne(textNode);
            ASSERT(ok && "delete empty text node fail");
          }
          replaceBlock(coord.blockNo, header);
          delete paragraphNode;
          coord.offset = 0;
          coord.cellNo = 0;
          cursor.setCoord(coord);
          return;
        } else if (prefix == "-") {
          // 说明是无序列表
          // 将段落转换为无序列表
          auto ul = new UnorderedList();
          auto ulItem = new UnorderedListItem();
          ul->appendChild(ulItem);
          for (auto i = coord.cellNo; i < paragraphNode->children().size(); ++i) {
            ulItem->appendChild(paragraphNode->children()[i]);
          }
          // 对第一个结点单独处理一下，需要去掉-
          auto cell = line[coord.cellNo];
          ASSERT(cell->isTextCell());
          auto textCell = (TextCell*)cell;
          auto textNode = textCell->text();
          textNode->remove(0, 1);
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            auto parentNode = textNode->parent();
            ASSERT(parentNode == ulItem);
            bool ok = ulItem->children().removeOne(textNode);
            ASSERT(ok && "delete empty text node fail");
          }
          replaceBlock(coord.blockNo, ul);
          delete paragraphNode;
          coord.offset = 0;
          coord.cellNo = 0;
          cursor.setCoord(coord);
          return;
        } else if (prefix == "1.") {
          // 说明是有序列表
          // 将段落转换为有序列表
          auto ol = new OrderedList();
          auto olItem = new OrderedListItem();
          ol->appendChild(olItem);
          for (auto i = coord.cellNo; i < paragraphNode->children().size(); ++i) {
            olItem->appendChild(paragraphNode->children()[i]);
          }
          // 对第一个结点单独处理一下，需要去掉-
          auto cell = line[coord.cellNo];
          ASSERT(cell->isTextCell());
          auto textCell = (TextCell*)cell;
          auto textNode = textCell->text();
          textNode->remove(0, 2);
          replaceBlock(coord.blockNo, ol);
          delete paragraphNode;
          coord.offset = 0;
          coord.cellNo = 0;
          cursor.setCoord(coord);
          return;
        }
      } else {
        // 大于6的就不可能变成标题了
      }
    } else if (parentNode->type() == NodeType::ul_item) {
      auto ppNode = parentNode->parent();
      ASSERT(ppNode->parent() == m_root.get() && "node hierarchy error");
      // 无序列表，行首输入[ ]空格，需要将无序列表转成checkbox
      auto listNode = node2container(ppNode);
      auto listItem = listNode->childAt(coord.lineNo);
      auto listItemNode = node2container(listItem);
      String s;
      for (int i = 0; i < std::min(coord.lineNo + 1, line.size()); ++i) {
        ASSERT(i >= 0 && i < line.size());
        if (!line[i]->isTextCell()) continue;
        s += ((TextCell*)line[i])->text()->toString(this);
      }

      auto prefix = s.left(coord.offset);
      if (prefix == "[ ]" || prefix == "[x]") {
        // 转换为checkbox
        // 需要考虑是在第一个，中间，还是最后一个，三种情况
        auto checkbox = new CheckboxList();
        auto checkboxItem = new CheckboxItem();
        checkboxItem->setChecked(prefix == "[x]");
        checkbox->appendChild(checkboxItem);
        SizeType leftLength = 3;
        for (auto node : listItemNode->children()) {
          if (node->type() == NodeType::text) {
            auto textNode = (Text*)node;
            auto s = textNode->toString(this);
            if (s.length() <= leftLength) {
              leftLength -= s.length();
              continue;
            } else {
              if (leftLength != 0) {
                textNode->remove(0, leftLength);
              }
              leftLength = 0;
            }
          }
          checkboxItem->appendChild(node);
        }
        if (coord.lineNo == 0) {
          insertBlock(coord.blockNo, checkbox);
          listNode->children().removeAt(coord.lineNo);
          if (listNode->children().empty()) {
            removeBlock(coord.blockNo + 1);
          }
        } else if (coord.lineNo == block.countOfLogicalLine()) {
          insertBlock(coord.blockNo + 1, checkbox);
          listNode->children().removeAt(coord.lineNo);
        } else {
          auto ul = new UnorderedList();
          for (auto i = coord.lineNo + 1; i < listNode->children().size(); ++i) {
            ul->appendChild(listNode->childAt(i));
          }
          insertBlock(coord.blockNo + 1, ul);
          for (auto i = listNode->children().size() - 1; i >= coord.lineNo; --i) {
            listNode->children().removeAt(i);
          }
          insertBlock(coord.blockNo + 1, checkbox);
          renderBlock(coord.blockNo);
          coord.blockNo++;
          coord.lineNo = 0;
        }
        coord.cellNo = 0;
        coord.offset = 0;
        cursor.setCoord(coord);
        return;
      }
    }
  }
  auto leftOffset = coord.offset - totalOffset;
  if (text == ")" || text == "]" || text == "}") {
    auto s = textNode->toString(this);
    ASSERT(leftOffset >= 0 && leftOffset < s.size());
    auto ch = s[leftOffset];
    if (ch == text) {
      coord.offset++;
      cursor.setCoord(coord);
      return;
    }
  }
  PieceTableItem item{PieceTableItem::add, m_addBuffer.size(), textToInsert.size()};
  m_addBuffer.append(textToInsert);
  textNode->insert(textCell->offset() + leftOffset, item);
  renderBlock(coord.blockNo);
  coord.offset += text.size();
  cursor.setCoord(coord);
  // updateCursorCellNo(cursor);
}
void Document::removeText(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto line = block.logicalLines()[coord.lineNo];
  if (line.empty()) {
    if (block.countOfLogicalLine() == 1 && coord.lineNo == 0) {
      auto blockNo = coord.blockNo;
      auto node = m_root->children()[blockNo];
      if (node->type() == NodeType::header) {
        DEBUG << "degrade header to paragraph";
        auto header = (Header*)node;
        replaceBlock(coord.blockNo, new Paragraph());
        delete header;
      } else if (node->type() == NodeType::ul || node->type() == NodeType::ol || node->type() == NodeType::checkbox) {
        auto listNode = node2container(node);
        ASSERT(listNode->size() == 1);
        replaceBlock(coord.blockNo, new Paragraph());
        delete listNode;
      } else {
#if 0
        // 删除当前block，光标左移动
        moveCursorToLeft(cursor);
        removeBlock(blockNo);
        return;
#endif
#if 1
        if (block.maxOffsetOfLogicalLine(0) == 0) {
          // 删除当前block，光标左移动
          moveCursorToLeft(cursor);
          removeBlock(blockNo);
        }
        return;
#endif
      }
    }
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_root->children().size());
    auto node = m_root->children()[coord.blockNo];
    if (node->type() == NodeType::ol || node->type() == NodeType::ul || node->type() == NodeType::checkbox) {
      auto olNode = node2container(node);
      ASSERT(coord.lineNo >= 0 && coord.lineNo < olNode->children().size());
      auto olItem = olNode->children()[coord.lineNo];
      // 合并这两个ol item
      if (coord.lineNo == 0) {
        // 如果是第一个ol item，降级为paragraph
        auto olItemNode = node2container(olItem);
        auto paragraph = new Paragraph();
        paragraph->appendChildren(olItemNode->children());
        insertBlock(coord.blockNo, paragraph);
        olNode->children().removeAt(0);
        renderBlock(coord.blockNo + 1);
        return;
      } else {
        // 其他情况就是合并两个ol item
        coord.offset = block.maxOffsetOfLogicalLine(coord.lineNo - 1);
        auto prevItem = olNode->children()[coord.lineNo - 1];
        auto curItem = olNode->children()[coord.lineNo];
        auto prevItemNode = node2container(prevItem);
        auto curItemNode = node2container(curItem);
        prevItemNode->appendChildren(curItemNode->children());
        olNode->children().removeAt(coord.lineNo);
        coord.lineNo--;
        cursor.setCoord(coord);
        renderBlock(coord.blockNo);
        delete curItemNode;
        return;
      }
    } else if (node->type() == NodeType::code_block) {
      auto codeBlockNode = node2container(node);
      if (coord.lineNo > 0) {
        coord.offset = block.maxOffsetOfLogicalLine(coord.lineNo - 1);
        auto text1 = codeBlockNode->childAt(coord.lineNo - 1);
        ASSERT(text1->type() == NodeType::text);
        auto text1Node = (Text*)text1;
        auto text2 = codeBlockNode->childAt(coord.lineNo);
        ASSERT(text2->type() == NodeType::text);
        auto text2Node = (Text*)text2;
        text1Node->merge(*text2Node);
        codeBlockNode->removeChildAt(coord.lineNo);
        coord.lineNo--;
        cursor.setCoord(coord);
        renderBlock(coord.blockNo);
        return;
      }
    }
    DEBUG << "line is empty";
    return;
  }
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
    ASSERT(false || "maybe not support now");
    return;
  }
  if (leftOffset - 1 > 0) {
    auto s = textNode->toString(this).mid(textCell->offset());
    auto ch = s[leftOffset - 2].unicode();
    // 如果是emoji的开始标志，两个都要删除
    if (ch == 0xd83d || ch == 0xd83c) {
      textNode->remove(textCell->offset() + leftOffset - 2, 2);
      coord.offset -= 2;
    } else {
      textNode->remove(textCell->offset() + leftOffset - 1, 1);
      coord.offset--;
    }
  } else {
    textNode->remove(textCell->offset() + leftOffset - 1, 1);
    coord.offset--;
  }
  cursor.setCoord(coord);
  updateCursorCellNo(cursor);
  renderBlock(coord.blockNo);
}
void Document::insertReturn(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto line = block.logicalLines()[coord.lineNo];
  if (line.empty()) {
    auto newBlock = new Paragraph();
    insertBlock(coord.blockNo + 1, newBlock);
    coord.blockNo++;
    coord.cellNo = 0;
    coord.offset = 0;
    cursor.setCoord(coord);
    return;
  }
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
    // 如果光标位于标题的行尾，则新建的block是段落
    // 否则还是标题
    if (rightTextNode->toString(this).isEmpty()) {
      newBlock = new Paragraph();
    } else {
      newBlock = new Header(header->level());
    }
  } else if (node->type() == NodeType::paragraph) {
    // 段落按回车可以触发升级到代码块
    originalBlock = (Paragraph*)node;
    String prefix;
    for (auto cell : line) {
      if (!cell->isTextCell()) continue;
      auto textCell = (TextCell*)cell;
      auto s = textCell->toString(this);
      prefix += s;
      if (prefix.size() > 3) break;
    }
    if (prefix.startsWith("```")) {
      leftTextNode->remove(0, 3);
      oldBlock = new CodeBlock(leftTextNode);
      if (rightTextNode->empty()) {
        replaceBlock(coord.blockNo, oldBlock);
        coord.cellNo = 0;
        coord.offset = 0;
        cursor.setCoord(coord);
        delete originalBlock;
        return;
      }
      newBlock = new Paragraph();
    } else {
      oldBlock = new Paragraph();
      newBlock = new Paragraph();
    }
  } else if (node->type() == NodeType::ol || node->type() == NodeType::ul || node->type() == NodeType::checkbox) {
    // 有序列表或无序列表
    auto listNode = node2container(node);
    SizeType listIndex = 0;
    SizeType itemIndex = 0;
    for (auto child : listNode->children()) {
      ASSERT(child->type() == NodeType::ol_item || child->type() == NodeType::ul_item ||
             child->type() == NodeType::checkbox_item);
      auto item = (Container*)child;
      if (textNode->parent() == child) {
        itemIndex = item->children().indexOf(textNode);
        break;
      }
      listIndex++;
    }
    // 考虑文本为空时，降级为段落的情况

    auto originalItem = (Container*)listNode->children()[listIndex];
    Container* oldItem = nullptr;
    Container* newItem = nullptr;
    if (node->type() == NodeType::ol) {
      oldItem = new OrderedListItem();
      newItem = new OrderedListItem();
    } else if (node->type() == NodeType::ul) {
      oldItem = new UnorderedListItem();
      newItem = new UnorderedListItem();
    } else if (node->type() == NodeType::checkbox) {
      auto oldItemTmp = new CheckboxItem();
      auto originalItemTmp = (CheckboxItem*)originalItem;
      oldItemTmp->setChecked(originalItemTmp->isChecked());
      oldItem = oldItemTmp;
      newItem = new CheckboxItem();
    }
    ASSERT(oldItem != nullptr && newItem != nullptr);
    for (int i = 0; i < itemIndex; ++i) {
      ASSERT(i < originalItem->children().size());
      oldItem->appendChild(originalItem->children()[i]);
    }
    oldItem->appendChild(leftTextNode);
    newItem->appendChild(rightTextNode);
    for (SizeType i = itemIndex + 1; i < originalItem->children().size(); ++i) {
      ASSERT(i < originalItem->children().size());
      newItem->appendChild(originalItem->children()[i]);
    }
    listNode->setChild(listIndex, oldItem);
    listNode->insertChild(listIndex + 1, newItem);
    renderBlock(coord.blockNo);
    coord.lineNo++;
    coord.cellNo = 0;
    coord.offset = 0;
    cursor.setCoord(coord);
    delete originalItem;
    return;
  } else if (node->type() == NodeType::code_block) {
    auto codeBlockNode = node2container(node);
    codeBlockNode->removeChildAt(coord.lineNo);
    codeBlockNode->insertChild(coord.lineNo, leftTextNode);
    codeBlockNode->insertChild(coord.lineNo + 1, rightTextNode);
    renderBlock(coord.blockNo);
    coord.lineNo++;
    coord.cellNo = 0;
    coord.offset = 0;
    cursor.setCoord(coord);
    return;
  } else {
    DEBUG << "not support now";
    ASSERT(false && "not support now");
    return;
  }
  ASSERT(originalBlock != nullptr);
  ASSERT(oldBlock != nullptr);
  ASSERT(newBlock != nullptr);
  for (int i = 0; i < coord.cellNo; ++i) {
    oldBlock->appendChild(originalBlock->children()[i]);
  }
  if (!leftTextNode->empty()) {
    oldBlock->appendChild(leftTextNode);
  }
  if (!rightTextNode->empty()) {
    newBlock->appendChild(rightTextNode);
  }
  for (SizeType i = coord.cellNo + 1; i < originalBlock->children().size(); ++i) {
    DEBUG << i;
    NodePtr& child = originalBlock->children()[i];
    newBlock->appendChild(child);
    DEBUG << child->type();
    if (child->type() == NodeType::text) {
      auto textNode = (Text*)child;
      auto s = textNode->toString(this);
      DEBUG << s;
    }
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
  m_blocks[blockNo] = Render::render(node, m_setting, this);
}
void Document::insertBlock(SizeType blockNo, parser::Node* node) {
  ASSERT(blockNo >= 0 && blockNo <= m_root->children().size());
  ASSERT(node != nullptr);
  m_root->insertChild(blockNo, node);
  m_blocks.insert(blockNo, Render::render(node, m_setting, this));
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
  if (line.empty()) {
    coord.cellNo = 0;
    cursor.setCoord(coord);
    return;
  }
  if (coord.offset == 0) {
    coord.cellNo = 0;
    cursor.setCoord(coord);
    return;
  }
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
void Document::removeBlock(SizeType blockNo) {
  ASSERT(blockNo >= 0 && blockNo < m_blocks.size());
  if (m_blocks.size() == 1) {
    // 如果只剩一个block，就保留
    return;
  }
  m_blocks.removeAt(blockNo);
  m_root->children().removeAt(blockNo);
}
void Document::moveCursorToEndOfDocument(Cursor& cursor) {
  auto coord = cursor.coord();
  coord.blockNo = m_blocks.size() - 1;
  if (m_blocks.empty()) {
    coord.lineNo = 0;
    coord.cellNo = 0;
    coord.offset = 0;
  } else {
    coord.lineNo = m_blocks[coord.blockNo].countOfLogicalLine() - 1;
    // 如果没有逻辑行
    if (coord.lineNo == -1) {
      coord.lineNo = 0;
      coord.cellNo = 0;
      coord.offset = 0;
    } else {
      coord.offset = m_blocks[coord.blockNo].maxOffsetOfLogicalLine(coord.lineNo);
      cursor.setCoord(coord);
    }
  }
  updateCursorCellNo(cursor);
}
}  // namespace md::editor
