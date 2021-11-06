//
// Created by PikachuHy on 2021/11/6.
//

#include "QtQuickMarkdownEditor.h"
using namespace md::editor;
QtQuickMarkdownEditor::QtQuickMarkdownEditor(QQuickItem *parent) : QQuickPaintedItem(parent) {
  m_editor = std::make_shared<Editor>();
  setAcceptHoverEvents(true);
  setAcceptedMouseButtons(Qt::AllButtons);
  setFlag(ItemAcceptsInputMethod, true);
}
void QtQuickMarkdownEditor::paint(QPainter *painter) {
  Q_ASSERT(painter != nullptr);
  m_editor->paintEvent(QPoint(0, 0), *painter);
}
void QtQuickMarkdownEditor::setText(const QString &text) {}
void QtQuickMarkdownEditor::setSource(const QString &source) {
  m_editor->loadFile(source);
  setImplicitWidth(m_editor->width());
  setImplicitHeight(m_editor->height());
}
void QtQuickMarkdownEditor::setPath(const QString &path) {}
