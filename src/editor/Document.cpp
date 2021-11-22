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
class DocumentOperationVisitor {
 public:
  DocumentOperationVisitor(Cursor& cursor, Document* doc) : cursor(cursor), m_doc(doc) {
    coord = cursor.coord();
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
  }

 protected:
  void renderBlock(SizeType blockNo) { m_doc->renderBlock(blockNo); }
  void updateCursor(Cursor& cursor, const CursorCoord& coord) { m_doc->updateCursor(cursor, coord); }
  void insertBlock(SizeType blockNo, parser::Node* node) { m_doc->insertBlock(blockNo, node); }
  void removeBlock(SizeType blockNo) { m_doc->removeBlock(blockNo); }
  void replaceBlock(SizeType blockNo, parser::Node* node) { m_doc->replaceBlock(blockNo, node); }
  void mergeBlock(SizeType blockNo1, SizeType blockNo2) { m_doc->mergeBlock(blockNo1, blockNo2); }
  void moveCursorToLeft(Cursor& cursor) { m_doc->moveCursorToLeft(cursor); }

 protected:
  Document* m_doc;
  Cursor& cursor;
  CursorCoord coord;
};
class InsertReturnVisitor
    : public MultipleVisitor<Paragraph, Header, OrderedList, UnorderedList, CheckboxList, CodeBlock>,
      public DocumentOperationVisitor {
 public:
  InsertReturnVisitor(Cursor& cursor, Document* doc) : DocumentOperationVisitor(cursor, doc) {
    coord = cursor.coord();
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    auto textAndOffset = line.textAt(coord.offset);
    this->textNode = textAndOffset.first;
    this->leftOffset = textAndOffset.second;
    auto leftRightText = textNode->split(leftOffset);
    this->leftTextNode = leftRightText.first;
    this->rightTextNode = leftRightText.second;
  }
  void visit(Paragraph* node) override {
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);

    Container* oldBlock = nullptr;
    Container* newBlock = nullptr;
    String prefix = line.left(coord.offset, m_doc);
    if (prefix.startsWith("```")) {
      leftTextNode->remove(0, 3);
      oldBlock = new CodeBlock(leftTextNode);
      if (rightTextNode->empty()) {
        m_doc->replaceBlock(coord.blockNo, oldBlock);
        coord.offset = 0;
        updateCursor(cursor, coord);
        delete node;
        return;
      }
      newBlock = new Paragraph();
    } else {
      oldBlock = new Paragraph();
      newBlock = new Paragraph();
    }
    splitNode(node, oldBlock, newBlock);
  }
  void visit(Header* node) override {
    auto oldBlock = new Header(node->level());
    Container* newBlock;
    if (rightTextNode->toString(m_doc).isEmpty()) {
      newBlock = new Paragraph();
    } else {
      newBlock = new Header(node->level());
    }
    splitNode(node, oldBlock, newBlock);
  }
  void visit(OrderedList* node) override {
    auto [listIndex, itemIndex] = indexOfItem(node);
    auto originalItem = (OrderedListItem*)node->childAt(listIndex);
    auto oldItem = new OrderedListItem();
    auto newItem = new OrderedListItem();
    splitListNode(node, originalItem, oldItem, newItem, listIndex, itemIndex);
  }
  void visit(UnorderedList* node) override {
    auto [listIndex, itemIndex] = indexOfItem(node);
    auto originalItem = (UnorderedListItem*)node->childAt(listIndex);
    auto oldItem = new UnorderedListItem();
    auto newItem = new UnorderedListItem();
    splitListNode(node, originalItem, oldItem, newItem, listIndex, itemIndex);
  }
  void visit(CheckboxList* node) override {
    auto [listIndex, itemIndex] = indexOfItem(node);
    auto originalItem = (CheckboxItem*)node->childAt(listIndex);
    auto oldItem = new CheckboxItem();
    oldItem->setChecked(originalItem->isChecked());
    auto newItem = new CheckboxItem();
    splitListNode(node, originalItem, oldItem, newItem, listIndex, itemIndex);
  }
  void visit(CodeBlock* node) override {
    node->removeChildAt(coord.lineNo);
    node->insertChild(coord.lineNo, leftTextNode);
    node->insertChild(coord.lineNo + 1, rightTextNode);
    m_doc->renderBlock(coord.blockNo);
    coord.lineNo++;
    coord.offset = 0;
    updateCursor(cursor, coord);
  }

 private:
  void splitNode(Container* originalBlock, Container* oldBlock, Container* newBlock) {
    ASSERT(originalBlock != nullptr);
    ASSERT(oldBlock != nullptr);
    ASSERT(newBlock != nullptr);
    // 需要找到子结点的下标
    ASSERT(textNode->parent() == originalBlock);
    int childIndex = 0;
    while (childIndex < originalBlock->size()) {
      if (originalBlock->childAt(childIndex) == textNode) {
        break;
      }
      childIndex++;
    }
    for (int i = 0; i < childIndex; ++i) {
      oldBlock->appendChild(originalBlock->children()[i]);
    }
    if (!leftTextNode->empty()) {
      oldBlock->appendChild(leftTextNode);
    }
    if (!rightTextNode->empty()) {
      newBlock->appendChild(rightTextNode);
    }
    for (SizeType i = childIndex + 1; i < originalBlock->children().size(); ++i) {
      DEBUG << i;
      NodePtr& child = originalBlock->children()[i];
      newBlock->appendChild(child);
      DEBUG << child->type();
      if (child->type() == NodeType::text) {
        auto textNode = (Text*)child;
        auto s = textNode->toString(m_doc);
        DEBUG << s;
      }
    }
    m_doc->replaceBlock(coord.blockNo, oldBlock);
    m_doc->insertBlock(coord.blockNo + 1, newBlock);
    coord.blockNo++;
    coord.offset = 0;
    updateCursor(cursor, coord);
    delete originalBlock;
  }
  std::pair<SizeType, SizeType> indexOfItem(Container* listNode) {
    SizeType listIndex = 0;
    SizeType itemIndex = 0;
    for (auto child : listNode->children()) {
      ASSERT(child->type() == NodeType::ol_item || child->type() == NodeType::ul_item ||
             child->type() == NodeType::checkbox_item);
      auto item = (Container*)child;
      if (textNode->parent() == child) {
        itemIndex = item->indexOf(textNode);
        break;
      }
      listIndex++;
    }
    return {listIndex, itemIndex};
  }
  void splitListNode(Container* listNode, Container* originalItem, Container* oldItem, Container* newItem,
                     SizeType listIndex, SizeType itemIndex) {
    ASSERT(oldItem != nullptr);
    ASSERT(newItem != nullptr);
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
    coord.offset = 0;
    updateCursor(cursor, coord);
    delete originalItem;
  }

 private:
  Text* textNode;
  int leftOffset;
  Text* leftTextNode;
  Text* rightTextNode;
};
class RemoveTextVisitor
    : public MultipleVisitor<Paragraph, Header, OrderedList, UnorderedList, CheckboxList, CodeBlock>,
      public DocumentOperationVisitor {
 public:
  RemoveTextVisitor(Cursor& cursor, Document* doc) : DocumentOperationVisitor(cursor, doc) {}
  void visit(Paragraph* node) override {
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty() && block.logicalLineAt(0).length() == 0) {
      // 删除当前block，光标左移动
      moveCursorToLeft(cursor);
      coord = cursor.coord();
      removeBlock(coord.blockNo);
      return;
    }
    // 考虑段首按删除的情况
    if (coord.lineNo == 0) {
      if (coord.blockNo > 0) {
        // 合并当前block和前一个block
        // 新坐标为前一个block的最后一行，最后一列
        auto prevBlock = m_doc->m_blocks[coord.blockNo - 1];
        coord.lineNo = prevBlock.countOfLogicalLine() - 1;
        coord.offset = prevBlock.logicalLineAt(prevBlock.countOfLogicalLine() - 1).length();
        mergeBlock(coord.blockNo - 1, coord.blockNo);
        coord.blockNo--;
        updateCursor(cursor, coord);
        return;
      } else {
        auto [textNode, leftOffset] = line.textAt(coord.offset);
        removeTextInNode(textNode, leftOffset);
        if (textNode->empty()) {
          node->removeChild(textNode);
        }
        endRemoveText();
      }
    } else {
      auto [textNode, leftOffset] = line.textAt(coord.offset);
      removeTextInNode(textNode, leftOffset);
      if (textNode->empty()) {
        node->removeChild(textNode);
      }
      endRemoveText();
    }
  }
  void visit(Header* node) override {
    auto& line = curLine();
    if (line.empty() || coord.lineNo == 0) {
      DEBUG << "degrade header to paragraph";
      auto paragraphNode = new Paragraph();
      paragraphNode->setChildren(node->children());
      replaceBlock(coord.blockNo, paragraphNode);
      delete node;
    } else {
      auto [textNode, leftOffset] = line.textAt(coord.offset);
      removeTextInNode(textNode, leftOffset);
      endRemoveText();
    }
  }
  void visit(OrderedList* node) override { removeTextInListNode(node); }
  void visit(UnorderedList* node) override { removeTextInListNode(node); }
  void visit(CheckboxList* node) override { removeTextInListNode(node); }
  void visit(CodeBlock* node) override {
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      if (coord.lineNo > 0) {
        ASSERT(coord.lineNo > 0);
        coord.offset = block.logicalLineAt(coord.lineNo - 1).length();
        auto text1 = node->childAt(coord.lineNo - 1);
        ASSERT(text1->type() == NodeType::text);
        auto text1Node = (Text*)text1;
        auto text2 = node->childAt(coord.lineNo);
        ASSERT(text2->type() == NodeType::text);
        auto text2Node = (Text*)text2;
        text1Node->merge(*text2Node);
        node->removeChildAt(coord.lineNo);
        coord.lineNo--;
        cursor.setCoord(coord);
        renderBlock(coord.blockNo);
        updateCursor(cursor, coord);
        return;
      }
    } else {
      auto [textNode, leftOffset] = line.textAt(coord.offset);
      removeTextInNode(textNode, leftOffset);
      endRemoveText();
    }
  }

 private:
  const LogicalLine& curLine() {
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    return block.logicalLineAt(coord.lineNo);
  }
  void removeTextInListNode(Container* node) {
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      replaceBlock(coord.blockNo, new Paragraph());
      delete node;
    } else if (coord.offset == 0) {
      auto olNode = node;
      ASSERT(coord.lineNo >= 0 && coord.lineNo < olNode->children().size());
      auto olItem = (Container*)olNode->childAt(coord.lineNo);
      // 合并这两个ol item
      if (coord.lineNo == 0) {
        // 如果是第一个ol item，降级为paragraph
        auto paragraph = new Paragraph();
        paragraph->appendChildren(olItem->children());
        insertBlock(coord.blockNo, paragraph);
        olNode->removeChildAt(0);
        if (olNode->empty()) {
          removeBlock(coord.blockNo + 1);
        } else {
          renderBlock(coord.blockNo + 1);
        }
        updateCursor(cursor, coord);
        return;
      } else {
        // 其他情况就是合并两个ol item
        ASSERT(coord.lineNo > 0);
        coord.offset = block.logicalLineAt(coord.lineNo - 1).length();
        auto prevItem = (Container*)olNode->childAt(coord.lineNo - 1);
        auto curItem = (Container*)olNode->childAt(coord.lineNo);
        prevItem->appendChildren(curItem->children());
        olNode->removeChildAt(coord.lineNo);
        coord.lineNo--;
        cursor.setCoord(coord);
        renderBlock(coord.blockNo);
        delete curItem;
        updateCursor(cursor, coord);
        return;
      }
    } else {
      auto [textNode, leftOffset] = line.textAt(coord.offset);
      removeTextInNode(textNode, leftOffset);
      endRemoveText();
    }
  }
  void removeTextInNode(Text* textNode, SizeType leftOffset) {
    if (leftOffset - 1 > 0) {
      auto s = textNode->toString(m_doc);
      auto ch = s[leftOffset - 2].unicode();
      // 如果是emoji的开始标志，两个都要删除
      if (ch == 0xd83d || ch == 0xd83c) {
        textNode->remove(leftOffset - 2, 2);
        coord.offset -= 2;
      } else {
        textNode->remove(leftOffset - 1, 1);
        coord.offset--;
      }
    } else {
      textNode->remove(leftOffset - 1, 1);
      coord.offset--;
    }
  }
  void endRemoveText() {
    renderBlock(coord.blockNo);
    updateCursor(cursor, coord);
  }
};
Document::Document(const String& str, sptr<RenderSetting> setting) : parser::Document(str), m_setting(setting) {
  for (auto node : m_root->children()) {
    const Block& block = Render::render(node, m_setting, this);
    m_blocks.push_back(block);
  }
}
void Document::updateCursor(Cursor& cursor, const CursorCoord& coord, bool updatePos) {
  cursor.setCoord(coord);
  if (!updatePos) return;
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
  cursor.setPos(pos + Point(0, y));
  cursor.setHeight(h);
}
void Document::moveCursorToRight(Cursor& cursor) {
  auto coord = cursor.coord();
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
  updateCursor(cursor, coord);
}
void Document::moveCursorToLeft(Cursor& cursor) {
  auto coord = cursor.coord();
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
  updateCursor(cursor, coord);
}
void Document::moveCursorToUp(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  int x = cursor.pos().x();
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
  updateCursor(cursor, coord);
}
void Document::moveCursorToDown(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  int x = cursor.pos().x();
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
  updateCursor(cursor, coord);
}
void Document::moveCursorToPos(Cursor& cursor, Point pos) {
  // 先找到哪个block
  SizeType blockNo = 0;
  int y = m_setting->docMargin.top();
  // 如果是点在文档上方的空白，也放到第一个位置
  if (pos.y() <= y) {
    auto coord = cursor.coord();
    coord.blockNo = 0;
    coord.lineNo = 0;
    coord.offset = 0;
    updateCursor(cursor, coord);
    return;
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
    coord.offset = 0;
    updateCursor(cursor, coord);
    return;
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
      auto coord = cursor.coord();
      coord.blockNo = blockNo;
      coord.lineNo = lineNo;
      coord.offset = line.offsetAt(Point(pos.x(), pos.y() - oldY), this, m_setting->lineSpacing);
      updateCursor(cursor, coord);
      return;
    }
    y += line.height();
  }
  DEBUG << "not handle";
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
  auto& line = block.logicalLineAt(coord.lineNo);
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
      if (c->size() > coord.lineNo) {
        c->removeChildAt(coord.lineNo);
      }
      c->insertChild(coord.lineNo, newTextNode);
    } else {
      c->appendChild(newTextNode);
    }
    coord.offset += text.size();
    renderBlock(coord.blockNo);
    updateCursor(cursor, coord);
    return;
  }
  auto [textNode, leftOffset] = line.textAt(coord.offset);
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
        auto prefix = line.left(coord.offset, this);
        if (!prefix.isEmpty() && prefix.count('#') == prefix.size()) {
          // 说明是header
          // 将段落转换为标题
          auto header = new Header(prefix.size());
          header->appendChildren(paragraphNode->children());
          textNode->remove(0, prefix.size());
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            ASSERT(textNode->parent() == header);
            header->removeChildAt(0);
          }
          replaceBlock(coord.blockNo, header);
          delete paragraphNode;
          coord.offset = 0;
          updateCursor(cursor, coord);
          return;
        } else if (prefix == "-") {
          // 说明是无序列表
          // 将段落转换为无序列表
          auto ul = new UnorderedList();
          auto ulItem = new UnorderedListItem();
          ul->appendChild(ulItem);
          ulItem->appendChildren(paragraphNode->children());
          textNode->remove(0, 1);
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            ASSERT(textNode->parent() == ulItem);
            ulItem->removeChildAt(0);
          }
          replaceBlock(coord.blockNo, ul);
          delete paragraphNode;
          coord.offset = 0;
          updateCursor(cursor, coord);
          return;
        } else if (prefix == "1.") {
          // 说明是有序列表
          // 将段落转换为有序列表
          auto ol = new OrderedList();
          auto olItem = new OrderedListItem();
          ol->appendChild(olItem);
          olItem->appendChildren(paragraphNode->children());
          textNode->remove(0, 2);
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            ASSERT(textNode->parent() == olItem);
            olItem->removeChildAt(0);
          }
          replaceBlock(coord.blockNo, ol);
          delete paragraphNode;
          coord.offset = 0;
          updateCursor(cursor, coord);
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
      String s = line.left(coord.offset, this);
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
          listNode->removeChildAt(coord.lineNo);
          if (listNode->children().empty()) {
            removeBlock(coord.blockNo + 1);
          } else {
            // 如果还有结点，需要重新渲染
            renderBlock(coord.blockNo + 1);
          }
        } else if (coord.lineNo == block.countOfLogicalLine()) {
          insertBlock(coord.blockNo + 1, checkbox);
          listNode->removeChildAt(coord.lineNo);
        } else {
          auto ul = new UnorderedList();
          for (auto i = coord.lineNo + 1; i < listNode->children().size(); ++i) {
            ul->appendChild(listNode->childAt(i));
          }
          insertBlock(coord.blockNo + 1, ul);
          for (auto i = listNode->children().size() - 1; i >= coord.lineNo; --i) {
            listNode->removeChildAt(i);
          }
          insertBlock(coord.blockNo + 1, checkbox);
          renderBlock(coord.blockNo);
          coord.blockNo++;
          coord.lineNo = 0;
        }
        coord.offset = 0;
        updateCursor(cursor, coord);
        return;
      }
    }
  }
  if (text == ")" || text == "]" || text == "}") {
    auto s = textNode->toString(this);
    ASSERT(leftOffset >= 0 && leftOffset < s.size());
    auto ch = s[leftOffset];
    if (ch == text) {
      coord.offset++;
      updateCursor(cursor, coord);
      return;
    }
  }
  PieceTableItem item{PieceTableItem::add, m_addBuffer.size(), textToInsert.size()};
  m_addBuffer.append(textToInsert);
  textNode->insert(leftOffset, item);
  renderBlock(coord.blockNo);
  coord.offset += text.size();
  updateCursor(cursor, coord);
}
void Document::removeText(Cursor& cursor) {
  RemoveTextVisitor visitor(cursor, this);
  auto node = m_root->childAt(cursor.coord().blockNo);
  node->accept(&visitor);
}
void Document::insertReturn(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  if (line.empty()) {
    auto newBlock = new Paragraph();
    insertBlock(coord.blockNo + 1, newBlock);
    coord.blockNo++;
    coord.offset = 0;
    updateCursor(cursor, coord);
    return;
  }
  InsertReturnVisitor visitor(cursor, this);
  auto node = m_root->childAt(cursor.coord().blockNo);
  node->accept(&visitor);
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
void Document::moveCursorToBol(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  coord.offset = line.moveToBol(coord.offset, this);
  updateCursor(cursor, coord);
}
void Document::moveCursorToEol(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  auto block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  auto [offset, x] = line.moveToEol(coord.offset, this);
  coord.offset = offset;
  cursor.setX(x);
  updateCursor(cursor, coord, false);
}
void Document::removeBlock(SizeType blockNo) {
  ASSERT(blockNo >= 0 && blockNo < m_blocks.size());
  if (m_blocks.size() == 1) {
    // 如果只剩一个block，就保留
    return;
  }
  m_blocks.erase(m_blocks.begin() + blockNo);
  m_root->children().erase(m_root->children().begin() + blockNo);
}
void Document::moveCursorToEndOfDocument(Cursor& cursor) {
  auto coord = cursor.coord();
  ASSERT(!m_blocks.empty());
  coord.blockNo = m_blocks.size() - 1;
  auto block = m_blocks[coord.blockNo];
  ASSERT(block.countOfLogicalLine() > 0);
  coord.lineNo = block.countOfLogicalLine() - 1;
  coord.offset = block.logicalLineAt(coord.lineNo).length();
  updateCursor(cursor, coord);
}
void Document::moveCursorToBeginOfDocument(Cursor& cursor) {
  auto coord = cursor.coord();
  coord.blockNo = 0;
  coord.lineNo = 0;
  coord.offset = 0;
  updateCursor(cursor, coord);
}
}  // namespace md::editor
