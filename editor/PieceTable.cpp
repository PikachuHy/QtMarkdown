//
// Created by PikachuHy on 2021/11/2.
//

#include "PieceTable.h"
#include "debug.h"
PieceTable::PieceTable(QString text): m_originalBuffer(text) {
    m_items.append(PieceTableItem{PieceTableItem::original, 0, m_originalBuffer.size()});
}

void PieceTable::insert(QString text, qsizetype offset) {
    // 定位到是哪个表项
    qsizetype i = 0;
    qsizetype totalOffset = 0;
    while (i < m_items.size()) {
        auto& item = m_items[i];
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
            auto addOffset = m_addBuffer.size();
            m_addBuffer.append(text);
            m_items.insert(i, PieceTableItem{PieceTableItem::add, addOffset, text.size()});
        } else if (totalOffset + item.length == offset) {
            // 考虑都是add buffer的情况
            if (item.offset + item.length == m_addBuffer.size()) {
                item.length+=text.size();
                m_addBuffer.append(text);
            } else {
                auto addOffset = m_addBuffer.size();
                m_addBuffer.append(text);
                m_items.insert(i + 1, PieceTableItem{PieceTableItem::add, addOffset, text.size()});
            }
        } else {
            // 位于中间就要拆
            auto originalOffset = offset - totalOffset + item.offset;
            auto originalLength = item.length - (offset - totalOffset);
            item.length = offset - totalOffset;
            m_items.insert(i + 1, PieceTableItem{PieceTableItem::original, originalOffset, originalLength});
            auto addOffset = m_addBuffer.size();
            m_addBuffer.append(text);
            m_items.insert(i + 1, PieceTableItem{PieceTableItem::add, addOffset, text.size()});
        }
        return;
    }
}

void PieceTable::remove(qsizetype offset, qsizetype length) {
    // 定位到是哪个表项
    qsizetype i = 0;
    qsizetype totalOffset = 0;
    while (i < m_items.size()) {
        auto& item = m_items[i];
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
                m_items.removeAt(i);
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
            m_items.insert(i + 1, PieceTableItem{item.bufferType, originalOffset, originalLength});
            item.length = offset - totalOffset;
        }
        break;
    }
}

PieceTable::iterator PieceTable::begin() {
    return {this, 0};
}

PieceTable::iterator PieceTable::end() {
    return {this, m_items.size()};
}

QString PieceTable::itemString(qsizetype index) {
    if (index < 0 || index > m_items.size()) {
        DEBUG << "ERROR" << "index" << index << "max" << m_items.size();
        return "";
    }
    const auto& item = m_items[index];
    if (item.bufferType == PieceTableItem::original) {
        return m_originalBuffer.mid(item.offset, item.length);
    } else {
        return m_addBuffer.mid(item.offset, item.length);
    }
}

QString PieceTableIterator::operator*() {
    return m_table->itemString(m_index);
}