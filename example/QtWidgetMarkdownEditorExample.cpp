//
// Created by pikachu on 2021/4/26.
//

#include "Editor.h"
#include <QApplication>
int main(int argc, char *argv[]) {
  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::Floor);
  QApplication a(argc, argv);
  Editor w;
  w.loadFile(":/test.md");
  w.resize(800, 600);
  w.show();
  return QApplication::exec();
}