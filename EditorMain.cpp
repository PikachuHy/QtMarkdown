//
// Created by pikachu on 2021/4/26.
//

#include <QApplication>
#include "Editor.h"
int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    Editor w;
    w.resize(800, 600);
    w.show();
    return QApplication::exec();
}