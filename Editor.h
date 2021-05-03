//
// Created by pikachu on 2021/3/19.
//

#ifndef QTMARKDOWNPARSER_EDITOR_H
#define QTMARKDOWNPARSER_EDITOR_H
#include <QScrollArea>
class EditorWidget;
class Editor: public QScrollArea {
    Q_OBJECT
public:
    explicit Editor(QWidget *parent = nullptr);
    void loadFile(const QString& path);
    void reload();
private:
    EditorWidget *m_editorWidget;
};


#endif //QTMARKDOWNPARSER_EDITOR_H
