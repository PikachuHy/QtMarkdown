//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_EDITOR_H
#define QTMARKDOWN_EDITOR_H
#include "Document.h"
#include "mddef.h"
namespace md::editor {
class Cursor;
class Editor {
 public:
  Editor();
  void loadFile(const String& path);
  void paintEvent(QPoint offset, Painter& painter);
  void keyPressEvent(KeyEvent* event);
  void mousePressEvent(MouseEvent* event);
  [[nodiscard]] int width() const;
  [[nodiscard]] int height() const;
  [[nodiscard]] Point cursorPos() const;
  void insertText(String str);

 private:
  void drawCursor(QPoint offset, Painter& painter);

 private:
  sptr<Document> m_doc;
  sptr<Cursor> m_cursor;
  bool m_showCursor;
};
}  // namespace md::editor

#endif  // QTMARKDOWN_EDITOR_H
