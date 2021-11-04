//
// Created by pikachu on 2021/10/28.
//

#include "EditorDocument.h"

#include <QFont>
#include <QRect>
#include <QString>
#include <utility>

#include "Cursor.h"
#include "Document.h"
#include "Parser.h"
#include "PieceTable.h"
#include "Render.h"
#include "debug.h"
using namespace md::parser;
EditorDocument::EditorDocument(const QString &text) : m_originalBuffer(text) {
  m_doc = Parser::parse(text);
}
void LineData::appendCell(const Cell &cell) { m_cells.append(cell); }
bool LineData::contains(Cursor &cursor) {
  auto pos = cursor.pos();
  return std::any_of(m_cells.begin(), m_cells.end(), [pos](const auto &cell) {
    auto rect = cell.rect;
    // 只需要计算y是不是在两个值之间
    auto y1 = rect.y();
    auto y2 = rect.y() + rect.height();
    if (y1 <= pos.y() && pos.y() < y2) {
      return true;
    }
    return false;
  });
}
std::pair<qsizetype, qsizetype> LineData::lastCoord() {
  auto cellNo = m_cells.size() - 1;
  auto offset = m_cells[cellNo].text.size();
  return {cellNo, offset};
}
void EditorDocument::createNewLineData() {
  auto line = new LineData();
  m_lineData.append(line);
}
void EditorDocument::updateCursor(Cursor *cursor) {
  if (!cursor) return;
  auto coord = cursor->coord();
  auto cell = m_lineData[coord.lineNo]->cells()[coord.cellNo];
  if (cursor->offset() == 0) {
    cursor->moveTo(cell.rect.x(), cell.rect.y());
  } else {
    auto text = cell.text.left(cursor->offset());
    QFontMetrics fm(cell.font);
    auto w = fm.horizontalAdvance(text);
    cursor->moveTo(cell.rect.x() + w, cell.rect.y());
  }
  cursor->updateHeight(cell.rect.height());
}
void EditorDocument::moveCursorLeft(Cursor *cursor, qsizetype length,
                                    bool ignoreCell) {
  if (!cursor) return;
  auto coord = cursor->coord();
  while (length > 0) {
    if (coord.offset > 0) {
      // 考虑cell的长度刚好等于左移动的长度
      auto cell = m_lineData[coord.lineNo]->cells()[coord.cellNo];
      if (cell.text.size() == length && coord.offset == length) {
        if (ignoreCell) {
          coord.offset = 0;
          // 就设置为上一个cell的最后一个位置
          cursor->setCursorCoord(coord);
          moveCursorLeft(cursor, 1, false);
          return;
        }
      } else {
        coord.offset--;
        length--;
        continue;
      }
    }
    if (coord.cellNo > 0) {
      coord.cellNo--;
      coord.offset =
          m_lineData[coord.lineNo]->cells()[coord.cellNo].text.size();
      if (!ignoreCell) {
        length--;
      }
      continue;
    }
    if (coord.lineNo > 0) {
      coord.lineNo--;
      coord.cellNo = m_lineData[coord.lineNo]->cells().size() - 1;
      coord.offset =
          m_lineData[coord.lineNo]->cells()[coord.cellNo].text.size();
      if (!ignoreCell) {
        length--;
      }
      continue;
    }
  }
  cursor->setCursorCoord(coord);
  // 如果已经在第一行的行首，不用处理
}
void EditorDocument::moveCursorRight(Cursor *cursor, qsizetype length,
                                     bool ignoreCell) {
  if (!cursor) return;
  auto coord = cursor->coord();
  while (length > 0) {
    if (coord.offset <
        m_lineData[coord.lineNo]->cells()[coord.cellNo].text.size()) {
      coord.offset++;
      length--;
      continue;
    }
    if (coord.cellNo < m_lineData[coord.lineNo]->cells().size() - 1) {
      coord.cellNo++;
      coord.offset = 0;
      if (!ignoreCell) {
        length--;
      }
      continue;
    }
    if (coord.lineNo < m_lineData.size() - 1) {
      coord.lineNo++;
      coord.cellNo = 0;
      coord.offset = 0;
      length--;
      continue;
    }
  }
  // 如果已经在最后一行的最后一个位置，不用处理
  cursor->setCursorCoord(coord);
}
void EditorDocument::moveCursorDown(Cursor *cursor) {
  if (!cursor) return;
  auto coord = cursor->coord();
  if (coord.lineNo < m_lineData.size() - 1) {
    // 先x不变，去下一行里找y
    coord.lineNo++;
    auto line = m_lineData[coord.lineNo];
    bool findCell = false;
    for (int i = 0; i < line->cells().size(); ++i) {
      auto cell = line->cells()[i];
      auto rect = cell.rect;
      bool hasFixPos = false;
      if (rect.x() <= cursor->x() && cursor->x() < rect.x() + rect.width()) {
        // 修正光标的x,y值
        auto newX = rect.x();
        QFontMetrics fm(cell.font);
        for (int j = 0; j < cell.text.size(); j++) {
          auto ch = cell.text[j];
          auto w = fm.horizontalAdvance(ch);
          if (newX <= cursor->x() && cursor->x() <= newX + w) {
            coord.cellNo = i;
            if (cursor->x() - newX > w / 2) {
              coord.offset = j + 1;
            } else {
              coord.offset = j;
            }
            cursor->setCursorCoord(coord);
            cursor->moveTo(newX + w - 1, rect.y());
            cursor->updateHeight(rect.height());
            hasFixPos = true;
            break;
          }
          newX += w;
        }
        if (!hasFixPos) continue;
        findCell = true;
        break;
      }
    }
    if (!findCell) {
      // 如果没找到cell，就取该行最后一个位置
      auto [cellNo, offset] = line->lastCoord();
      coord.cellNo = cellNo;
      coord.offset = offset;
      cursor->setCursorCoord(coord);
    }
    return;
  }
  // 如果已经在最后一行了，直接去到最后一行的最后一个位置
  auto line = m_lineData[coord.lineNo];
  coord.cellNo = line->cells().size() - 1;
  coord.offset = line->cells()[coord.cellNo].text.size();
  cursor->setCursorCoord(coord);
}
void EditorDocument::moveCursorUp(Cursor *cursor) {
  if (!cursor) return;
  auto coord = cursor->coord();
  if (coord.lineNo > 0) {
    // 先x不变，去下一行里找y
    coord.lineNo--;
    auto line = m_lineData[coord.lineNo];
    bool findCell = false;
    for (int i = 0; i < line->cells().size(); ++i) {
      auto cell = line->cells()[i];
      auto rect = cell.rect;
      if (rect.x() <= cursor->x() && cursor->x() <= rect.x() + rect.width()) {
        // 修正光标的x,y值
        auto newX = rect.x();
        QFontMetrics fm(cell.font);
        bool hasFixPos = false;
        for (int j = 0; j < cell.text.size(); j++) {
          auto ch = cell.text[j];
          auto w = fm.horizontalAdvance(ch);
          if (newX <= cursor->x() && cursor->x() <= newX + w) {
            coord.cellNo = i;
            if (cursor->x() - newX > w / 2) {
              coord.offset = j + 1;
            } else {
              coord.offset = j;
            }
            cursor->setCursorCoord(coord);
            cursor->moveTo(newX + w - 1, rect.y());
            cursor->updateHeight(rect.height());
            hasFixPos = true;
            break;
          }
          newX += w;
        }
        if (!hasFixPos) continue;
        findCell = true;
        break;
      }
    }
    if (!findCell) {
      // 如果没找到cell，就取该行最后一个位置
      auto [cellNo, offset] = line->lastCoord();
      coord.cellNo = cellNo;
      coord.offset = offset;
      cursor->setCursorCoord(coord);
    }
    return;
  }
  // 如果已经在第一行了，直接去到第一行的第一个位置
  coord.cellNo = 0;
  coord.offset = 0;
  cursor->setCursorCoord(coord);
}
void EditorDocument::fixCursorPos(Cursor *cursor) {
  if (!cursor) return;
  auto pos = cursor->pos();
  auto coord = cursor->coord();
  for (int i = 0; i < m_lineData.size(); ++i) {
    auto line = m_lineData[i];
    if (!line->contains(*cursor)) continue;
    coord.lineNo = i;
    // 修正光标的x,y值
    bool hasFixCursor = false;
    for (int cellNo = 0; cellNo < line->cells().size(); ++cellNo) {
      auto cell = line->cells()[cellNo];
      auto newX = cell.rect.x();
      QFontMetrics fm(cell.font);
      for (int j = 0; j < cell.text.size(); j++) {
        auto ch = cell.text[j];
        auto w = fm.horizontalAdvance(ch);
        if (newX <= pos.x() && pos.x() < newX + w) {
          // 找到所在的文字后，移动到文字后面
          cursor->moveTo(newX + w - 1, cell.rect.y());
          cursor->updateHeight(cell.rect.height());
          hasFixCursor = true;
          coord.cellNo = cellNo;
          coord.offset = j + 1;
          break;
        }
        newX += w;
      }
    }
    if (!hasFixCursor) {
      // 如果没有修正，那么取最后一个矩形的坐标
      if (!line->cells().isEmpty()) {
        auto rect = line->cells().back().rect;
        cursor->moveTo(rect.x() + rect.width(), rect.y());
        cursor->updateHeight(rect.height());
        auto [cellNo, offset] = line->lastCoord();
        coord.cellNo = cellNo;
        coord.offset = offset;
      }
    }
    break;
  }
  cursor->setCursorCoord(coord);
}
void EditorDocument::draw(Render *render) {
  if (!render) return;
  m_lineData.clear();
  m_doc->accept(render);
}
void EditorDocument::appendCell(const Cell &cell) {
  m_lineData.back()->appendCell(cell);
}

