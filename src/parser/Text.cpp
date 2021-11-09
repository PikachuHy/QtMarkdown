//
// Created by PikachuHy on 2021/11/5.
//

#include "Text.h"

#include "debug.h"
namespace md::parser {
String Text::toString(const DocPtr& doc) const {
  String s;
  for (auto item : m_items) {
    s += item.toString(doc);
  }
  return s;
}
void Text::insert(SizeType totalOffset, PieceTableItem item) {
  int i = 0;
  SizeType curOffset = 0;
  while (i < m_items.size()) {
    if (curOffset <= totalOffset && totalOffset <= curOffset + m_items[i].length) {
      // 判断头和尾
      if (curOffset == totalOffset) {
        // 头
        m_items.insert(i, item);
      } else if (curOffset + m_items[i].length == totalOffset) {
        // 尾
        // 判断是不是能合并
        if (item.bufferType == m_items[i].bufferType && m_items[i].offset + m_items[i].length == item.offset) {
          m_items[i].length += item.length;
        } else {
          m_items.insert(i + 1, item);
        }
      } else {
        // 中间
        // 拆分为两个
        auto oldLength = totalOffset - curOffset;
        auto item2Length = m_items[i].length - oldLength;
        m_items[i].length = oldLength;
        PieceTableItem item2{m_items[i].bufferType, m_items[i].offset + m_items[i].length, item2Length};
        m_items.insert(i + 1, item);
        m_items.insert(i + 2, item2);
      }
      return;
    }
    curOffset += m_items[i].length;
    i++;
  }
}
void Text::remove(SizeType totalOffset, SizeType length) {
  ASSERT(totalOffset >= 0);
  // 删除时，右边必须时开区间，因为是从当前的这个offset往后算
  auto [i, curOffset] = findItem(totalOffset, false);
  auto leftOffset = totalOffset - curOffset;
  ASSERT(i >= 0 && i < m_items.size());
  if (leftOffset + length > m_items[i].length) {
    auto leftLength = length - m_items[i].length;
    m_items.removeAt(i);
    remove(totalOffset, leftLength);
  } else {
    // 然后这里也要判断是头 尾 还是中间
    if (curOffset == totalOffset) {
      // 如果刚好整个item需要删除
      if (curOffset + m_items[i].length == totalOffset + length) {
        m_items.remove(i);
      } else {
        m_items[i].offset += length;
      }
    } else if (curOffset + m_items[i].length == totalOffset + length) {
      m_items[i].length -= length;
    } else {
      // 拆
      auto oldLength = totalOffset - curOffset;
      auto item2Length = m_items[i].length - oldLength - length;
      m_items[i].length = oldLength;
      PieceTableItem item2{m_items[i].bufferType, m_items[i].offset + m_items[i].length + length, item2Length};
      m_items.insert(i + 1, item2);
    }
  }
}
std::pair<SizeType, SizeType> Text::findItem(SizeType totalOffset, bool includeRight) const {
  SizeType curOffset = 0;
  for (SizeType i = 0; i < m_items.size(); ++i) {
    if (includeRight) {
      if (curOffset <= totalOffset && totalOffset <= curOffset + m_items[i].length) {
        return {i, curOffset};
      }
    } else {
      if (curOffset <= totalOffset && totalOffset < curOffset + m_items[i].length) {
        return {i, curOffset};
      }
    }
    curOffset += m_items[i].length;
  }
  ASSERT(false && "no item find");
}
std::pair<Text*, Text*> Text::split(SizeType totalOffset) {
  ASSERT(totalOffset >= 0);
  auto [splitIndex, curOffset] = findItem(totalOffset);
  auto leftOffset = totalOffset - curOffset;
  auto leftText = new Text();
  auto rightText = new Text();
  for (int i = 0; i < splitIndex - 1; ++i) {
    leftText->m_items.append(m_items[i]);
  }
  // splitIndex所在item要拆成两个，当然，也要考虑首尾的情况
  PieceTableItem& item = m_items[splitIndex];
  if (curOffset == totalOffset) {
    rightText->m_items.append(item);
  } else if (curOffset + item.length == totalOffset) {
    leftText->m_items.append(item);
  } else {
    // 拆
    auto oldLength = totalOffset - curOffset;
    auto item2Length = item.length - oldLength;
    item.length = oldLength;
    leftText->m_items.append(item);
    PieceTableItem item2{item.bufferType, item.offset + item.length, item2Length};
    rightText->m_items.append(item2);
  }
  for (SizeType i = splitIndex + 1; i < m_items.size(); ++i) {
    rightText->m_items.append(m_items[i]);
  }
  return {leftText, rightText};
}
bool Text::empty() const {
  if (m_items.empty()) return true;
  for (auto item : m_items) {
    if (item.length > 0) return false;
  }
  return true;
}
void Text::merge(Text& text) {
  for (auto item : text.m_items) {
    m_items.append(item);
  }
}

String LatexBlock::toString(DocPtr const& doc) const {
  String s;
  for (auto node : m_children) {
    if (node->type() == NodeType::text) {
      auto text = static_cast<Text*>(node);
      s += text->toString(doc);
    }
  }
  return s;
}
}  // namespace md::parser