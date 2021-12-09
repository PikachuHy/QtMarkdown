//
// Created by pikachu on 2021/4/26.
//

#include <QApplication>

#include "editor/QtWidgetMarkdownEditor.h"
using md::editor::QtWidgetMarkdownEditor;

int main(int argc, char *argv[]) {
  QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
  QApplication a(argc, argv);
#ifdef BUILD_STATIC
  Q_INIT_RESOURCE(md);
#endif
  QApplication::setOrganizationName("PikachuHy");
  QApplication::setOrganizationDomain("pikachu.net.cn");
  QApplication::setApplicationName("QtMarkdownEditor");
  QtWidgetMarkdownEditor w;
  w.resize(800, 600);
  w.loadFile(":/test.md");
  w.show();
  return QApplication::exec();
}