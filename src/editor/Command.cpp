//
// Created by PikachuHy on 2021/11/26.
//

#include "Command.h"

#include "Cursor.h"
#include "core/Utf8Util.h"
#include "debug.h"
#include "parser/Document.h"
#include "render/Render.h"
#include "MarkdownSerializer.h"
using namespace md::parser;
using namespace md::render;
namespace md::editor {

static SizeType computeContentPos(const render::Block& block, SizeType lineNo, SizeType offset) {
  SizeType pos = 0;
  for (SizeType i = 0; i < lineNo; ++i) {
    pos += block.logicalLineAt(i).length();
  }
  return pos + offset;
}

// ---- InsertTextCommand ----

InsertTextCommand::InsertTextCommand(Document* doc, CursorCoord coord, String text) : Command(doc), m_coord(coord) {
  if (text == "(") {
    m_text = "()";
  } else if (text == "[") {
    m_text = "[]";
  } else if (text == "{") {
    m_text = "{}";
  } else {
    m_text = text;
  }
}

void InsertTextCommand::execute(Cursor& cursor) {
  // Save snapshot
  auto* blockNode = m_doc->root()->childAt(m_coord.blockNo);
  m_snapshot = blockNode->clone();

  // Compute content position
  const auto& block = m_doc->blocks()[m_coord.blockNo];
  m_contentPos = computeContentPos(block, m_coord.lineNo, m_coord.offset);

  // Serialize block and find markdown position
  auto [markdown, mdPos] = m_doc->cursorToMarkdownPosition(m_coord);

  // Handle bracket skip: if we're inserting ")" and the next char is ")", just move cursor
  if (m_text == ")" || m_text == "]" || m_text == "}") {
    if (mdPos < markdown.length() && markdown[mdPos] == m_text[0]) {
      CursorCoord newCoord = m_coord;
      newCoord.offset++;
      m_doc->updateCursor(cursor, newCoord);
      m_finishedCoord = newCoord;
      return;
    }
  }

  // Insert text in markdown
  String editedMD = markdown.left(mdPos) + m_text + markdown.mid(mdPos);

  // Add to add buffer and reparse
  SizeType addOffset = m_doc->appendToAddBuffer(editedMD);

  // Replace blocks
  m_doc->replaceBlocksFromText(m_coord.blockNo, m_coord.blockNo + 1,
                                editedMD, addOffset, editedMD.length());

  // Compute new cursor position
  auto* newBlockNode = m_doc->root()->childAt(m_coord.blockNo);
  auto oldType = m_snapshot->type();
  auto newType = newBlockNode->type();

  // If block type changed (e.g., Paragraph→Header from "# "), cursor resets to start of content
  if (oldType != newType) {
    m_finishedCoord = CursorCoord{m_coord.blockNo, 0, 0};
  } else {
    SizeType newContentPos = m_contentPos + m_text.length();
    // For auto-paired brackets, cursor should be between them
    if (m_text.length() == 2 && (m_text[0] == '(' || m_text[0] == '[' || m_text[0] == '{')) {
      newContentPos = m_contentPos + 1;
    }
    m_finishedCoord = m_doc->findCursorFromContentPosition(m_coord.blockNo, newContentPos);
  }

  m_doc->updateCursor(cursor, m_finishedCoord);
  m_doc->ensureTrailingParagraph();
}

void InsertTextCommand::undo(Cursor& cursor) {
  m_doc->replaceBlock(m_coord.blockNo, m_snapshot->clone());
  m_doc->ensureTrailingParagraph();
  m_doc->updateCursor(cursor, m_coord);
}

bool InsertTextCommand::merge(Command* command) {
  if (this->type() != command->type()) return false;
  auto* other = static_cast<InsertTextCommand*>(command);
  if (m_finishedCoord != other->m_coord) return false;
  if (m_text.length() >= 20) return false;  // Avoid unbounded merge
  m_text += other->m_text;
  m_finishedCoord = other->m_finishedCoord;
  return true;
}

// ---- RemoveTextCommand ----

void RemoveTextCommand::execute(Cursor& cursor) {
  const auto& block = m_doc->blocks()[m_coord.blockNo];
  SizeType contentPos = computeContentPos(block, m_coord.lineNo, m_coord.offset);
  m_contentPos = contentPos;

  // Case 1: cursor at start of block — merge with previous or degrade
  if (contentPos == 0) {
    if (m_coord.blockNo > 0) {
      // Merge with previous block
      String prevMD = m_doc->serializeBlock(m_coord.blockNo - 1);
      String curMD = m_doc->serializeBlock(m_coord.blockNo);

      // Strip trailing "\n\n" from previous block
      if (prevMD.endsWith("\n\n")) prevMD = prevMD.left(prevMD.length() - 2);
      else if (prevMD.endsWith("\n")) prevMD = prevMD.left(prevMD.length() - 1);

      String joinedMD = prevMD + curMD;

      // Save snapshots of both blocks
      m_snapshot = m_doc->root()->childAt(m_coord.blockNo - 1)->clone();

      // Compute cursor position: end of previous block's content
      SizeType prevContentLen = 0;
      const auto& prevBlock = m_doc->blocks()[m_coord.blockNo - 1];
      for (SizeType i = 0; i < prevBlock.countOfLogicalLine(); ++i) {
        prevContentLen += prevBlock.logicalLineAt(i).length();
      }

      SizeType addOffset = m_doc->appendToAddBuffer(joinedMD);
      m_doc->replaceBlocksFromText(m_coord.blockNo - 1, m_coord.blockNo + 1,
                                    joinedMD, addOffset, joinedMD.length());

      CursorCoord newCoord = m_doc->findCursorFromContentPosition(m_coord.blockNo - 1, prevContentLen);
      m_finishedCoord = newCoord;
      m_doc->updateCursor(cursor, newCoord);
      m_hasAction = true;
      m_doc->ensureTrailingParagraph();
      return;
    }

    // At start of first block and offset 0 — check for header degrade
    auto* blockNode = m_doc->root()->childAt(m_coord.blockNo);
    if (blockNode->type() == NodeType::header) {
      m_snapshot = blockNode->clone();
      auto* headerNode = static_cast<Header*>(blockNode);
      String md = m_doc->serializeBlock(m_coord.blockNo);
      // Remove "# prefix": skip the "#"*level + " " prefix
      SizeType prefixLen = headerNode->level() + 1;
      String editedMD = md.mid(prefixLen);
      SizeType addOffset = m_doc->appendToAddBuffer(editedMD);
      m_doc->replaceBlocksFromText(m_coord.blockNo, m_coord.blockNo + 1,
                                    editedMD, addOffset, editedMD.length());
      m_finishedCoord = m_coord;
      m_doc->updateCursor(cursor, m_coord);
      m_hasAction = true;
      m_doc->ensureTrailingParagraph();
      return;
    }

    // At start of document — nothing to delete
    return;
  }

  // Case 2: normal delete within block
  m_snapshot = m_doc->root()->childAt(m_coord.blockNo)->clone();

  auto [markdown, mdPos] = m_doc->cursorToMarkdownPosition(m_coord);

  if (mdPos == 0) return;  // Safety check

  // Delete one character before cursor in markdown
  // Handle multi-unit UTF characters
  SizeType charLen = 1;
  if (mdPos > 0) {
    auto prevStart = ::md::previousCodePointStart(markdown.toStdString(), mdPos);
    charLen = mdPos - prevStart;
  }
  String editedMD = markdown.left(mdPos - charLen) + markdown.mid(mdPos);

  SizeType addOffset = m_doc->appendToAddBuffer(editedMD);
  m_doc->replaceBlocksFromText(m_coord.blockNo, m_coord.blockNo + 1,
                                editedMD, addOffset, editedMD.length());

  SizeType newContentPos = contentPos - charLen;
  if (newContentPos < 0) newContentPos = 0;
  m_finishedCoord = m_doc->findCursorFromContentPosition(m_coord.blockNo, newContentPos);
  m_doc->updateCursor(cursor, m_finishedCoord);
  m_hasAction = true;
  m_doc->ensureTrailingParagraph();
}

void RemoveTextCommand::undo(Cursor& cursor) {
  m_doc->replaceBlock(m_coord.blockNo, m_snapshot->clone());
  m_doc->ensureTrailingParagraph();
  m_doc->updateCursor(cursor, m_coord);
}

// ---- InsertReturnCommand ----

void InsertReturnCommand::execute(Cursor& cursor) {
  const auto& block = m_doc->blocks()[m_coord.blockNo];

  SizeType contentPos = 0;
  if (block.countOfLogicalLine() > 0) {
    contentPos = computeContentPos(block, m_coord.lineNo, m_coord.offset);
  }

  // Check for code-block prefix: if line starts with "```", handle specially
  // (re-parse would misinterpret ```\n... as a complete code block)
  if (block.countOfLogicalLine() > 0 && contentPos >= 3) {
    auto& line = block.logicalLineAt(m_coord.lineNo);
    String linePrefix = line.left(m_coord.offset, m_doc->bufferProvider());
    if (linePrefix.startsWith("```")) {
      m_snapshots.push_back({m_coord.blockNo, m_doc->root()->childAt(m_coord.blockNo)->clone()});
      auto [textNode, textOffset] = line.textAt(m_coord.offset);
      auto [leftText, rightText] = textNode->split(textOffset);
      leftText->remove(0, 3);
      auto cb = std::make_unique<CodeBlock>(std::move(leftText));
      auto newP = std::make_unique<Paragraph>();
      if (rightText && !rightText->empty()) {
        newP->appendChild(std::move(rightText));
      }
      m_doc->replaceBlock(m_coord.blockNo, std::move(cb));
      m_doc->insertBlock(m_coord.blockNo + 1, std::move(newP));
      m_finishedCoord = CursorCoord{m_coord.blockNo + 1, 0, 0};
      m_doc->updateCursor(cursor, m_finishedCoord);
      m_doc->ensureTrailingParagraph();
      return;
    }
  }

  // Empty line or end-of-content: directly insert a new empty block
  bool isEndOfContent = true;
  if (block.countOfLogicalLine() > 0) {
    SizeType totalContent = 0;
    for (SizeType i = 0; i < block.countOfLogicalLine(); ++i) {
      totalContent += block.logicalLineAt(i).length();
    }
    isEndOfContent = (contentPos >= totalContent);
  }

  if (block.countOfLogicalLine() == 0 || isEndOfContent) {
    m_snapshots.push_back({m_coord.blockNo, m_doc->root()->childAt(m_coord.blockNo)->clone()});
    m_doc->insertBlock(m_coord.blockNo + 1, std::make_unique<Paragraph>());
    CursorCoord newCoord{m_coord.blockNo + 1, 0, 0};
    m_finishedCoord = newCoord;
    m_doc->updateCursor(cursor, newCoord);
    m_doc->ensureTrailingParagraph();
    return;
  }

  // Save snapshot of current block
  m_snapshots.push_back({m_coord.blockNo, m_doc->root()->childAt(m_coord.blockNo)->clone()});

  auto [markdown, mdPos] = m_doc->cursorToMarkdownPosition(m_coord);

  // Insert newline at cursor position
  String editedMD = markdown.left(mdPos) + "\n" + markdown.mid(mdPos);

  SizeType addOffset = m_doc->appendToAddBuffer(editedMD);

  // Reparse — may produce 1+ blocks
  m_doc->replaceBlocksFromText(m_coord.blockNo, m_coord.blockNo + 1,
                                editedMD, addOffset, editedMD.length());

  // Find cursor in the new blocks: it should be at the start of the new block
  // After splitting, cursor goes to the start of whatever is after the \n
  // This is blockNo + 1 (if a new block was created) or a new line within the same block
  SizeType newContentPos = contentPos;  // Content position within original block
  CursorCoord newCoord;
  SizeType totalContent = 0;
  for (SizeType i = 0; i < m_coord.lineNo; ++i) {
    totalContent += block.logicalLineAt(i).length();
  }
  SizeType lineStartContent = totalContent;
  // Cursor is at lineStartContent + m_coord.offset within the content
  // After inserting \n, content before \n stays in one block, after goes to next
  // So try current block first
  newCoord = m_doc->findCursorFromContentPosition(m_coord.blockNo, contentPos);
  // If cursor landed at end of block, it might have moved to next block
  if (newCoord.blockNo == m_coord.blockNo) {
    const auto& newBlock = m_doc->blocks()[m_coord.blockNo];
    if (newCoord.offset >= newBlock.logicalLineAt(newCoord.lineNo).length()) {
      // Try next block
      if (m_coord.blockNo + 1 < m_doc->blocks().size()) {
        newCoord = CursorCoord{m_coord.blockNo + 1, 0, 0};
      }
    }
  } else {
    // Cursor moved to different block
    // The \n might have created a new block. Cursor should be at start of the block after the split
    newCoord = CursorCoord{newCoord.blockNo, 0, 0};
  }

  m_finishedCoord = newCoord;
  m_doc->updateCursor(cursor, newCoord);
  m_doc->ensureTrailingParagraph();
}

void InsertReturnCommand::undo(Cursor& cursor) {
  // Restore snapshot of original block
  for (auto& [blockNo, snapshot] : m_snapshots) {
    m_doc->replaceBlock(blockNo, snapshot->clone());
  }
  m_doc->ensureTrailingParagraph();
  m_doc->updateCursor(cursor, m_coord);
}

// ---- UpgradeToHeaderCommand ----

UpgradeToHeaderCommand::UpgradeToHeaderCommand(Document* doc, CursorCoord coord, int level)
    : Command(doc), m_coord(coord), m_level(level) {}

void UpgradeToHeaderCommand::execute(Cursor& cursor) {
  auto* blockNode = m_doc->root()->childAt(m_coord.blockNo);
  m_snapshot = blockNode->clone();

  String md = m_doc->serializeBlock(m_coord.blockNo);
  // Prepend "#" markers — remove trailing "\n\n", prepend, add back
  bool hadNewlines = md.endsWith("\n\n");
  if (hadNewlines) md = md.left(md.length() - 2);
  else if (md.endsWith("\n")) md = md.left(md.length() - 1);

  String prefix;
  for (int i = 0; i < m_level; ++i) prefix += "#";
  String editedMD = prefix + " " + md;
  if (hadNewlines) editedMD += "\n\n";

  SizeType addOffset = m_doc->appendToAddBuffer(editedMD);
  m_doc->replaceBlocksFromText(m_coord.blockNo, m_coord.blockNo + 1,
                                editedMD, addOffset, editedMD.length());
  m_finishedCoord = CursorCoord{m_coord.blockNo, 0, 0};
  m_doc->updateCursor(cursor, m_finishedCoord);
  m_doc->ensureTrailingParagraph();
}

void UpgradeToHeaderCommand::undo(Cursor& cursor) {
  m_doc->replaceBlock(m_coord.blockNo, m_snapshot->clone());
  m_doc->ensureTrailingParagraph();
  m_doc->updateCursor(cursor, m_coord);
}

// ---- RemoveTextRangeCommand ----

RemoveTextRangeCommand::RemoveTextRangeCommand(Document* doc, CursorCoord begin, CursorCoord end)
    : Command(doc), m_begin(begin), m_end(end) {}

void RemoveTextRangeCommand::execute(Cursor& cursor) {
  if (m_end < m_begin) {
    std::swap(m_begin, m_end);
  }

  // Handle same-block selection removal
  if (m_begin.blockNo == m_end.blockNo) {
    m_snapshots.push_back({m_begin.blockNo, m_doc->root()->childAt(m_begin.blockNo)->clone()});

    auto [md, mdBegin] = m_doc->cursorToMarkdownPosition(m_begin);
    auto [md2, mdEnd] = m_doc->cursorToMarkdownPosition(m_end);

    if (mdBegin < mdEnd) {
      String editedMD = md.left(mdBegin) + md.mid(mdEnd);
      SizeType addOffset = m_doc->appendToAddBuffer(editedMD);
      m_doc->replaceBlocksFromText(m_begin.blockNo, m_begin.blockNo + 1,
                                    editedMD, addOffset, editedMD.length());
      m_hasAction = true;
    }

    m_finishedCoord = m_begin;
    m_doc->updateCursor(cursor, m_begin);
    m_doc->ensureTrailingParagraph();
    return;
  }

  // Cross-block selection: serialize each block, remove text range, reparse
  String combinedMD;
  SizeType globalBegin = 0;  // Position in combinedMD where m_begin maps
  SizeType globalEnd = 0;    // Position in combinedMD where m_end maps

  for (SizeType i = m_begin.blockNo; i <= m_end.blockNo; ++i) {
    String blockMD = m_doc->serializeBlock(i);
    // For intermediate blocks, strip trailing "\n\n"
    if (i < m_end.blockNo && blockMD.endsWith("\n\n")) {
      blockMD = blockMD.left(blockMD.length() - 2);
    }
    if (i == m_begin.blockNo) {
      globalBegin = combinedMD.length();
      auto [_, pos] = m_doc->cursorToMarkdownPosition(m_begin);
      globalBegin += pos;
    }
    if (i == m_end.blockNo) {
      globalEnd = combinedMD.length();
      auto [_, pos] = m_doc->cursorToMarkdownPosition(m_end);
      globalEnd += pos;
    }
    combinedMD += blockMD;
  }

  // Save snapshots
  for (SizeType i = m_begin.blockNo; i <= m_end.blockNo; ++i) {
    m_snapshots.push_back({i, m_doc->root()->childAt(i)->clone()});
  }

  if (globalBegin < globalEnd) {
    String editedMD = combinedMD.left(globalBegin) + combinedMD.mid(globalEnd);
    SizeType addOffset = m_doc->appendToAddBuffer(editedMD);
    m_doc->replaceBlocksFromText(m_begin.blockNo, m_end.blockNo + 1,
                                  editedMD, addOffset, editedMD.length());
    m_hasAction = true;
  }

  m_finishedCoord = m_begin;
  m_doc->updateCursor(cursor, m_begin);
  m_doc->ensureTrailingParagraph();
}

void RemoveTextRangeCommand::undo(Cursor& cursor) {
  // Restore snapshots in reverse order
  for (auto it = m_snapshots.rbegin(); it != m_snapshots.rend(); ++it) {
    m_doc->replaceBlock(it->first, it->second->clone());
  }
  m_doc->ensureTrailingParagraph();
  m_doc->updateCursor(cursor, m_begin);
}

// ---- CommandStack ----

void CommandStack::push(std::unique_ptr<Command> command) {
  while (m_commands.size() != m_top) {
    m_commands.pop_back();
  }
  if (m_commands.empty()) {
    m_commands.push_back(std::move(command));
  } else {
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
