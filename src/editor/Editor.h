//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_EDITOR_H
#define QTMARKDOWN_EDITOR_H
#include "QtMarkdown_global.h"
#include "Document.h"
#include "mddef.h"
namespace md::editor {
class Cursor;
class QTMARKDOWNSHARED_EXPORT Editor {
 public:
  Editor();
  void loadText(const String& text);
  void loadFile(const String& path);
  bool saveToFile(const String& path);
  void drawDoc(Point offset, Painter& painter);
  void drawCursor(Point offset, Painter& painter);
  void keyPressEvent(KeyEvent* event);
  void mousePressEvent(MouseEvent* event);
  [[nodiscard]] int width() const;
  [[nodiscard]] int height() const;
  [[nodiscard]] Point cursorPos() const;
  void insertText(String str);
  void reset();
  [[nodiscard]] String cursorCoord() const;
  [[nodiscard]] sptr<Document> document() const { return m_doc; }

 private:
 private:
  sptr<Document> m_doc;
  sptr<Cursor> m_cursor;
  sptr<render::RenderSetting> m_renderSetting;
  bool m_showCursor;
};
}  // namespace md::editor

#endif  // QTMARKDOWN_EDITOR_H
