//
// Created by pikachu on 2021/3/19.
//

#include "Editor.h"
#include <QApplication>
#include <QDate>

Editor::Editor(QWidget *parent) : QTextEdit(parent) {
    QTextCharFormat backgroundFormat;
    backgroundFormat.setBackground(QColor("lightGray"));
    QTextCursor cursor(textCursor());
    cursor.insertText(tr("Character formats"),
                      backgroundFormat);

    cursor.insertBlock();

    cursor.insertText(tr("Text can be displayed in a variety of "
                         "different character formats. "), backgroundFormat);
    cursor.insertText(tr("We can emphasize text by "));
    cursor.insertText(tr("making it italic"), backgroundFormat);
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Editor w;
    w.show();
    return QApplication::exec();
}