//
// Created by PikachuHy on 2021/11/6.
//

#ifndef QTMARKDOWN_QTQUICKMARKDOWNEDITOR_H
#define QTMARKDOWN_QTQUICKMARKDOWNEDITOR_H
#include <QQuickPaintedItem>
#include <QTimer>

#include "editor/Editor.h"
class QTMARKDOWNSHARED_EXPORT QtQuickMarkdownEditor : public QQuickPaintedItem {
  Q_OBJECT
  Q_PROPERTY(QString text READ text WRITE setText)
  Q_PROPERTY(QString source READ source WRITE setSource)
  Q_PROPERTY(QString path READ path WRITE setPath)
  QML_ELEMENT
  QML_NAMED_ELEMENT(QtQuickMarkdownEditor)
 public:
  explicit QtQuickMarkdownEditor(QQuickItem *parent = nullptr);
  void paint(QPainter *painter) override;
  [[nodiscard]] QString text() const { return m_text; };
  void setText(const QString &text);
  [[nodiscard]] QString source() const { return m_source; };
  void setSource(const QString &source);
  [[nodiscard]] QString path() const { return m_path; };
  void setPath(const QString &path);

 protected:
  void hoverMoveEvent(QHoverEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void focusOutEvent(QFocusEvent *event) override;

 public:
  QVariant inputMethodQuery(Qt::InputMethodQuery query) const override;

 protected:
  void inputMethodEvent(QInputMethodEvent *event) override;

 private:
  QString m_text;
  QString m_source;
  QString m_path;
  std::shared_ptr<md::editor::Editor> m_editor;
  QTimer m_cursorTimer;
};

#endif  // QTMARKDOWN_QTQUICKMARKDOWNEDITOR_H
