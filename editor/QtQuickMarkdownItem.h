//
// Created by pikachu on 5/22/2021.
//

#ifndef QTMARKDOWN_QTQUICKMARKDOWNITEM_H
#define QTMARKDOWN_QTQUICKMARKDOWNITEM_H
#include "QtMarkdown_global.h"
#include <QQuickPaintedItem>
#include <QTimer>
class Render;
class Cursor;
class EditorDocument;
class QTMARKDOWNSHARED_EXPORT QtQuickMarkdownItem : public QQuickPaintedItem {
  Q_OBJECT
  Q_PROPERTY(QString text READ text WRITE setText)
  Q_PROPERTY(QString source READ source WRITE setSource)
  Q_PROPERTY(QString path READ path WRITE setPath)
  QML_ELEMENT
  QML_NAMED_ELEMENT(QtQuickMarkdownItem)
public:
  explicit QtQuickMarkdownItem(QQuickItem *parent = nullptr);
  ~QtQuickMarkdownItem();
  void paint(QPainter *painter) override;
  [[nodiscard]] QString text() const;
  void setText(const QString &text);
  [[nodiscard]] QString source() const;
  void setSource(const QString &source);
  [[nodiscard]] QString path() const;
  void setPath(const QString &path);
signals:
  void codeCopied(QString code);
  void linkClicked(QString url);
  void imageClicked(QString path);

protected:
  void mousePressEvent(QMouseEvent *event) override;
  void hoverMoveEvent(QHoverEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void focusInEvent(QFocusEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;

private:
  void calculateHeight();

private:
  QString m_text;
  QString m_source;
  QString m_path;
  Render *m_render;
  Cursor *m_cursor;
  EditorDocument *m_doc;
  int m_lastWidth;
  int m_lastImplicitWidth;
  QTimer m_cursorTimer;
  bool m_holdCtrl;
};

#endif // QTMARKDOWN_QTQUICKMARKDOWNITEM_H
