//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_QTWIDGETMARKDOWNEDITOR_H
#define QTMARKDOWN_QTWIDGETMARKDOWNEDITOR_H

#include <QAbstractScrollArea>
#include <QTimer>
#include <QWidget>

#include "editor/Editor.h"
namespace md::editor {
class QTMARKDOWNSHARED_EXPORT QtWidgetMarkdownEditor : public QAbstractScrollArea {
  Q_OBJECT
 public:
  explicit QtWidgetMarkdownEditor(QWidget* parent = nullptr);
  void loadFile(QString path);
  QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;
  void reload();

 protected:
  void paintEvent(QPaintEvent* event) override;
  void scrollContentsBy(int dx, int dy) override;
  [[nodiscard]] QSize viewportSizeHint() const override;
  void keyPressEvent(QKeyEvent* event) override;
  void inputMethodEvent(QInputMethodEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;

 private:
  std::shared_ptr<md::editor::Editor> m_editor;
  QPoint m_offset;
  QTimer m_cursorTimer;
};
}  // namespace md::editor
#endif  // QTMARKDOWN_QTWIDGETMARKDOWNEDITOR_H
