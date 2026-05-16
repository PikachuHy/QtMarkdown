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
#include "render/mddef.h"
namespace md::editor {
class QTMARKDOWNEDITORCORE_EXPORT Command {
 public:
  enum Type { insert_text, remove_text, insert_return, upgrade_to_header, remove_text_range };
  Command(Document* doc) : m_doc(doc) {}
  virtual ~Command() = default;
  [[nodiscard]] virtual Type type() const = 0;
  virtual bool merge(Command* command) = 0;
  virtual void execute(Cursor& cursor) = 0;
  virtual void undo(Cursor& cursor) = 0;

 protected:
  Document* m_doc;
};
class QTMARKDOWNEDITORCORE_EXPORT InsertTextCommand : public Command {
 public:
  InsertTextCommand(Document* doc, CursorCoord coord, String text);
  [[nodiscard]] Type type() const override { return insert_text; }
  bool merge(Command* command) override;
  void execute(Cursor& cursor) override;
  void undo(Cursor& cursor) override;

 private:
  CursorCoord m_coord;
  CursorCoord m_finishedCoord;
  String m_text;
  std::unique_ptr<parser::Node> m_snapshot;
  SizeType m_contentPos = 0;
};
class QTMARKDOWNEDITORCORE_EXPORT RemoveTextCommand : public Command {
 public:
  RemoveTextCommand(Document* doc, CursorCoord coord) : Command(doc), m_coord(coord) {}
  [[nodiscard]] Type type() const override { return remove_text; }
  bool merge(Command* command) override { return false; }
  void execute(Cursor& cursor) override;
  void undo(Cursor& cursor) override;
  [[nodiscard]] bool hasUndoAction() const { return m_hasAction; }

 private:
  CursorCoord m_coord;
  CursorCoord m_finishedCoord;
  bool m_hasAction = false;
  int m_originalBlockCount = 0;
  std::vector<std::pair<SizeType, std::unique_ptr<parser::Node>>> m_snapshots;
  SizeType m_contentPos = 0;
};
class QTMARKDOWNEDITORCORE_EXPORT InsertReturnCommand : public Command {
 public:
  InsertReturnCommand(Document* doc, CursorCoord coord) : Command(doc), m_coord(coord) {}
  [[nodiscard]] Type type() const override { return insert_return; }
  bool merge(Command* command) override { return false; }
  void execute(Cursor& cursor) override;
  void undo(Cursor& cursor) override;

 private:
  void handleListEnter(Cursor& cursor, parser::Container* listNode, const render::Block& block, SizeType contentPos);
  void handleCodeBlockEnter(Cursor& cursor, const render::Block& block, SizeType contentPos);
  void handleContentSplit(Cursor& cursor, const render::Block& block, SizeType contentPos);
  CursorCoord m_coord;
  CursorCoord m_finishedCoord;
  int m_originalBlockCount = 0;
  std::vector<std::pair<SizeType, std::unique_ptr<parser::Node>>> m_snapshots;
};
class QTMARKDOWNEDITORCORE_EXPORT UpgradeToHeaderCommand : public Command {
 public:
  UpgradeToHeaderCommand(Document* doc, CursorCoord coord, int level);
  [[nodiscard]] Type type() const override { return upgrade_to_header; }
  void execute(Cursor& cursor) override;
  void undo(Cursor& cursor) override;
  bool merge(Command* command) override { return false; }
  bool hasUndoAction() const { return true; }
  CursorCoord finishedCoord() const { return m_finishedCoord; }
 private:
  CursorCoord m_coord;
  CursorCoord m_finishedCoord;
  int m_level;
  std::unique_ptr<parser::Node> m_snapshot;
};
class QTMARKDOWNEDITORCORE_EXPORT RemoveTextRangeCommand : public Command {
 public:
  RemoveTextRangeCommand(Document* doc, CursorCoord begin, CursorCoord end);
  [[nodiscard]] Type type() const override { return remove_text_range; }
  void execute(Cursor& cursor) override;
  void undo(Cursor& cursor) override;
  bool merge(Command* command) override { return false; }
  bool hasUndoAction() const { return m_hasAction; }
 private:
  CursorCoord m_begin;
  CursorCoord m_end;
  bool m_hasAction = false;
  std::vector<std::pair<SizeType, std::unique_ptr<parser::Node>>> m_snapshots;
  CursorCoord m_finishedCoord;
};
class QTMARKDOWNEDITORCORE_EXPORT CommandStack {
 public:
  static constexpr size_t kMaxCommands = 500;
  void push(std::unique_ptr<Command> command);
  void undo(Cursor& cursor);
  void redo(Cursor& cursor);

 private:
  std::vector<std::unique_ptr<Command>> m_commands;
  size_t m_top = 0;
};
}  // namespace md::editor
#endif  // QTMARKDOWN_COMMAND_H
