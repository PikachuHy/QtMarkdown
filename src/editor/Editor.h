//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_EDITOR_H
#define QTMARKDOWN_EDITOR_H
#include "Document.h"
#include "mddef.h"
namespace md::editor {
class Editor {
 public:
  void loadFile(const String& path);
  void paintEvent(QPoint offset, Painter& painter);
  [[nodiscard]] int width() const;
  [[nodiscard]] int height() const;

 private:
  sptr<Document> m_doc;
};
}  // namespace md::editor

#endif  // QTMARKDOWN_EDITOR_H
