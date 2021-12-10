//
// Created by PikachuHy on 2021/11/26.
//

#include "Command.h"

#include "Cursor.h"
#include "debug.h"
#include "parser/Document.h"
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
  void moveCursorToLeft(Cursor& cursor) {
    auto coord = m_doc->moveCursorToLeft(cursor.coord());
    m_doc->updateCursor(cursor, coord);
  }

 protected:
  Document* m_doc;
  Cursor& cursor;
  CursorCoord coord;
};
class InsertReturnVisitor
    : public MultipleVisitor<Paragraph, Header, OrderedList, UnorderedList, CheckboxList, CodeBlock>,
      public DocumentOperationVisitor {
 public:
  InsertReturnVisitor(Cursor& cursor, Document* doc) : DocumentOperationVisitor(cursor, doc) { coord = cursor.coord(); }
  void visit(Paragraph* node) override {
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      auto newBlock = new Paragraph();
      m_doc->insertBlock(coord.blockNo + 1, newBlock);
      coord.blockNo++;
      coord.lineNo = 0;
      coord.offset = 0;
      m_doc->updateCursor(cursor, coord);
      return;
    }
    beginInsertReturn();
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
    beginInsertReturn();
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
    const auto& block = m_doc->m_blocks[coord.blockNo];
    const auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      splitListNode(node, new OrderedList(), new OrderedList());
      return;
    }
    beginInsertReturn();
    auto [listIndex, itemIndex] = indexOfItem(node);
    auto originalItem = (OrderedListItem*)node->childAt(listIndex);
    auto oldItem = new OrderedListItem();
    auto newItem = new OrderedListItem();
    splitListNode(node, originalItem, oldItem, newItem, listIndex, itemIndex);
  }
  void visit(UnorderedList* node) override {
    const auto& block = m_doc->m_blocks[coord.blockNo];
    const auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      splitListNode(node, new UnorderedList(), new UnorderedList());
      return;
    }
    beginInsertReturn();
    auto [listIndex, itemIndex] = indexOfItem(node);
    auto originalItem = (UnorderedListItem*)node->childAt(listIndex);
    auto oldItem = new UnorderedListItem();
    auto newItem = new UnorderedListItem();
    splitListNode(node, originalItem, oldItem, newItem, listIndex, itemIndex);
  }
  void visit(CheckboxList* node) override {
    const auto& block = m_doc->m_blocks[coord.blockNo];
    const auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      splitListNode(node, new CheckboxList(), new CheckboxList());
      return;
    }
    beginInsertReturn();
    auto [listIndex, itemIndex] = indexOfItem(node);
    auto originalItem = (CheckboxItem*)node->childAt(listIndex);
    auto oldItem = new CheckboxItem();
    oldItem->setChecked(originalItem->isChecked());
    auto newItem = new CheckboxItem();
    splitListNode(node, originalItem, oldItem, newItem, listIndex, itemIndex);
  }
  void visit(CodeBlock* node) override {
    beginInsertReturn();
    node->removeChildAt(coord.lineNo);
    node->insertChild(coord.lineNo, leftTextNode);
    node->insertChild(coord.lineNo + 1, rightTextNode);
    m_doc->renderBlock(coord.blockNo);
    coord.lineNo++;
    coord.offset = 0;
    updateCursor(cursor, coord);
  }

 private:
  void beginInsertReturn() {
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
    coord.lineNo = 0;
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
  void splitListNode(Container* originalListNode, Container* oldListNode, Container* newListNode) {
    for (int i = 0; i < coord.lineNo; ++i) {
      oldListNode->appendChild(originalListNode->childAt(i));
    }
    for (auto i = coord.lineNo + 1; i < originalListNode->size(); ++i) {
      newListNode->appendChild(originalListNode->childAt(i));
    }
    auto index = coord.blockNo;
    m_doc->removeBlock(index);
    if (oldListNode->empty()) {
      delete oldListNode;
    } else {
      m_doc->insertBlock(index, oldListNode);
      index++;
    }
    m_doc->insertBlock(index, new Paragraph());
    auto coord = cursor.coord();
    coord.blockNo = index;
    coord.lineNo = 0;
    coord.offset = 0;
    m_doc->updateCursor(cursor, coord);
    index++;
    if (newListNode->empty()) {
      delete newListNode;
    } else {
      m_doc->insertBlock(index, newListNode);
    }
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
      coord = cursor.coord();
      // 删除当前block，光标左移动
      moveCursorToLeft(cursor);
      removeBlock(coord.blockNo);
      return;
    }
    // 考虑段首按删除的情况
    if (coord.lineNo == 0) {
      if (coord.offset == 0) {
        if (coord.blockNo == 0) {
          // do nothing
          DEBUG << "do nothing";
          return;
        } else if (coord.blockNo > 0) {
          // 合并当前block和前一个block
          // 新坐标为前一个block的最后一行，最后一列
          auto prevBlock = m_doc->m_blocks[coord.blockNo - 1];
          coord.lineNo = prevBlock.countOfLogicalLine() - 1;
          coord.offset = prevBlock.logicalLineAt(prevBlock.countOfLogicalLine() - 1).length();
          mergeBlock(coord.blockNo - 1, coord.blockNo);
          coord.blockNo--;
          updateCursor(cursor, coord);
          return;
        }
      }
      auto [textNode, leftOffset] = line.textAt(coord.offset);
      removeTextInNode(textNode, leftOffset);
      // TODO: 处理深层次的空结点
      if (textNode->empty()) {
        if (textNode->parent() == node) {
          node->removeChild(textNode);
        } else {
          auto p = textNode->parent();
          if (p->type() == NodeType::link) {
            if (p->parent() == node) {
              node->removeChild(p);
            } else {
              DEBUG << p->parent()->type();
              ASSERT(false && "too deep hierarchy");
            }
          } else {
            DEBUG << p->type();
            ASSERT(false && "un support now");
          }
        }
      }
      endRemoveText();
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
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    const auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty() || (coord.lineNo == 0 && coord.offset == 0)) {
      DEBUG << "degrade header to paragraph";
      auto paragraphNode = new Paragraph();
      paragraphNode->setChildren(node->children());
      replaceBlock(coord.blockNo, paragraphNode);
      m_doc->updateCursor(cursor, cursor.coord());
      delete node;
    } else {
      auto [textNode, leftOffset] = line.textAt(coord.offset);
      removeTextInNode(textNode, leftOffset);
      if (textNode->empty()) {
        if (textNode->parent() == node) {
          node->removeChild(textNode);
        } else {
          DEBUG << "unhandled empty text node";
        }
      }
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
      if (textNode->empty()) {
        auto p = textNode->parent();
        ASSERT(p != nullptr);
        auto c = (Container*)p;
        c->removeChild(textNode);
      }
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

class InsertTextVisitor
    : public MultipleVisitor<Paragraph, Header, OrderedList, UnorderedList, CheckboxList, CodeBlock>,
      public DocumentOperationVisitor {
 public:
  InsertTextVisitor(Cursor& cursor, Document* doc, SizeType offset, SizeType length, SizeType cursorOffsetDelta,
                    bool isSpace, bool maySkipChar, String targetSkipChar)
      : DocumentOperationVisitor(cursor, doc),
        offset(offset),
        length(length),
        cursorOffsetDelta(cursorOffsetDelta),
        isSpace(isSpace),
        maySkipChar(maySkipChar),
        targetSkipChar(targetSkipChar) {}
  void visit(Paragraph* node) override {
    auto coord = cursor.coord();
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      Text* newTextNode = new Text(PieceTableItem::add, offset, length);
      coord.offset += cursorOffsetDelta;
      node->appendChild(newTextNode);
      renderBlock(coord.blockNo);
      updateCursor(cursor, coord);
      return;
    }
    auto [textNode, leftOffset] = line.textAt(coord.offset);
    auto parentNode = textNode->parent();
    ASSERT(parentNode != nullptr);
    auto ppNode = parentNode->parent();
    ASSERT(ppNode == m_doc->m_root.get() && "node hierarchy error");
    if (isSpace) {
      if (coord.offset <= 6) {
        auto prefix = line.left(coord.offset, m_doc);
        if (!prefix.isEmpty() && prefix.count('#') == prefix.size()) {
          // 说明是header
          // 将段落转换为标题
          auto header = new Header(prefix.size());
          header->appendChildren(node->children());
          textNode->remove(0, prefix.size());
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            ASSERT(textNode->parent() == header);
            header->removeChildAt(0);
          }
          replaceBlock(coord.blockNo, header);
          delete node;
          coord.offset = 0;
          updateCursor(cursor, coord);
          return;
        } else if (prefix == "-") {
          // 说明是无序列表
          // 将段落转换为无序列表
          auto ul = new UnorderedList();
          auto ulItem = new UnorderedListItem();
          ul->appendChild(ulItem);
          ulItem->appendChildren(node->children());
          textNode->remove(0, 1);
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            ASSERT(textNode->parent() == ulItem);
            ulItem->removeChildAt(0);
          }
          replaceBlock(coord.blockNo, ul);
          delete node;
          coord.offset = 0;
          updateCursor(cursor, coord);
          return;
        } else if (prefix == "1.") {
          // 说明是有序列表
          // 将段落转换为有序列表
          auto ol = new OrderedList();
          auto olItem = new OrderedListItem();
          ol->appendChild(olItem);
          olItem->appendChildren(node->children());
          textNode->remove(0, 2);
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            ASSERT(textNode->parent() == olItem);
            olItem->removeChildAt(0);
          }
          replaceBlock(coord.blockNo, ol);
          delete node;
          coord.offset = 0;
          updateCursor(cursor, coord);
          return;
        }
      } else {
        // 大于6的就不可能变成标题了
      }
    }
    insertTextInNode(textNode, leftOffset);
  }
  void visit(Header* node) override {
    auto coord = cursor.coord();
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      Text* newTextNode = new Text(PieceTableItem::add, offset, length);
      coord.offset += cursorOffsetDelta;
      node->appendChild(newTextNode);
      renderBlock(coord.blockNo);
      updateCursor(cursor, coord);
      return;
    }
    auto [textNode, leftOffset] = line.textAt(coord.offset);
    insertTextInNode(textNode, leftOffset);
  }
  void visit(OrderedList* node) override {
    auto coord = cursor.coord();
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);

    if (line.empty()) {
      insertTextInEmptyList(node);
    }
    auto [textNode, leftOffset] = line.textAt(coord.offset);
    insertTextInNode(textNode, leftOffset);
  }
  void visit(UnorderedList* node) override {
    auto coord = cursor.coord();
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      insertTextInEmptyList(node);
      return;
    }

    auto [textNode, leftOffset] = line.textAt(coord.offset);
    auto parentNode = textNode->parent();
    ASSERT(parentNode != nullptr);
    auto ppNode = parentNode->parent();
    ASSERT(ppNode == node && "node hierarchy error");
    String s = line.left(coord.offset, m_doc);
    auto prefix = s.left(coord.offset);
    auto listItem = (UnorderedListItem*)node->childAt(coord.lineNo);
    if (prefix == "[ ]" || prefix == "[x]") {
      // 转换为checkbox
      // 需要考虑是在第一个，中间，还是最后一个，三种情况
      auto checkbox = new CheckboxList();
      auto checkboxItem = new CheckboxItem();
      checkboxItem->setChecked(prefix == "[x]");
      checkbox->appendChild(checkboxItem);
      SizeType leftLength = 3;
      for (auto& child : listItem->children()) {
        if (child->type() == NodeType::text) {
          auto textNode = (Text*)child;
          auto s = textNode->toString(m_doc);
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
        checkboxItem->appendChild(child);
      }
      if (coord.lineNo == 0) {
        insertBlock(coord.blockNo, checkbox);
        node->removeChildAt(coord.lineNo);
        if (node->children().empty()) {
          removeBlock(coord.blockNo + 1);
        } else {
          // 如果还有结点，需要重新渲染
          renderBlock(coord.blockNo + 1);
        }
      } else if (coord.lineNo == block.countOfLogicalLine()) {
        insertBlock(coord.blockNo + 1, checkbox);
        node->removeChildAt(coord.lineNo);
      } else {
        auto ul = new UnorderedList();
        for (auto i = coord.lineNo + 1; i < node->size(); ++i) {
          ul->appendChild(node->childAt(i));
        }
        insertBlock(coord.blockNo + 1, ul);
        for (auto i = node->size() - 1; i >= coord.lineNo; --i) {
          node->removeChildAt(i);
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
    insertTextInNode(textNode, leftOffset);
  }
  void visit(CheckboxList* node) override {
    auto coord = cursor.coord();
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      insertTextInEmptyList(node);
      return;
    }
    auto [textNode, leftOffset] = line.textAt(coord.offset);
    insertTextInNode(textNode, leftOffset);
  }
  void visit(CodeBlock* node) override {
    auto coord = cursor.coord();
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
    auto block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      Text* newTextNode = new Text(PieceTableItem::add, offset, length);
      if (node->size() > coord.lineNo) {
        node->removeChildAt(coord.lineNo);
      }
      node->insertChild(coord.lineNo, newTextNode);
      coord.offset += cursorOffsetDelta;
      renderBlock(coord.blockNo);
      updateCursor(cursor, coord);
      return;
    }
    auto [textNode, leftOffset] = line.textAt(coord.offset);
    insertTextInNode(textNode, leftOffset);
  }

 private:
  void insertTextInNode(Text* textNode, int leftOffset) {
    if (maySkipChar) {
      auto s = textNode->toString(m_doc);
      ASSERT(leftOffset >= 0 && leftOffset < s.size());
      auto ch = s[leftOffset];
      if (ch == targetSkipChar) {
        coord.offset++;
        updateCursor(cursor, coord);
        return;
      }
    }
    PieceTableItem item{PieceTableItem::add, offset, length};
    textNode->insert(leftOffset, item);
    renderBlock(coord.blockNo);
    coord.offset += cursorOffsetDelta;
    updateCursor(cursor, coord);
  }
  void insertTextInEmptyList(Container* node) {
    Text* newTextNode = new Text(PieceTableItem::add, offset, length);
    ASSERT(coord.lineNo >= 0 && coord.lineNo < node->size());
    auto listItem = (Container*)node->childAt(coord.lineNo);
    listItem->appendChild(newTextNode);
    coord.offset += cursorOffsetDelta;
    renderBlock(coord.blockNo);
    updateCursor(cursor, coord);
  }

 private:
  SizeType offset;
  SizeType length;
  bool isSpace;
  bool maySkipChar;
  String targetSkipChar;
  SizeType cursorOffsetDelta;
};

