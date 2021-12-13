//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_EDITOR_H
#define QTMARKDOWN_EDITOR_H
#include <utility>

#include "QtMarkdown_global.h"
#include "Document.h"
#include "CursorCoord.h"
#include "mddef.h"
namespace md::editor {
class Cursor;
class SelectionRange;
enum CursorShape {
  ArrowCursor,
  UpArrowCursor,
  CrossCursor,
  WaitCursor,
  IBeamCursor,
  SizeVerCursor,
  SizeHorCursor,
  SizeBDiagCursor,
  SizeFDiagCursor,
  SizeAllCursor,
  BlankCursor,
  SplitVCursor,
  SplitHCursor,
  PointingHandCursor,
  ForbiddenCursor,
  WhatsThisCursor,
  BusyCursor,
  OpenHandCursor,
  ClosedHandCursor,
  DragCopyCursor,
  DragMoveCursor,
  DragLinkCursor,
  LastCursor = DragLinkCursor,
  BitmapCursor = 24,
  CustomCursor = 25
};

class QTMARKDOWNSHARED_EXPORT Editor {
 public:
  Editor();
  void loadText(const String& text);
  std::pair<bool, String> loadFile(const String& path);
  String title();
  bool saveToFile(const String& path);
  void drawDoc(Point offset, Painter& painter);
  void drawCursor(Point offset, Painter& painter);
  void drawSelection(Point offset, Painter& painter);
  void keyPressEvent(KeyEvent* event);
  void keyReleaseEvent(KeyEvent* event);
  void mousePressEvent(Point offset, MouseEvent* event);
  void mouseMoveEvent(Point offset, MouseEvent* event);
  void mouseReleaseEvent(Point offset, MouseEvent* event);
  CursorShape cursorShape(Point offset, Point pos);
  [[nodiscard]] int width() const;
  [[nodiscard]] int height() const;
  [[nodiscard]] Point cursorPos() const;
  [[nodiscard]] Rect cursorRect() const;
  void insertText(String str);
  void setPreedit(String str);
  void commitString(String str);
  void reset();
  [[nodiscard]] String cursorCoord() const;
  [[nodiscard]] sptr<Document> document() const { return m_doc; }
  void setLinkClickedCallback(std::function<void(String)> cb) { m_linkClickedCallback = std::move(cb); }
  void setImageClickedCallback(std::function<void(String)> cb) { m_imageClickedCallback = std::move(cb); }
  void setCopyCodeBtnClickedCallback(std::function<void(String)> cb) { m_copyCodeBtnClickedCallback = std::move(cb); }
  void setCheckBoxClickedCallback(std::function<void()> cb) { m_checkBoxClickedCallback = std::move(cb); }
  void setWidth(int w);
  void setResPathList(StringList pathList);
  void renderDocument();

 private:
  void selectUp();
  void selectDown();
  void selectLeft();
  void selectRight();
  void selectBol();
  void selectEol();
  void selectAll();
  void generateSelectionInstruction();
  void removeSelection();

 private:
  sptr<Document> m_doc;
  sptr<Cursor> m_cursor;
  sptr<render::RenderSetting> m_renderSetting;
  std::vector<InstructionPtr> m_selectionInstructions;
  bool m_holdCtrl = false;
  bool m_holdShift = false;
  bool m_mousePressing = false;
  bool m_preediting = false;
  int m_preeditLength;
  Point m_preeditPos;
  bool m_hasSelection = false;
  sptr<SelectionRange> m_selectionRange;
  std::function<void(String)> m_linkClickedCallback;
  std::function<void(String)> m_imageClickedCallback;
  std::function<void(String)> m_copyCodeBtnClickedCallback;
  std::function<void()> m_checkBoxClickedCallback;
  friend class MousePressVisitor;
};
}  // namespace md::editor

#endif  // QTMARKDOWN_EDITOR_H
