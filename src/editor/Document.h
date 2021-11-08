//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_DOCUMENT_H
#define QTMARKDOWN_DOCUMENT_H
#include "mddef.h"
#include "parser/Document.h"
#include "render/Instruction.h"
namespace md::editor {
class Cursor;
class Document : public parser::Document, public std::enable_shared_from_this<Document> {
 public:
  explicit Document(const String& str);
  void updateCursor(Cursor& cursor);
  void moveCursorToRight(Cursor& cursor);
  void moveCursorToLeft(Cursor& cursor);
  void moveCursorToBol(Cursor& cursor);
  void moveCursorToEol(Cursor& cursor);
  void moveCursorToUp(Cursor& cursor);
  void moveCursorToDown(Cursor& cursor);
  void moveCursorToPos(Cursor& cursor, Point pos);
  void insertText(Cursor& cursor, String text);
  void removeText(Cursor& cursor);
  void insertReturn(Cursor& cursor);

 private:
  void updateCursorOffset(Cursor& cursor, int blockNo, int lineNo, int itemNo, int textOffset);
  void replaceBlock(SizeType blockNo, parser::Node* node);
  void insertBlock(SizeType blockNo, parser::Node* node);
  void renderBlock(SizeType blockNo);
  void mergeBlock(SizeType blockNo1, SizeType blockNo2);
  parser::Container* node2container(parser::Node* node);
  void updateCursorCellNo(Cursor& cursor);

 private:
  QList<render::Block> m_blocks;
  friend class Editor;
};
}  // namespace md::editor

#endif  // QTMARKDOWN_DOCUMENT_H
