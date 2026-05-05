//
// Created by PikachuHy on 2021/11/26.
//

#include "Command.h"

#include "Cursor.h"
#include "core/Utf8Util.h"
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
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->blocks().size());
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
    : public NodeVisitor,
      public DocumentOperationVisitor {
 public:
  InsertReturnVisitor(Cursor& cursor, Document* doc) : DocumentOperationVisitor(cursor, doc) { coord = cursor.coord(); }
  void visit(Paragraph* node) override {
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->blocks().size());
    const auto& block = m_doc->blocks()[coord.blockNo];
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
    String prefix = line.left(coord.offset, m_doc->bufferProvider());
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
    if (rightTextNode->toString(m_doc->bufferProvider()).isEmpty()) {
      newBlock = std::make_unique<Paragraph>();
    } else {
      newBlock = std::make_unique<Header>(node->level());
    }
    splitNode(node, std::move(oldBlock), std::move(newBlock));
  }
  void visit(OrderedList* node) override { handleInsertReturnInList(node); }
  void visit(UnorderedList* node) override { handleInsertReturnInList(node); }
  void visit(CheckboxList* node) override { handleInsertReturnInList(node); }
  void visit(CodeBlock* node) override {
    beginInsertReturn();
    if (!this->hasTextNode) {
      // Cursor is on an empty line. Insert a new empty Text child and re-render.
      node->insertChild(coord.lineNo + 1, std::make_unique<Text>(0, 0));
      m_doc->renderBlock(coord.blockNo);
      coord.lineNo++;
      coord.offset = 0;
      updateCursor(cursor, coord);
      return;
    }
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
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->blocks().size());
    const auto& block = m_doc->blocks()[coord.blockNo];
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
    if (leftTextNode && !leftTextNode->empty()) {
      oldBlock->appendChild(std::move(leftTextNode));
    }
    if (rightTextNode && !rightTextNode->empty()) {
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
      ASSERT(dynamic_cast<ListItemNode*>(child.get()) != nullptr);
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
  void handleInsertReturnInList(Container* node) {
    const auto& block = m_doc->blocks()[coord.blockNo];
    const auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      if (node->type() == NodeType::ol)
        splitListNode(node, std::make_unique<OrderedList>(), std::make_unique<OrderedList>());
      else if (node->type() == NodeType::ul)
        splitListNode(node, std::make_unique<UnorderedList>(), std::make_unique<UnorderedList>());
      else if (node->type() == NodeType::checkbox)
        splitListNode(node, std::make_unique<CheckboxList>(), std::make_unique<CheckboxList>());
      return;
    }
    beginInsertReturn();
    auto [listIndex, itemIndex] = indexOfItem(node);
    auto originalItem = static_cast<Container*>(node->childAt(listIndex));
    std::unique_ptr<Container> oldItem;
    std::unique_ptr<Container> newItem;
    if (node->type() == NodeType::ol) {
      oldItem = std::make_unique<OrderedListItem>();
      newItem = std::make_unique<OrderedListItem>();
    } else if (node->type() == NodeType::ul) {
      oldItem = std::make_unique<UnorderedListItem>();
      newItem = std::make_unique<UnorderedListItem>();
    } else if (node->type() == NodeType::checkbox) {
      oldItem = std::make_unique<CheckboxItem>();
      static_cast<CheckboxItem*>(oldItem.get())->setChecked(
          static_cast<CheckboxItem*>(originalItem)->isChecked());
      newItem = std::make_unique<CheckboxItem>();
    }
    splitListNode(node, originalItem, std::move(oldItem), std::move(newItem), listIndex, itemIndex);
  }

 private:
  bool hasTextNode;
  Text* textNode;
  int leftOffset;
  std::unique_ptr<Text> leftTextNode;
  std::unique_ptr<Text> rightTextNode;
};
class RemoveTextVisitor
    : public NodeVisitor,
      public DocumentOperationVisitor {
 public:
  RemoveTextVisitor(Cursor& cursor, Document* doc, RemoveTextCommand* cmd = nullptr)
      : DocumentOperationVisitor(cursor, doc), m_cmd(cmd) {}
  void visit(Paragraph* node) override {
    const auto& block = m_doc->blocks()[coord.blockNo];
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
          if (m_cmd) {
            auto prevContainer = m_doc->node2container(m_doc->root()->childAt(coord.blockNo - 1));
            auto curContainer = m_doc->node2container(m_doc->root()->childAt(coord.blockNo));
            ASSERT(prevContainer);
            ASSERT(curContainer);
            m_cmd->m_mergePrevChildCount = prevContainer->children().size();
            if (!prevContainer->children().empty()) {
              auto& lastChild = prevContainer->children().back();
              if (lastChild->type() == NodeType::text && !curContainer->children().empty() && curContainer->children().front()->type() == NodeType::text) {
                m_cmd->m_mergePrevLastTextLen = static_cast<Text*>(lastChild.get())->toString(m_doc->bufferProvider()).length();
              }
            }
            m_cmd->m_undoAction = RemoveTextCommand::UndoAction::block_merge;
          }
          const auto& prevBlock = m_doc->blocks()[coord.blockNo - 1];
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
      if (textNode->empty()) {
        if (textNode->parent() == node) {
          node->removeChild(textNode);
        } else {
          node->removeChild(textNode->parent());
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
    const auto& block = m_doc->blocks()[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    const auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty() || (coord.lineNo == 0 && coord.offset == 0)) {
      DEBUG << "degrade header to paragraph";
      if (m_cmd) {
        m_cmd->m_headerLevel = node->level();
        m_cmd->m_undoAction = RemoveTextCommand::UndoAction::header_degrade;
      }
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
          node->removeChild(textNode->parent());
        }
      }
      endRemoveText();
    }
  }
  void visit(OrderedList* node) override { removeTextInListNode(node); }
  void visit(UnorderedList* node) override { removeTextInListNode(node); }
  void visit(CheckboxList* node) override { removeTextInListNode(node); }
  void visit(CodeBlock* node) override {
    const auto& block = m_doc->blocks()[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);
    if (line.empty()) {
      if (coord.lineNo > 0) {
        ASSERT(coord.lineNo > 0);
        coord.offset = block.logicalLineAt(coord.lineNo - 1).length();
        auto text1 = node->childAt(coord.lineNo - 1);
        ASSERT(text1->type() == NodeType::text);
        auto text1Node = static_cast<Text*>(text1);
        auto text2 = node->childAt(coord.lineNo);
        ASSERT(text2->type() == NodeType::text);
        auto text2Node = static_cast<Text*>(text2);
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
    const auto& block = m_doc->blocks()[coord.blockNo];
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
    if (leftOffset == 0) return;
    auto s = textNode->toString(m_doc->bufferProvider());
    auto prevStart = ::md::previousCodePointStart(s.toStdString(), leftOffset);
    SizeType charLen = leftOffset - prevStart;
    if (m_cmd && m_cmd->m_deletedText.isEmpty()) {
      m_cmd->m_deletedText = s.mid(prevStart, charLen);
      m_cmd->m_undoAction = RemoveTextCommand::UndoAction::text_delete;
    }
    textNode->remove(prevStart, charLen);
    coord.offset -= charLen;
  }
  void endRemoveText() {
    renderBlock(coord.blockNo);
    updateCursor(cursor, coord);
  }

  RemoveTextCommand* m_cmd = nullptr;
};

class InsertTextVisitor
    : public NodeVisitor,
      public DocumentOperationVisitor {
 public:
  InsertTextVisitor(Cursor& cursor, Document* doc, SizeType offset, SizeType length, SizeType cursorOffsetDelta,
                    bool isSpace, bool maySkipChar, String targetSkipChar, InsertTextCommand* cmd = nullptr)
      : DocumentOperationVisitor(cursor, doc),
        offset(offset),
        length(length),
        cursorOffsetDelta(cursorOffsetDelta),
        isSpace(isSpace),
        maySkipChar(maySkipChar),
        targetSkipChar(targetSkipChar),
        m_cmd(cmd) {}
  void visit(Paragraph* node) override {
    auto coord = cursor.coord();
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->blocks().size());
    const auto& block = m_doc->blocks()[coord.blockNo];
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
        auto prefix = line.left(coord.offset, m_doc->bufferProvider());
        if (!prefix.isEmpty() && prefix.count('#') == prefix.size()) {
          // 说明是header
          // 将段落转换为标题
          if (m_cmd) {
            m_cmd->m_hasBlockConversion = true;
            m_cmd->m_convertedBlockType = NodeType::header;
            m_cmd->m_headerLevel = prefix.size();
          }
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
          if (m_cmd) {
            m_cmd->m_hasBlockConversion = true;
            m_cmd->m_convertedBlockType = NodeType::ul;
          }
          auto ul = std::make_unique<UnorderedList>();
          auto ulItem = std::make_unique<UnorderedListItem>();
          auto* ulItemRaw = ulItem.get();
          ul->appendChild(std::move(ulItem));
          ulItemRaw->appendChildren(std::move(node->children()));
          textNode->remove(0, 1);
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            ASSERT(textNode->parent() == ulItemRaw);
            ulItemRaw->removeChildAt(0);
          }
          replaceBlock(coord.blockNo, std::move(ul));
          coord.offset = 0;
          updateCursor(cursor, coord);
          return;
        } else if (prefix == "1.") {
          // 说明是有序列表
          // 将段落转换为有序列表
          if (m_cmd) {
            m_cmd->m_hasBlockConversion = true;
            m_cmd->m_convertedBlockType = NodeType::ol;
          }
          auto ol = std::make_unique<OrderedList>();
          auto olItem = std::make_unique<OrderedListItem>();
          auto* olItemRaw = olItem.get();
          ol->appendChild(std::move(olItem));
          olItemRaw->appendChildren(std::move(node->children()));
          textNode->remove(0, 2);
          if (textNode->empty()) {
            // 如果变成空文本了，删除这个结点
            ASSERT(textNode->parent() == olItemRaw);
            olItemRaw->removeChildAt(0);
          }
          replaceBlock(coord.blockNo, std::move(ol));
          coord.offset = 0;
          updateCursor(cursor, coord);
          return;
        } else if (prefix == ">") {
          // 说明是引用块
          // 将段落转换为引用块
          if (m_cmd) {
            m_cmd->m_hasBlockConversion = true;
            m_cmd->m_convertedBlockType = NodeType::quote_block;
          }
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
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->blocks().size());
    const auto& block = m_doc->blocks()[coord.blockNo];
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
  void visit(OrderedList* node) override { insertTextInListHelper(node); }
  void visit(CheckboxList* node) override { insertTextInListHelper(node); }
  void visit(UnorderedList* node) override {
    auto coord = cursor.coord();
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->blocks().size());
    const auto& block = m_doc->blocks()[coord.blockNo];
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
    String s = line.left(coord.offset, m_doc->bufferProvider());
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
          auto s = textNode->toString(m_doc->bufferProvider());
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
      } else if (coord.lineNo == block.countOfLogicalLine() - 1) {
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
  void visit(CodeBlock* node) override {
    auto coord = cursor.coord();
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->blocks().size());
    const auto& block = m_doc->blocks()[coord.blockNo];
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
      auto s = textNode->toString(m_doc->bufferProvider());
      ASSERT(leftOffset >= 0 && leftOffset < s.size());
      auto ch = s[leftOffset];
      if (targetSkipChar.size() == 1 && ch == targetSkipChar[0]) {
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
  void insertTextInListHelper(Container* node) {
    auto coord = cursor.coord();
    ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->blocks().size());
    const auto& block = m_doc->blocks()[coord.blockNo];
    ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
    auto& line = block.logicalLineAt(coord.lineNo);

    if (line.empty()) {
      insertTextInEmptyList(node);
      return;
    }
    auto [textNode, leftOffset] = line.textAt(coord.offset);
    insertTextInNode(textNode, leftOffset);
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
  InsertTextCommand* m_cmd = nullptr;
};

void InsertTextCommand::undo(Cursor& cursor) {
  if (m_hasBlockConversion) {
    auto node = m_doc->root()->childAt(m_coord.blockNo);
    auto paragraph = std::make_unique<Paragraph>();
    switch (m_convertedBlockType) {
      case NodeType::header: {
        auto headerNode = static_cast<Header*>(node);
        paragraph->setChildren(std::move(headerNode->children()));
        break;
      }
      case NodeType::ul: {
        auto ulNode = static_cast<UnorderedList*>(node);
        if (!ulNode->children().empty()) {
          auto item = static_cast<Container*>(ulNode->childAt(0));
          paragraph->setChildren(std::move(item->children()));
        }
        break;
      }
      case NodeType::ol: {
        auto olNode = static_cast<OrderedList*>(node);
        if (!olNode->children().empty()) {
          auto item = static_cast<Container*>(olNode->childAt(0));
          paragraph->setChildren(std::move(item->children()));
        }
        break;
      }
      case NodeType::quote_block: {
        auto quoteNode = static_cast<QuoteBlock*>(node);
        paragraph->setChildren(std::move(quoteNode->children()));
        break;
      }
      default:
        ASSERT(false && "unknown converted block type");
        break;
    }
    m_doc->replaceBlock(m_coord.blockNo, std::move(paragraph));
    m_doc->updateCursor(cursor, m_coord);
    return;
  }

  DEBUG << m_coord << m_finishedCoord;
  m_doc->updateCursor(cursor, m_finishedCoord);
  while (cursor.coord() != m_coord) {
    RemoveTextVisitor visitor(cursor, m_doc);
    auto node = m_doc->root()->childAt(m_coord.blockNo);
    node->accept(&visitor);
  }
}
void InsertTextCommand::execute(Cursor& cursor) {
  m_doc->updateCursor(cursor, m_coord);
  InsertTextVisitor visitor(cursor, m_doc, m_offset, m_length, m_cursorOffsetDelta, m_isSpace, m_maySkipChar,
                            m_targetSkipChar, this);
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
  m_offset = m_doc->appendToAddBuffer(textToInsert);
  m_length = textToInsert.size();
  m_cursorOffsetDelta = text.size();
}
bool InsertTextCommand::merge(Command* command) {
  if (this->type() != command->type()) return false;
  auto insertTextCommand = (InsertTextCommand*)command;
  if (insertTextCommand->m_hasBlockConversion) return false;
  if (m_finishedCoord != insertTextCommand->m_coord) return false;
  if (m_offset + m_length != insertTextCommand->m_offset) return false;
  m_length += insertTextCommand->m_length;
  m_finishedCoord = insertTextCommand->m_finishedCoord;
  return true;
}
void RemoveTextCommand::execute(Cursor& cursor) {
  m_doc->updateCursor(cursor, m_coord);
  RemoveTextVisitor visitor(cursor, m_doc, this);
  auto node = m_doc->root()->childAt(cursor.coord().blockNo);
  node->accept(&visitor);
  m_finishedCoord = cursor.coord();
}
void RemoveTextCommand::undo(Cursor& cursor) {
  if (m_undoAction == RemoveTextCommand::UndoAction::none) {
    DEBUG << "RemoveTextCommand::undo(): nothing recorded";
    return;
  }

  if (m_undoAction == RemoveTextCommand::UndoAction::text_delete) {
    if (m_deletedText.isEmpty()) {
      DEBUG << "RemoveTextCommand::undo(): text_delete but deleted text is empty";
      return;
    }

    SizeType offset = m_doc->appendToAddBuffer(m_deletedText);

    m_doc->updateCursor(cursor, m_finishedCoord);

    InsertTextVisitor visitor(
        cursor, m_doc,
        offset,
        m_deletedText.length(),
        m_deletedText.length(),  // cursorOffsetDelta = text length
        false,                   // isSpace = false
        false,                   // maySkipChar = false
        "");                     // targetSkipChar (unused)
    auto node = m_doc->root()->childAt(cursor.coord().blockNo);
    node->accept(&visitor);

    m_doc->updateCursor(cursor, m_coord);
    return;
  }

  if (m_undoAction == RemoveTextCommand::UndoAction::block_merge) {
    auto blockNo = m_finishedCoord.blockNo;
    auto container = m_doc->node2container(
        m_doc->root()->childAt(blockNo));
    ASSERT(container);
    SizeType splitIndex = m_mergePrevChildCount;

    if (m_mergePrevLastTextLen > 0) {
      SizeType textIdx = m_mergePrevChildCount - 1;
      ASSERT(textIdx >= 0 && textIdx < container->children().size());
      auto& child = container->children()[textIdx];
      ASSERT(child->type() == NodeType::text);
      auto textNode = static_cast<Text*>(child.get());
      auto splitResult = textNode->split(m_mergePrevLastTextLen);
      container->setChild(textIdx, std::move(splitResult.first));
      container->insertChild(textIdx + 1, std::move(splitResult.second));
    }

    auto newBlock = std::make_unique<Paragraph>();
    SizeType childCount = container->children().size();
    for (SizeType i = splitIndex; i < childCount; ++i) {
      newBlock->appendChild(std::move(container->children()[i]));
    }
    ASSERT(newBlock->children().size() == childCount - splitIndex);
    container->children().resize(splitIndex);

    m_doc->insertBlock(blockNo + 1, std::move(newBlock));
    m_doc->renderBlock(blockNo);

    m_doc->updateCursor(cursor, m_coord);
    return;
  }

  if (m_undoAction == RemoveTextCommand::UndoAction::header_degrade) {
    auto node = m_doc->root()->childAt(m_coord.blockNo);
    ASSERT(node->type() == NodeType::paragraph);
    auto paragraphNode = static_cast<Paragraph*>(node);
    auto header = std::make_unique<Header>(m_headerLevel);
    header->setChildren(std::move(paragraphNode->children()));
    m_doc->replaceBlock(m_coord.blockNo, std::move(header));

    m_doc->updateCursor(cursor, m_coord);
    return;
  }

  DEBUG << "RemoveTextCommand::undo(): unhandled UndoAction";
}
void InsertReturnCommand::execute(Cursor& cursor) {
  m_doc->updateCursor(cursor, m_coord);
  auto coord = m_coord;
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_doc->blocks().size());
  const auto& block = m_doc->blocks()[coord.blockNo];
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
UpgradeToHeaderCommand::UpgradeToHeaderCommand(Document* doc, CursorCoord coord, int level)
    : Command(doc), m_coord(coord), m_level(level) {}
void UpgradeToHeaderCommand::execute(Cursor& cursor) {
  auto node = m_doc->root()->childAt(m_coord.blockNo);
  if (node->type() != NodeType::paragraph) return;
  auto paragraphNode = static_cast<Paragraph*>(node);
  auto header = std::make_unique<Header>(m_level);
  header->setChildren(std::move(paragraphNode->children()));
  m_doc->replaceBlock(m_coord.blockNo, std::move(header));
  m_doc->ensureTrailingParagraph();
  m_finishedCoord = CursorCoord{m_coord.blockNo, 0, 0};
  m_doc->updateCursor(cursor, m_finishedCoord);
}
void UpgradeToHeaderCommand::undo(Cursor& cursor) {
  auto node = m_doc->root()->childAt(m_coord.blockNo);
  ASSERT(node->type() == NodeType::header);
  auto headerNode = static_cast<Header*>(node);
  auto paragraph = std::make_unique<Paragraph>();
  paragraph->setChildren(std::move(headerNode->children()));
  m_doc->replaceBlock(m_coord.blockNo, std::move(paragraph));
  m_doc->ensureTrailingParagraph();
  m_doc->updateCursor(cursor, m_coord);
}
RemoveTextRangeCommand::RemoveTextRangeCommand(Document* doc, CursorCoord begin, CursorCoord end)
    : Command(doc), m_begin(begin), m_end(end) {}
void RemoveTextRangeCommand::execute(Cursor& cursor) {
  if (m_end < m_begin) {
    std::swap(m_begin, m_end);
  }
  CursorCoord coord = m_end;

  // Phase 1: same block, same line, same Text node -- bulk delete
  if (m_begin.blockNo == m_end.blockNo) {
    const auto& block = m_doc->blocks()[m_begin.blockNo];
    if (m_begin.lineNo == m_end.lineNo) {
      const auto& line = block.logicalLineAt(m_begin.lineNo);
      auto beginPair = line.textAt(m_begin.offset);
      auto endPair = line.textAt(m_end.offset);
      if (beginPair.first && beginPair.first == endPair.first) {
        SizeType length = m_end.offset - m_begin.offset;
        if (length > 0) {
          m_deletedText = beginPair.first->toString(m_doc->bufferProvider()).mid(beginPair.second, length);
          beginPair.first->remove(beginPair.second, length);
          m_doc->renderBlock(m_begin.blockNo);
          m_hasAction = true;
          m_doc->updateCursor(cursor, m_begin);
          return;
        }
      }
    }
  }

  // Phase 2: character-by-character in reverse
  m_doc->updateCursor(cursor, m_end);
  String reversed;
  while (!(coord == m_begin)) {
    const auto& block = m_doc->blocks()[coord.blockNo];
    const auto& line = block.logicalLineAt(coord.lineNo);
    auto [textNode, textOffset] = line.textAt(coord.offset);
    if (textNode && textOffset > 0) {
      String ch = textNode->toString(m_doc->bufferProvider()).mid(textOffset - 1, 1);
      reversed += ch;
    }
    RemoveTextVisitor visitor(cursor, m_doc, nullptr);
    auto node = m_doc->root()->childAt(coord.blockNo);
    node->accept(&visitor);
    m_doc->updateCursor(cursor, cursor.coord());
    coord = cursor.coord();
  }
  std::reverse(reversed.begin(), reversed.end());
  m_deletedText = reversed;
  m_hasAction = !m_deletedText.isEmpty();
}
void RemoveTextRangeCommand::undo(Cursor& cursor) {
  if (m_deletedText.isEmpty()) return;
  m_doc->updateCursor(cursor, m_begin);

  SizeType addOffset = m_doc->appendToAddBuffer(m_deletedText);

  InsertTextVisitor visitor(
      cursor, m_doc,
      addOffset,
      m_deletedText.size(),
      m_deletedText.size(),
      false,
      false,
      "");
  auto node = m_doc->root()->childAt(cursor.coord().blockNo);
  node->accept(&visitor);
  m_doc->updateCursor(cursor, m_begin);
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
      if (m_commands.size() >= kMaxCommands) {
        m_commands.erase(m_commands.begin());
        if (m_top > 0) --m_top;
      }
      m_commands.push_back(std::move(command));
    }
  }
  m_top = m_commands.size();
}
void CommandStack::undo(Cursor& cursor) {
  ASSERT(m_top <= m_commands.size());
  if (m_top == 0) return;
  m_commands[m_top - 1]->undo(cursor);
  m_top--;
}
void CommandStack::redo(Cursor& cursor) {
  ASSERT(m_top <= m_commands.size());
  if (m_top == m_commands.size()) return;
  m_commands[m_top]->execute(cursor);
  m_top++;
}
}  // namespace md::editor
