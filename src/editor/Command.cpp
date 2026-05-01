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
  void insertBlock(SizeType blockNo, std::unique_ptr<parser::Node> node) { m_doc->insertBlock(blockNo, std::move(node)); }
  void removeBlock(SizeType blockNo) { m_doc->removeBlock(blockNo); }
  void replaceBlock(SizeType blockNo, std::unique_ptr<parser::Node> node) { m_doc->replaceBlock(blockNo, std::move(node)); }
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
    const auto& block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      insertBlock(coord.blockNo + 1, std::make_unique<Paragraph>());
      coord.blockNo++;
      coord.lineNo = 0;
      coord.offset = 0;
      m_doc->updateCursor(cursor, coord);
      return;
    }
    beginInsertReturn();
    std::unique_ptr<Container> oldBlock;
    std::unique_ptr<Container> newBlock;
    String prefix = line.left(coord.offset, m_doc->parserDoc());
    if (prefix.startsWith("```")) {
      leftTextNode->remove(0, 3);
      oldBlock = std::make_unique<CodeBlock>(std::move(leftTextNode));
      if (rightTextNode->empty()) {
        m_doc->replaceBlock(coord.blockNo, std::move(oldBlock));
        coord.offset = 0;
        updateCursor(cursor, coord);
        return;
      }
      newBlock = std::make_unique<Paragraph>();
    } else {
      oldBlock = std::make_unique<Paragraph>();
      newBlock = std::make_unique<Paragraph>();
    }
    splitNode(node, std::move(oldBlock), std::move(newBlock));
  }
  void visit(Header* node) override {
    beginInsertReturn();
    auto oldBlock = std::make_unique<Header>(node->level());
    std::unique_ptr<Container> newBlock;
    if (!this->hasTextNode) {
      newBlock = std::make_unique<Header>(node->level());
      m_doc->insertBlock(coord.blockNo + 1, std::move(newBlock));
      coord.blockNo++;
      coord.lineNo = 0;
      coord.offset = 0;
      updateCursor(cursor, coord);
      return;
    }
    if (rightTextNode->toString(m_doc->parserDoc()).isEmpty()) {
      newBlock = std::make_unique<Paragraph>();
    } else {
      newBlock = std::make_unique<Header>(node->level());
    }
    splitNode(node, std::move(oldBlock), std::move(newBlock));
  }
  void visit(OrderedList* node) override {
    const auto& block = m_doc->m_blocks[coord.blockNo];
    const auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      splitListNode(node, std::make_unique<OrderedList>(), std::make_unique<OrderedList>());
      return;
    }
    beginInsertReturn();
    auto [listIndex, itemIndex] = indexOfItem(node);
    auto originalItem = static_cast<OrderedListItem*>(node->childAt(listIndex));
    auto oldItem = std::make_unique<OrderedListItem>();
    auto newItem = std::make_unique<OrderedListItem>();
    splitListNode(node, originalItem, std::move(oldItem), std::move(newItem), listIndex, itemIndex);
  }
  void visit(UnorderedList* node) override {
    const auto& block = m_doc->m_blocks[coord.blockNo];
    const auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      splitListNode(node, std::make_unique<UnorderedList>(), std::make_unique<UnorderedList>());
      return;
    }
    beginInsertReturn();
    auto [listIndex, itemIndex] = indexOfItem(node);
    auto originalItem = static_cast<UnorderedListItem*>(node->childAt(listIndex));
    auto oldItem = std::make_unique<UnorderedListItem>();
    auto newItem = std::make_unique<UnorderedListItem>();
    splitListNode(node, originalItem, std::move(oldItem), std::move(newItem), listIndex, itemIndex);
  }
  void visit(CheckboxList* node) override {
    const auto& block = m_doc->m_blocks[coord.blockNo];
    const auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      splitListNode(node, std::make_unique<CheckboxList>(), std::make_unique<CheckboxList>());
      return;
    }
    beginInsertReturn();
    auto [listIndex, itemIndex] = indexOfItem(node);
    auto originalItem = static_cast<CheckboxItem*>(node->childAt(listIndex));
    auto oldItem = std::make_unique<CheckboxItem>();
    oldItem->setChecked(originalItem->isChecked());
    auto newItem = std::make_unique<CheckboxItem>();
    splitListNode(node, originalItem, std::move(oldItem), std::move(newItem), listIndex, itemIndex);
  }
  void visit(CodeBlock* node) override {
    beginInsertReturn();
    node->removeChildAt(coord.lineNo);
    node->insertChild(coord.lineNo, std::move(leftTextNode));
    node->insertChild(coord.lineNo + 1, std::move(rightTextNode));
    m_doc->renderBlock(coord.blockNo);
    coord.lineNo++;
    coord.offset = 0;
    updateCursor(cursor, coord);
  }

 private:
  void beginInsertReturn() {
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
    const auto& block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.hasTextAt(coord.offset)) {
      this->hasTextNode = true;
      auto textAndOffset = line.textAt(coord.offset);
      this->textNode = textAndOffset.first;
      this->leftOffset = textAndOffset.second;
      auto leftRightText = textNode->split(leftOffset);
      this->leftTextNode = std::move(leftRightText.first);
      this->rightTextNode = std::move(leftRightText.second);
    } else {
      this->hasTextNode = false;
      this->textNode = nullptr;
    }
  }
  void splitNode(Container* originalBlock, std::unique_ptr<Container> oldBlock, std::unique_ptr<Container> newBlock) {
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
      oldBlock->appendChild(std::move(originalBlock->children()[i]));
    }
    if (!leftTextNode->empty()) {
      oldBlock->appendChild(std::move(leftTextNode));
    }
    if (!rightTextNode->empty()) {
      newBlock->appendChild(std::move(rightTextNode));
    }
    for (SizeType i = childIndex + 1; i < originalBlock->children().size(); ++i) {
      DEBUG << i;
      newBlock->appendChild(std::move(originalBlock->children()[i]));
    }
    m_doc->replaceBlock(coord.blockNo, std::move(oldBlock));
    m_doc->insertBlock(coord.blockNo + 1, std::move(newBlock));
    coord.blockNo++;
    coord.lineNo = 0;
    coord.offset = 0;
    updateCursor(cursor, coord);
  }
  std::pair<SizeType, SizeType> indexOfItem(Container* listNode) {
    SizeType listIndex = 0;
    SizeType itemIndex = 0;
    for (auto& child : listNode->children()) {
      ASSERT(child->type() == NodeType::ol_item || child->type() == NodeType::ul_item ||
             child->type() == NodeType::checkbox_item);
      auto* containerChild = static_cast<Container*>(child.get());
      if (textNode->parent() == child.get()) {
        itemIndex = containerChild->indexOf(textNode);
        break;
      }
      listIndex++;
    }
    return {listIndex, itemIndex};
  }
  void splitListNode(Container* listNode, Container* originalItem, std::unique_ptr<Container> oldItem, std::unique_ptr<Container> newItem,
                     SizeType listIndex, SizeType itemIndex) {
    ASSERT(oldItem != nullptr);
    ASSERT(newItem != nullptr);
    for (int i = 0; i < itemIndex; ++i) {
      ASSERT(i < originalItem->children().size());
      oldItem->appendChild(std::move(originalItem->children()[i]));
    }
    oldItem->appendChild(std::move(leftTextNode));
    newItem->appendChild(std::move(rightTextNode));
    for (SizeType i = itemIndex + 1; i < originalItem->children().size(); ++i) {
      ASSERT(i < originalItem->children().size());
      newItem->appendChild(std::move(originalItem->children()[i]));
    }
    listNode->setChild(listIndex, std::move(oldItem));
    listNode->insertChild(listIndex + 1, std::move(newItem));
    renderBlock(coord.blockNo);
    coord.lineNo++;
    coord.offset = 0;
    updateCursor(cursor, coord);
  }
  void splitListNode(Container* originalListNode, std::unique_ptr<Container> oldListNode,
                     std::unique_ptr<Container> newListNode) {
    for (int i = 0; i < coord.lineNo; ++i) {
      oldListNode->appendChild(std::move((*originalListNode)[i]));
    }
    for (auto i = coord.lineNo + 1; i < originalListNode->size(); ++i) {
      newListNode->appendChild(std::move((*originalListNode)[i]));
    }
    auto index = coord.blockNo;
    m_doc->removeBlock(index);
    if (!oldListNode->empty()) {
      m_doc->insertBlock(index, std::move(oldListNode));
      index++;
    }
    m_doc->insertBlock(index, std::make_unique<Paragraph>());
    auto coord2 = cursor.coord();
    coord2.blockNo = index;
    coord2.lineNo = 0;
    coord2.offset = 0;
    m_doc->updateCursor(cursor, coord2);
    index++;
    if (!newListNode->empty()) {
      m_doc->insertBlock(index, std::move(newListNode));
    }
  }

 private:
  bool hasTextNode;
  Text* textNode;
  int leftOffset;
  std::unique_ptr<Text> leftTextNode;
  std::unique_ptr<Text> rightTextNode;
};
class RemoveTextVisitor
    : public MultipleVisitor<Paragraph, Header, OrderedList, UnorderedList, CheckboxList, CodeBlock>,
      public DocumentOperationVisitor {
 public:
  RemoveTextVisitor(Cursor& cursor, Document* doc) : DocumentOperationVisitor(cursor, doc) {}
  void visit(Paragraph* node) override {
    const auto& block = m_doc->m_blocks[coord.blockNo];
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
          const auto& prevBlock = m_doc->m_blocks[coord.blockNo - 1];
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
    const auto& block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    const auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty() || (coord.lineNo == 0 && coord.offset == 0)) {
      DEBUG << "degrade header to paragraph";
      auto paragraphNode = std::make_unique<Paragraph>();
      paragraphNode->setChildren(std::move(node->children()));
      replaceBlock(coord.blockNo, std::move(paragraphNode));
      m_doc->updateCursor(cursor, cursor.coord());
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
    const auto& block = m_doc->m_blocks[coord.blockNo];
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
    const auto& block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      replaceBlock(coord.blockNo, std::make_unique<Paragraph>());
    } else if (coord.offset == 0) {
      auto olNode = node;
      ASSERT(coord.lineNo >= 0 && coord.lineNo < olNode->children().size());
      auto olItem = static_cast<Container*>(olNode->childAt(coord.lineNo));
      // 合并这两个ol item
      if (coord.lineNo == 0) {
        // 如果是第一个ol item，降级为paragraph
        auto paragraph = std::make_unique<Paragraph>();
        paragraph->appendChildren(std::move(olItem->children()));
        insertBlock(coord.blockNo, std::move(paragraph));
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
        auto prevItem = static_cast<Container*>(olNode->childAt(coord.lineNo - 1));
        auto curItem = static_cast<Container*>(olNode->childAt(coord.lineNo));
        prevItem->appendChildren(std::move(curItem->children()));
        olNode->removeChildAt(coord.lineNo);
        coord.lineNo--;
        cursor.setCoord(coord);
        renderBlock(coord.blockNo);
        updateCursor(cursor, coord);
        return;
      }
    } else {
      auto [textNode, leftOffset] = line.textAt(coord.offset);
      removeTextInNode(textNode, leftOffset);
      if (textNode->empty()) {
        auto p = textNode->parent();
        ASSERT(p != nullptr);
        auto c = static_cast<Container*>(p);
        c->removeChild(textNode);
      }
      endRemoveText();
    }
  }
  void removeTextInNode(Text* textNode, SizeType leftOffset) {
    if (leftOffset - 1 > 0) {
      auto s = textNode->toString(m_doc->parserDoc());
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
    const auto& block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      auto newTextNode = std::make_unique<Text>(PieceTableItem::add, offset, length);
      coord.offset += cursorOffsetDelta;
      node->appendChild(std::move(newTextNode));
      renderBlock(coord.blockNo);
      updateCursor(cursor, coord);
      return;
    }
    auto [textNode, leftOffset] = line.textAt(coord.offset);
    auto parentNode = textNode->parent();
    ASSERT(parentNode != nullptr);
    auto ppNode = parentNode->parent();
    ASSERT(ppNode == m_doc->root() && "node hierarchy error");
    if (isSpace) {
      if (coord.offset <= 6) {
        auto prefix = line.left(coord.offset, m_doc->parserDoc());
        if (!prefix.isEmpty() && prefix.count('#') == prefix.size()) {
          // 说明是header
          // 将段落转换为标题
          auto header = std::make_unique<Header>(prefix.size());
          header->appendChildren(std::move(node->children()));
          textNode->remove(0, prefix.size());
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            ASSERT(textNode->parent() == header.get());
            header->removeChildAt(0);
          }
          replaceBlock(coord.blockNo, std::move(header));
          coord.offset = 0;
          updateCursor(cursor, coord);
          return;
        } else if (prefix == "-") {
          // 说明是无序列表
          // 将段落转换为无序列表
          auto ul = std::make_unique<UnorderedList>();
          auto ulItem = std::make_unique<UnorderedListItem>();
          auto* ulItemRaw = ulItem.get();
          ul->appendChild(std::move(ulItem));
          ulItemRaw->appendChildren(std::move(node->children()));
          textNode->remove(0, 1);
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            ASSERT(textNode->parent() == ulItem.get());
            ulItem->removeChildAt(0);
          }
          replaceBlock(coord.blockNo, std::move(ul));
          coord.offset = 0;
          updateCursor(cursor, coord);
          return;
        } else if (prefix == "1.") {
          // 说明是有序列表
          // 将段落转换为有序列表
          auto ol = std::make_unique<OrderedList>();
          auto olItem = std::make_unique<OrderedListItem>();
          auto* olItemRaw = olItem.get();
          ol->appendChild(std::move(olItem));
          olItemRaw->appendChildren(std::move(node->children()));
          textNode->remove(0, 2);
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            ASSERT(textNode->parent() == olItem.get());
            olItem->removeChildAt(0);
          }
          replaceBlock(coord.blockNo, std::move(ol));
          coord.offset = 0;
          updateCursor(cursor, coord);
          return;
        } else if (prefix == ">") {
          // 说明是引用块
          // 将段落转换为引用块
          auto quoteBlock = std::make_unique<QuoteBlock>();
          quoteBlock->appendChildren(std::move(node->children()));
          textNode->remove(0, prefix.size());
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            ASSERT(textNode->parent() == quoteBlock.get());
            quoteBlock->removeChildAt(0);
          }
          replaceBlock(coord.blockNo, std::move(quoteBlock));
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
    const auto& block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      auto newTextNode = std::make_unique<Text>(PieceTableItem::add, offset, length);
      coord.offset += cursorOffsetDelta;
      node->appendChild(std::move(newTextNode));
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
    const auto& block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);

    if (line.empty()) {
      insertTextInEmptyList(node);
      return;
    }
    auto [textNode, leftOffset] = line.textAt(coord.offset);
    insertTextInNode(textNode, leftOffset);
  }
  void visit(UnorderedList* node) override {
    auto coord = cursor.coord();
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
    const auto& block = m_doc->m_blocks[coord.blockNo];
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
    String s = line.left(coord.offset, m_doc->parserDoc());
    auto prefix = s.left(coord.offset);
    auto listItem = static_cast<UnorderedListItem*>(node->childAt(coord.lineNo));
    if (prefix == "[ ]" || prefix == "[x]") {
      // 转换为checkbox
      // 需要考虑是在第一个，中间，还是最后一个，三种情况
      auto checkbox = std::make_unique<CheckboxList>();
      auto checkboxItem = std::make_unique<CheckboxItem>();
      checkboxItem->setChecked(prefix == "[x]");
      auto* checkboxItemRaw = checkboxItem.get();
      checkbox->appendChild(std::move(checkboxItem));
      SizeType leftLength = 3;
      for (auto& child : listItem->children()) {
        if (child->type() == NodeType::text) {
          auto textNode = static_cast<Text*>(child.get());
          auto s = textNode->toString(m_doc->parserDoc());
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
        checkboxItemRaw->appendChild(std::move(child));
      }
      if (coord.lineNo == 0) {
        insertBlock(coord.blockNo, std::move(checkbox));
        node->removeChildAt(coord.lineNo);
        if (node->children().empty()) {
          removeBlock(coord.blockNo + 1);
        } else {
          // 如果还有结点，需要重新渲染
          renderBlock(coord.blockNo + 1);
        }
      } else if (coord.lineNo == block.countOfLogicalLine()) {
        insertBlock(coord.blockNo + 1, std::move(checkbox));
        node->removeChildAt(coord.lineNo);
      } else {
        auto ul = std::make_unique<UnorderedList>();
        for (auto i = coord.lineNo + 1; i < node->size(); ++i) {
          ul->appendChild(std::move((*node)[i]));
        }
        insertBlock(coord.blockNo + 1, std::move(ul));
        for (auto i = node->size() - 1; i >= coord.lineNo; --i) {
          node->removeChildAt(i);
        }
        insertBlock(coord.blockNo + 1, std::move(checkbox));
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
    const auto& block = m_doc->m_blocks[coord.blockNo];
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
    const auto& block = m_doc->m_blocks[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      auto newTextNode = std::make_unique<Text>(PieceTableItem::add, offset, length);
      if (node->size() > coord.lineNo) {
        node->removeChildAt(coord.lineNo);
      }
      node->insertChild(coord.lineNo, std::move(newTextNode));
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
      auto s = textNode->toString(m_doc->parserDoc());
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
    auto newTextNode = std::make_unique<Text>(PieceTableItem::add, offset, length);
    ASSERT(coord.lineNo >= 0 && coord.lineNo < node->size());
    auto listItem = static_cast<Container*>(node->childAt(coord.lineNo));
    listItem->appendChild(std::move(newTextNode));
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
    auto node = m_doc->root()->childAt(m_coord.blockNo);
    node->accept(&visitor);
  }
}
void InsertTextCommand::execute(Cursor& cursor) {
  m_doc->updateCursor(cursor, m_coord);
  InsertTextVisitor visitor(cursor, m_doc, m_offset, m_length, m_cursorOffsetDelta, m_isSpace, m_maySkipChar,
                            m_targetSkipChar);
  auto node = m_doc->root()->childAt(m_coord.blockNo);
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
  m_offset = m_doc->addBuffer().size();
  m_length = textToInsert.size();
  m_cursorOffsetDelta = text.size();
  m_doc->addBuffer().append(textToInsert);
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
  auto node = m_doc->root()->childAt(cursor.coord().blockNo);
  node->accept(&visitor);
}
void RemoveTextCommand::undo(Cursor& cursor) {}
void InsertReturnCommand::execute(Cursor& cursor) {
  m_doc->updateCursor(cursor, m_coord);
  auto coord = m_coord;
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->m_blocks.size());
  const auto& block = m_doc->m_blocks[coord.blockNo];
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
  auto node = m_doc->root()->childAt(cursor.coord().blockNo);
  node->accept(&visitor);
  m_finishedCoord = cursor.coord();
}
void InsertReturnCommand::undo(Cursor& cursor) {
  m_doc->updateCursor(cursor, m_finishedCoord);
  RemoveTextVisitor visitor(cursor, m_doc);
  auto node = m_doc->root()->childAt(cursor.coord().blockNo);
  node->accept(&visitor);
}
void CommandStack::push(std::unique_ptr<Command> command) {
  while (m_commands.size() != m_top) {
    m_commands.pop_back();
  }
  if (m_commands.empty()) {
    m_commands.push_back(std::move(command));
  } else {
    // 判断和栈顶元素能不能合并
    auto* topCommand = m_commands.back().get();
    if (!topCommand->merge(command.get())) {
      m_commands.push_back(std::move(command));
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
