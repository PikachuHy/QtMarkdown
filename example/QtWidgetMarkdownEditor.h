//
// Created by PikachuHy on 2021/11/5.
//

#ifndef QTMARKDOWN_QTWIDGETMARKDOWNEDITOR_H
#define QTMARKDOWN_QTWIDGETMARKDOWNEDITOR_H

#include <QAbstractScrollArea>
#include <QWidget>

#include "editor/Editor.h"
class QtWidgetMarkdownEditor : public QAbstractScrollArea {
  Q_OBJECT
 public:
  explicit QtWidgetMarkdownEditor(QWidget* parent = nullptr);
  void loadFile(QString path);

 protected:
  void paintEvent(QPaintEvent* event) override;
  void scrollContentsBy(int dx, int dy) override;
  [[nodiscard]] QSize viewportSizeHint() const override;

 private:
  std::shared_ptr<md::editor::Editor> m_editor;
  QPoint m_offset;
};

#endif  // QTMARKDOWN_QTWIDGETMARKDOWNEDITOR_H
