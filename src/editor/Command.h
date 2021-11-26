//
// Created by PikachuHy on 2021/11/26.
//

#ifndef QTMARKDOWN_COMMAND_H
#define QTMARKDOWN_COMMAND_H
#include <vector>

#include "CursorCoord.h"
#include "Document.h"
#include "mddef.h"
namespace md::editor {
class Command {
 public:
  enum Type { insert_text, remove_text, insert_return };
  Command(Document* doc) : m_doc(doc) {}
  [[nodiscard]] virtual Type type() const = 0;
  virtual bool merge(Command* command) = 0;
  virtual void execute(Cursor& cursor) = 0;
  virtual void undo(Cursor& cursor) = 0;

 protected:
  Document* m_doc;
};
class InsertTextCommand : public Command {
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
class RemoveTextCommand : public Command {
 public:
  RemoveTextCommand(Document* doc, CursorCoord coord) : Command(doc), m_coord(coord) {}
  [[nodiscard]] Type type() const override { return remove_text; }
  bool merge(Command* command) override { return false; }
  void execute(Cursor& cursor) override;
  void undo(Cursor& cursor) override;

 private:
  CursorCoord m_coord;
};
class InsertReturnCommand : public Command {
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
class CommandStack {
 public:
  void push(Command* command);
  void undo(Cursor& cursor);
  void redo(Cursor& cursor);

 private:
  std::vector<Command*> m_commands;
  int m_top;
};
}  // namespace md::editor
#endif  // QTMARKDOWN_COMMAND_H
