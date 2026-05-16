//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_DOCUMENT_H
#define QTMARKDOWN_DOCUMENT_H
#include "QtMarkdown_global.h"
#include "render/mddef.h"
#include "parser/Document.h"
#include "parser/IBufferProvider.h"
#include "render/Instruction.h"
#include "render/Render.h"
#include "core/Types.h"
#include "core/IImageProvider.h"
#include "CursorNavigator.h"

namespace md::editor {
class Command;
class CommandStack;
class Cursor;
struct CursorCoord;
class QTMARKDOWNEDITORCORE_EXPORT Document {
 public:
  explicit Document(const String& str, sptr<render::RenderSetting> setting,
                    core::IImageProvider* imageProvider = nullptr);
  parser::Container* root() const { return m_parserDoc->root(); }
  const String& addBuffer() const { return m_parserDoc->addBuffer(); }
  const parser::IBufferProvider& bufferProvider() const { return *m_parserDoc; }
  void accept(parser::NodeVisitor* visitor) const { m_parserDoc->accept(visitor); }
  SizeType appendToAddBuffer(const String& text) {
    SizeType offset = m_parserDoc->addBuffer().size();
    m_parserDoc->addBuffer().append(text);
    return offset;
  }
  CursorCoord moveCursorToRight(CursorCoord coord) { return m_navigator.moveCursorToRight(coord); }
  CursorCoord moveCursorToLeft(CursorCoord coord) { return m_navigator.moveCursorToLeft(coord); }
  CursorCoord moveCursorToBol(CursorCoord coord) { return m_navigator.moveCursorToBol(coord); }
  std::pair<CursorCoord, int> moveCursorToEol(CursorCoord coord) { return m_navigator.moveCursorToEol(coord); }
  CursorCoord moveCursorToUp(CursorCoord coord, core::Point pos) { return m_navigator.moveCursorToUp(coord, pos); }
  CursorCoord moveCursorToDown(CursorCoord coord, core::Point pos) { return m_navigator.moveCursorToDown(coord, pos); }
  CursorCoord moveCursorToPos(core::Point pos) { return m_navigator.moveCursorToPos(pos); }
  CursorCoord moveCursorToBeginOfDocument() { return m_navigator.moveCursorToBeginOfDocument(); }
  CursorCoord moveCursorToEndOfDocument() { return m_navigator.moveCursorToEndOfDocument(); }
  void insertText(Cursor& cursor, const String& text);
  void removeText(Cursor& cursor);
  void insertReturn(Cursor& cursor);
  const render::BlockList& blocks() const { return m_blocks; };

  void renderAllBlock();
  void replaceBlock(SizeType blockNo, std::unique_ptr<parser::Node> node);
  void insertBlock(SizeType blockNo, std::unique_ptr<parser::Node> node);
  void renderBlock(SizeType blockNo);
  void removeBlock(SizeType blockNo);
  void mergeBlock(SizeType blockNo1, SizeType blockNo2);
  void removeTextRange(const CursorCoord& begin, const CursorCoord& end);

  void undo(Cursor& cursor);
  void redo(Cursor& cursor);

  void upgradeToHeader(Cursor& cursor, int level);

  void updateCursor(Cursor& cursor, const CursorCoord& coord, bool updatePos = true) { m_navigator.updateCursor(cursor, coord, updatePos); }
  std::tuple<core::Point, int, int> mapToScreen(const CursorCoord& coord) { return m_navigator.mapToScreen(coord); }
  bool isBol(const CursorCoord& coord) const { return m_navigator.isBol(coord); }
  void ensureTrailingParagraph();

  const render::RenderSetting& setting() const { return *m_setting; }

  String serializeBlock(SizeType blockNo) const;
  struct MarkdownPosition {
    String text;
    SizeType pos;
  };
  MarkdownPosition cursorToMarkdownPosition(const CursorCoord& coord) const;
  CursorCoord findCursorFromContentPosition(SizeType blockNo, SizeType contentPos) const;
  void replaceBlocksFromText(SizeType startBlockNo, SizeType endBlockNo,
                              const String& editedMD, SizeType addOffset, SizeType addLength);
  int countOfBlock() const { return m_blocks.size(); }

 private:
  void assertBlocksInSync();
  std::unique_ptr<parser::Document> m_parserDoc;
  render::BlockList m_blocks;
  sptr<render::RenderSetting> m_setting;
  sptr<CommandStack> m_commandStack;
  core::IImageProvider* m_imageProvider = nullptr;
  CursorNavigator m_navigator{m_blocks, *m_parserDoc, *m_parserDoc->root(), *m_setting};
};
}  // namespace md::editor

#endif  // QTMARKDOWN_DOCUMENT_H
