//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_EDITOR_H
#define QTMARKDOWN_EDITOR_H
#include <functional>
#include <utility>

#include "QtMarkdown_global.h"
#include "Document.h"
#include "CursorCoord.h"
#include "core/AbstractPainter.h"
#include "core/Types.h"
#include "core/Event.h"
#include "core/IImageProvider.h"
namespace md::editor {
class Cursor;
class SelectionRange;
class EditorRenderer;
class EditorInputHandler;
class FileManager;
enum CursorShape {
  IBeamCursor = 4,
  PointingHandCursor = 13,
};

class QTMARKDOWNSHARED_EXPORT Editor {
 public:
  Editor();
  ~Editor();
  void loadText(const String& text);
  std::pair<bool, String> loadFile(const String& path);
  String title();
  bool saveToFile(const String& path);
  void drawDoc(core::AbstractPainter& painter, const core::Point& offset);
  void drawCursor(core::AbstractPainter& painter, const core::Point& offset);
  void drawSelection(core::AbstractPainter& painter, const core::Point& offset);
  void keyPressEvent(const core::KeyEvent& event);
  void keyReleaseEvent(const core::KeyEvent& event);
  void mousePressEvent(const core::Point& offset, const core::MouseEvent& event);
  void mouseMoveEvent(const core::Point& offset, const core::MouseEvent& event);
  void mouseReleaseEvent(const core::Point& offset, const core::MouseEvent& event);
  CursorShape cursorShape(const core::Point& offset, const core::Point& pos);
  [[nodiscard]] int width() const;
  [[nodiscard]] int height() const;
  [[nodiscard]] core::Point cursorPos() const;
  [[nodiscard]] core::Rect cursorRect() const;
  void insertText(String str);
  void setPreedit(const String& str);
  void commitString(const String& str);
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

  // -- Public accessors for tests --
  Cursor& cursor() const { return *m_cursor; }
  bool hasSelection() const { return m_hasSelection; }

  // -- Getters for MousePressVisitor --
  bool isHoldCtrl() const { return m_holdCtrl; }
  bool isHoldShift() const { return m_holdShift; }
  void setHoldCtrl(bool v);
  void setHoldShift(bool v);
  void triggerLinkClicked(const String& url);
  void triggerImageClicked(const String& path);
  void triggerCopyCodeClicked(const String& code);
  void triggerCheckBoxClicked();

 private:
  sptr<Document> m_doc;
  sptr<Cursor> m_cursor;
  sptr<core::IImageProvider> m_imageProvider;
  sptr<render::RenderSetting> m_renderSetting;
  std::unique_ptr<EditorRenderer> m_renderer;
  std::unique_ptr<EditorInputHandler> m_inputHandler;
  std::vector<InstructionPtr> m_selectionInstructions;
  bool m_holdCtrl = false;
  bool m_holdShift = false;
  bool m_mousePressing = false;
  bool m_hasSelection = false;
  sptr<SelectionRange> m_selectionRange;
  std::function<void(String)> m_linkClickedCallback;
  std::function<void(String)> m_imageClickedCallback;
  std::function<void(String)> m_copyCodeBtnClickedCallback;
  std::function<void()> m_checkBoxClickedCallback;
  friend class EditorInputHandler;
};
}  // namespace md::editor

#endif  // QTMARKDOWN_EDITOR_H