void EditorDocument::insertText(QString text, Cursor &cursor) {
  // 要先算出cell在原Text中的偏移量
  auto coord = cursor.coord();
  auto cell = m_lineData[coord.lineNo]->cells()[coord.cellNo];
  if (!cell.table) {
    DEBUG << "table in nullptr";
    return;
  }
  auto addOffset = m_addBuffer.size();
  auto addLength = text.length();
  m_addBuffer.append(text);
  auto offset = cell.totalOffset + coord.offset;
  cell.table->insert(offset, addOffset, addLength);
}

PieceTable *EditorDocument::pieceTable(Text *text) {
  if (!text) return nullptr;
  if (m_text2pieceTable.contains(text)) {
    return m_text2pieceTable[text];
  }
  auto table = new PieceTable(*this, text->offset(), text->length());
  m_text2pieceTable[text] = table;
  return table;
}

void EditorDocument::removeText(qsizetype length, Cursor &cursor) {
  // 要先算出cell在原Text中的偏移量
  auto coord = cursor.coord();
  auto cell = m_lineData[coord.lineNo]->cells()[coord.cellNo];
  if (!cell.table) {
    DEBUG << "table is nullptr";
    return;
  }
  auto offset = cell.totalOffset + coord.offset;
  cell.table->remove(offset - length, length);
}
void EditorDocument::insertReturn(Cursor &cursor) {
  auto coord = cursor.coord();
  auto cell = m_lineData[coord.lineNo]->cells()[coord.cellNo];
  if (!cell.node) {
    DEBUG << "node is nullptr";
    return;
  }
  if (!cell.node->parent()) {
    DEBUG << "node parent is nullptr";
    return;
  }
  auto parentNode = cell.node->parent();
  if (parentNode->type() == NodeType::paragraph) {
  }
}
QString EditorDocument::text2str(md::parser::Text *text) {
  if (!text) {
    DEBUG << "text is nullptr";
    return {};
  }
  QString s;
  auto table = pieceTable(text);
  for (const auto &item : *table) {
    s += item;
  }
  return s;
}
