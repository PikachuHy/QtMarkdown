//
// Created by PikachuHy on 2021/11/2.
//

#include "PieceTable.h"

#include "Document.h"
#include "debug.h"
namespace md::parser {
String PieceTableItem::toString(const DocPtr& doc) const {
  auto s = bufferType == original ? doc->m_originalBuffer.mid(offset, length) : doc->m_addBuffer.mid(offset, length);
  if (s.endsWith("\n")) {
    DEBUG << "换行";
  }
  return s;
}
QDebug operator<<(QDebug debug, const PieceTableItem& item) {
  QDebugStateSaver saver(debug);
  debug.nospace() << '(' << (item.bufferType == PieceTableItem::original ? "original" : "add") << ", " << item.offset
                  << ", " << item.length << ')';

  return debug;
}
}  // namespace md::parser
#if 0
PieceTable::PieceTable(EditorDocument& doc, qsizetype offset, qsizetype length)
    : m_doc(doc) {
  m_cells.emplace_back(
      PieceTableItem{PieceTableItem::original, offset, length});
}

void PieceTable::insert(qsizetype offset, qsizetype addOffset,
                        qsizetype addLength) {
  // 定位到是哪个表项
  qsizetype i = 0;
  qsizetype totalOffset = 0;
  while (i < m_cells.size()) {
    auto& item = m_cells[i];
    if (totalOffset > offset) {
      // 理论上不应该出现这种情况
      DEBUG << "ERROR";
      return;
    }
    if (offset > totalOffset + item.length) {
      totalOffset += item.length;
      i++;
      continue;
    }
    // 找到表项，拆分
    // 考虑刚好位于开头和结尾的情况
    if (totalOffset == offset) {
      m_cells.insert(i,
                     PieceTableItem{PieceTableItem::add, addOffset, addLength});
    } else if (totalOffset + item.length == offset) {
      // 考虑都是add buffer的情况
      if (item.offset + item.length == addOffset) {
        item.length += addLength;
      } else {
        m_cells.insert(
            i + 1, PieceTableItem{PieceTableItem::add, addOffset, addLength});
      }
    } else {
      // 位于中间就要拆
      auto originalOffset = offset - totalOffset + item.offset;
      auto originalLength = item.length - (offset - totalOffset);
      item.length = offset - totalOffset;
      m_cells.insert(i + 1, PieceTableItem{PieceTableItem::original,
                                           originalOffset, originalLength});
      m_cells.insert(i + 1,
                     PieceTableItem{PieceTableItem::add, addOffset, addLength});
    }
    return;
  }
}

void PieceTable::remove(qsizetype offset, qsizetype length) {
  // 定位到是哪个表项
  qsizetype i = 0;
  qsizetype totalOffset = 0;
  while (i < m_cells.size()) {
    auto& item = m_cells[i];
    if (totalOffset > offset) {
      // 理论上不应该出现这种情况
      DEBUG << "ERROR";
      return;
    }
    if (totalOffset + item.length <= offset) {
      totalOffset += item.length;
      i++;
      continue;
    }
    // 同样需要考虑开头和结尾
    if (totalOffset == offset) {
      // 直接把offset往后移动就行
      if (item.length == length) {
        // 如果这个删完了
        // 直接从表中删除条目
        m_cells.removeAt(i);
      } else {
        item.offset += length;
      }
    } else if (totalOffset + item.length == offset + length) {
      // 减小长度
      item.length -= length;
    } else {
      // 在中间就需要拆
      auto originalOffset = item.offset + (offset - totalOffset) + length;
      auto originalLength = item.length - (offset - totalOffset) - length;
      m_cells.insert(i + 1, PieceTableItem{item.bufferType, originalOffset,
                                           originalLength});
      item.length = offset - totalOffset;
    }
    break;
  }
}

PieceTable::iterator PieceTable::begin() { return {this, 0}; }

PieceTable::iterator PieceTable::end() { return {this, m_cells.size()}; }

QString PieceTable::itemString(qsizetype index) {
  if (index < 0 || index > m_cells.size()) {
    DEBUG << "ERROR"
          << "index" << index << "max" << m_cells.size();
    return "";
  }
  const auto& item = m_cells[index];
  if (item.bufferType == PieceTableItem::original) {
    return m_doc.m_originalBuffer.mid(item.offset, item.length);
  } else {
    return m_doc.m_addBuffer.mid(item.offset, item.length);
  }
}

QString PieceTableIterator::operator*() { return m_table->itemString(m_index); }
#endif