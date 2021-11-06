//
// Created by pikachu on 2021/4/26.
//

#include <QApplication>

#include "QtWidgetMarkdownEditor.h"
int main(int argc, char *argv[]) {
  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
      Qt::HighDpiScaleFactorRoundingPolicy::Floor);
  QApplication a(argc, argv);
  QtWidgetMarkdownEditor w;
  w.resize(800, 600);
  w.loadFile(":/test.md");
  w.show();
  return QApplication::exec();
}