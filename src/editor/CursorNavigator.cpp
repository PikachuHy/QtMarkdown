//
// Created by PikachuHy on 2021/11/5.
//

#include "CursorNavigator.h"
#include "Cursor.h"
#include "core/Utf8Util.h"
#include "debug.h"
#include "parser/Text.h"

using namespace md::parser;
using namespace md::render;

namespace md::editor {

void CursorNavigator::updateCursor(Cursor& cursor, const CursorCoord& coord, bool updatePos) {
  cursor.setCoord(coord);
  if (!updatePos) return;
  auto [pos, h, ascent] = mapToScreen(coord);
  cursor.setPos(pos);
  cursor.setHeight(h);
  cursor.setAscent(ascent);
}

std::tuple<core::Point, int, int> CursorNavigator::mapToScreen(const CursorCoord& coord) {
  int y = m_setting.docMargin.top;
  for (int blockNo = 0; blockNo < coord.blockNo; ++blockNo) {
    y += m_setting.blockSpacing;
    y += m_blocks[blockNo].height();
  }
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  auto [pos, h, ascent] = line.cursorAt(coord.offset, m_doc);
  if (h == line.height()) {
    h -= m_setting.lineSpacing;
  }
  auto resultPos = pos + core::Point(0, y);
  return {resultPos, h, ascent};
}

CursorCoord CursorNavigator::moveCursorToRight(CursorCoord coord) {
  const auto& block = m_blocks[coord.blockNo];
  auto& line = block.logicalLineAt(coord.lineNo);
  SizeType totalOffset = line.length();
  if (totalOffset >= coord.offset + 1) {
    String s = line.left(coord.offset + 1, m_doc);
    auto seqLen = ::md::utf8SequenceLength(s[coord.offset]);
    coord.offset += seqLen;
  } else {
    if (coord.lineNo + 1 < block.countOfLogicalLine()) {
      coord.offset = 0;
      coord.lineNo++;
    } else {
      if (coord.blockNo + 1 < m_blocks.size()) {
        coord.blockNo++;
        coord.lineNo = 0;
        coord.offset = 0;
      }
    }
  }
  return coord;
}

CursorCoord CursorNavigator::moveCursorToLeft(CursorCoord coord) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  if (coord.offset > 0) {
    auto& line = block.logicalLineAt(coord.lineNo);
    String s = line.left(coord.offset, m_doc);
    auto prevStart = ::md::previousCodePointStart(s.toStdString(), coord.offset);
    coord.offset = prevStart;
  } else if (coord.lineNo > 0) {
    coord.lineNo--;
    coord.offset = block.logicalLineAt(coord.lineNo).length();
  } else if (coord.blockNo > 0) {
    coord.blockNo--;
    coord.lineNo = m_blocks[coord.blockNo].countOfLogicalLine() - 1;
    coord.offset = m_blocks[coord.blockNo].logicalLineAt(coord.lineNo).length();
  } else {
    DEBUG << "do nothing";
  }
  return coord;
}

CursorCoord CursorNavigator::moveCursorToUp(CursorCoord coord, core::Point pos) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  int x = pos.x;
  if (line.canMoveUp(coord.offset, m_doc)) {
    coord.offset = line.moveUp(coord.offset, x, m_doc);
  } else {
    if (coord.lineNo > 0) {
      coord.lineNo--;
      coord.offset = block.logicalLineAt(coord.lineNo).moveToX(x, m_doc, true);
    } else if (coord.blockNo > 0) {
      coord.blockNo--;
      coord.lineNo = m_blocks[coord.blockNo].countOfLogicalLine() - 1;
      coord.offset = m_blocks[coord.blockNo].logicalLineAt(coord.lineNo).moveToX(x, m_doc, true);
    } else {
      coord.blockNo = 0;
      coord.lineNo = 0;
      coord.offset = 0;
    }
  }
  return coord;
}

