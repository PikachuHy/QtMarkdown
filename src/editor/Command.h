//
// Created by PikachuHy on 2021/11/26.
//

#ifndef QTMARKDOWN_COMMAND_H
#define QTMARKDOWN_COMMAND_H
#include "QtMarkdown_global.h"
#include <memory>
#include <vector>

#include "CursorCoord.h"
#include "Document.h"
#include "mddef.h"
namespace md::editor {
class QTMARKDOWNSHARED_EXPORT Command {
 public:
  enum Type { insert_text, remove_text, insert_return };
  Command(Document* doc) : m_doc(doc) {}
  virtual ~Command() = default;
  [[nodiscard]] virtual Type type() const = 0;
  virtual bool merge(Command* command) = 0;
  virtual void execute(Cursor& cursor) = 0;
  virtual void undo(Cursor& cursor) = 0;

 protected:
  Document* m_doc;
};
class QTMARKDOWNSHARED_EXPORT InsertTextCommand : public Command {
 public:
  InsertTextCommand(Document* doc, CursorCoord coord, String text);
  [[nodiscard]] Type type() const override { return insert_text; }
  bool merge(Command* command) override;
  void execute(Cursor& cursor) override;
  void undo(Cursor& cursor) override;

 private:
  CursorCoord m_coord;
  SizeType m_offset;
  SizeType m_length;
  SizeType m_cursorOffsetDelta;
  bool m_isSpace;
  bool m_maySkipChar;
  String m_targetSkipChar;
  CursorCoord m_finishedCoord;
};
class QTMARKDOWNSHARED_EXPORT RemoveTextCommand : public Command {
 public:
  enum UndoAction { none, text_delete, block_merge, header_degrade };
  RemoveTextCommand(Document* doc, CursorCoord coord) : Command(doc), m_coord(coord) {}
  [[nodiscard]] Type type() const override { return remove_text; }
  bool merge(Command* command) override { return false; }
  void execute(Cursor& cursor) override;
  void undo(Cursor& cursor) override;

 private:
  friend class RemoveTextVisitor;
  CursorCoord m_coord;
  UndoAction m_undoAction = none;
  String m_deletedText;
  CursorCoord m_finishedCoord;
  SizeType m_mergePrevChildCount = 0;
  SizeType m_mergePrevLastTextLen = 0;
  int m_headerLevel = 0;
};
class QTMARKDOWNSHARED_EXPORT InsertReturnCommand : public Command {
 public:
  InsertReturnCommand(Document* doc, CursorCoord coord) : Command(doc), m_coord(coord) {}
  [[nodiscard]] Type type() const override { return insert_return; }
  bool merge(Command* command) override { return false; }
  void execute(Cursor& cursor) override;
  void undo(Cursor& cursor) override;

 private:
  CursorCoord m_coord;
  CursorCoord m_finishedCoord;
};
class QTMARKDOWNSHARED_EXPORT CommandStack {
 public:
  void push(std::unique_ptr<Command> command);
  void undo(Cursor& cursor);
  void redo(Cursor& cursor);

 private:
  std::vector<std::unique_ptr<Command>> m_commands;
  int m_top = 0;
};
}  // namespace md::editor
#endif  // QTMARKDOWN_COMMAND_H