void InsertTextCommand::undo(Cursor& cursor) {
  DEBUG << m_coord << m_finishedCoord;
  m_doc->updateCursor(cursor, m_finishedCoord);
  // 无法处理block的升降级
  while (cursor.coord() != m_coord) {
    RemoveTextVisitor visitor(cursor, m_doc);
    auto node = m_doc->m_root->childAt(m_coord.blockNo);
    node->accept(&visitor);
  }
}
void InsertTextCommand::execute(Cursor& cursor) {
  m_doc->updateCursor(cursor, m_coord);
  InsertTextVisitor visitor(cursor, m_doc, m_offset, m_length, m_cursorOffsetDelta, m_isSpace, m_maySkipChar,
                            m_targetSkipChar);
  auto node = m_doc->m_root->childAt(m_coord.blockNo);
  node->accept(&visitor);
  m_finishedCoord = cursor.coord();
}

InsertTextCommand::InsertTextCommand(Document* doc, CursorCoord coord, String text) : Command(doc), m_coord(coord) {
  auto textToInsert = text;
  if (text == "(") {
    textToInsert = "()";
  } else if (text == "[") {
    textToInsert = "[]";
  } else if (text == "{") {
    textToInsert = "{}";
  }
  if (text == ")" || text == "]" || text == "}") {
    m_maySkipChar = true;
    m_targetSkipChar = text;
  } else {
    m_maySkipChar = false;
  }
  if (text == " ") {
    m_isSpace = true;
  } else {
    m_isSpace = false;
  }
  m_offset = m_doc->m_addBuffer.size();
  m_length = textToInsert.size();
  m_cursorOffsetDelta = text.size();
  m_doc->m_addBuffer.append(textToInsert);
}
bool InsertTextCommand::merge(Command* command) {
  if (this->type() != command->type()) return false;
  auto insertTextCommand = (InsertTextCommand*)command;
  if (m_finishedCoord != insertTextCommand->m_coord) return false;
  if (m_offset + m_length != insertTextCommand->m_offset) return false;
  m_length += insertTextCommand->m_length;
  m_finishedCoord = insertTextCommand->m_finishedCoord;
  return true;
}
void RemoveTextCommand::execute(Cursor& cursor) {
  m_doc->updateCursor(cursor, m_coord);
  RemoveTextVisitor visitor(cursor, m_doc);
  auto node = m_doc->m_root->childAt(cursor.coord().blockNo);
  node->accept(&visitor);
}
void RemoveTextCommand::undo(Cursor& cursor) {}
void InsertReturnCommand::execute(Cursor& cursor) {
  m_doc->updateCursor(cursor, m_coord);
  auto coord = m_coord;
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
  auto block = m_doc->m_blocks[coord.blockNo];
#if 0
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  if (line.empty()) {
    auto newBlock = new Paragraph();
    m_doc->insertBlock(coord.blockNo + 1, newBlock);
    coord.blockNo++;
    coord.lineNo = 0;
    coord.offset = 0;
    m_doc->updateCursor(cursor, coord);
    return;
  }
#endif
  InsertReturnVisitor visitor(cursor, m_doc);
  auto node = m_doc->m_root->childAt(cursor.coord().blockNo);
  node->accept(&visitor);
  m_finishedCoord = cursor.coord();
}
void InsertReturnCommand::undo(Cursor& cursor) {
  m_doc->updateCursor(cursor, m_finishedCoord);
  RemoveTextVisitor visitor(cursor, m_doc);
  auto node = m_doc->m_root->childAt(cursor.coord().blockNo);
  node->accept(&visitor);
}
void CommandStack::push(Command* command) {
  while (m_commands.size() != m_top) {
    m_commands.pop_back();
  }
  if (m_commands.empty()) {
    m_commands.push_back(command);
  } else {
    // 判断和栈顶元素能不能合并
    auto topCommand = m_commands.back();
    if (!topCommand->merge(command)) {
      m_commands.push_back(command);
    }
  }
  m_top = m_commands.size();
}
void CommandStack::undo(Cursor& cursor) {
  ASSERT(m_top >= 0 && m_top <= m_commands.size());
  if (m_top == 0) return;
  m_commands[m_top - 1]->undo(cursor);
  m_top--;
}
void CommandStack::redo(Cursor& cursor) {
  ASSERT(m_top >= 0 && m_top <= m_commands.size());
  if (m_top == m_commands.size()) return;
  m_commands[m_top]->execute(cursor);
  m_top++;
}
}  // namespace md::editor