CursorCoord CursorNavigator::moveCursorToDown(CursorCoord coord, core::Point pos) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  int x = pos.x;
  if (line.canMoveDown(coord.offset, m_doc)) {
    DEBUG << "move down";
    coord.offset = line.moveDown(coord.offset, x, m_doc);
  } else {
    if (coord.lineNo + 1 < block.countOfLogicalLine()) {
      coord.lineNo++;
      coord.offset = block.logicalLineAt(coord.lineNo).moveToX(x, m_doc);
    } else if (coord.blockNo + 1 < m_blocks.size()) {
      coord.blockNo++;
      coord.lineNo = 0;
      coord.offset = m_blocks[coord.blockNo].logicalLineAt(coord.lineNo).moveToX(x, m_doc);
    } else {
      coord.offset = line.length();
    }
  }
  return coord;
}

CursorCoord CursorNavigator::moveCursorToPos(core::Point pos) {
  SizeType blockNo = 0;
  int y = m_setting.docMargin.top;
  if (pos.y <= y) {
    CursorCoord coord;
    coord.blockNo = 0;
    coord.lineNo = 0;
    coord.offset = 0;
    return coord;
  }
  bool findBlock = false;
  while (blockNo < m_blocks.size()) {
    int h = m_blocks[blockNo].height();
    if (pos.y >= y && y + h >= pos.y) {
      findBlock = true;
      break;
    }
    y += h + m_setting.blockSpacing;
    blockNo++;
  }
  if (!findBlock) {
    return moveCursorToEndOfDocument();
  }
  const auto& block = m_blocks[blockNo];
  if (block.countOfLogicalLine() == 0) {
    CursorCoord coord;
    coord.blockNo = blockNo;
    coord.lineNo = 0;
    coord.offset = 0;
    return coord;
  }
  auto oldY = y;
  for (int lineNo = 0; lineNo < block.countOfLogicalLine(); ++lineNo) {
    auto& line = block.logicalLineAt(lineNo);
    if (y <= pos.y && pos.y <= y + line.height()) {
      CursorCoord coord;
      coord.blockNo = blockNo;
      coord.lineNo = lineNo;
      coord.offset = line.offsetAt(core::Point(pos.x, pos.y - oldY), m_doc, m_setting.lineSpacing);
      return coord;
    }
    y += line.height();
  }
  DEBUG << "not handle -- falling back to end of document";
  ASSERT(false && "not handle");
  return moveCursorToEndOfDocument();
}

CursorCoord CursorNavigator::moveCursorToBol(CursorCoord coord) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  coord.offset = line.moveToBol(coord.offset, m_doc);
  return coord;
}

std::pair<CursorCoord, int> CursorNavigator::moveCursorToEol(CursorCoord coord) {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  auto& line = block.logicalLineAt(coord.lineNo);
  auto [offset, x] = line.moveToEol(coord.offset, m_doc);
  coord.offset = offset;
  return {coord, x};
}

CursorCoord CursorNavigator::moveCursorToEndOfDocument() {
  CursorCoord coord;
  ASSERT(!m_blocks.empty());
  coord.blockNo = m_blocks.size() - 1;
  while (coord.blockNo > 0) {
    const auto& block = m_blocks[coord.blockNo];
    if (block.countOfLogicalLine() == 1 && block.logicalLineAt(0).length() == 0 &&
        m_root.childAt(coord.blockNo)->type() == parser::NodeType::paragraph) {
      coord.blockNo--;
    } else {
      break;
    }
  }
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(block.countOfLogicalLine() > 0);
  coord.lineNo = block.countOfLogicalLine() - 1;
  coord.offset = block.logicalLineAt(coord.lineNo).length();
  return coord;
}

CursorCoord CursorNavigator::moveCursorToBeginOfDocument() {
  CursorCoord coord;
  coord.blockNo = 0;
  coord.lineNo = 0;
  coord.offset = 0;
  return coord;
}

bool CursorNavigator::isBol(const CursorCoord& coord) const {
  ASSERT(coord.blockNo >= 0 && coord.blockNo < m_blocks.size());
  const auto& block = m_blocks[coord.blockNo];
  ASSERT(coord.lineNo >= 0 && coord.lineNo < block.countOfLogicalLine());
  const auto& line = block.logicalLineAt(coord.lineNo);
  return line.isBol(coord.offset, m_doc);
}

}  // namespace md::editor
