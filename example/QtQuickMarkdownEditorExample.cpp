//
// Created by pikachu on 5/22/2021.
//

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include "editor/QtQuickMarkdownEditor.h"
#include "Controller.h"
using md::editor::QtQuickMarkdownEditor;
int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
  QGuiApplication app(argc, argv);
  QGuiApplication::setOrganizationName("PikachuHy");
  QGuiApplication::setOrganizationDomain("pikachu.net.cn");
  QGuiApplication::setApplicationName("QtMarkdownEditor");
#ifdef BUILD_STATIC
  Q_INIT_RESOURCE(md);
#endif
#ifdef Q_OS_WIN
  QQuickStyle::setStyle("Windows");
#else
  QQuickStyle::setStyle("macOS");
#endif
  QQmlApplicationEngine engine;
  qmlRegisterType<QtQuickMarkdownEditor>("QtMarkdown", 1, 0, "QtQuickMarkdownEditor");
  qmlRegisterType<Controller>("Controller", 1, 0, "Controller");
  const QUrl url(QStringLiteral("qrc:/main.qml"));
  QObject::connect(
      &engine, &QQmlApplicationEngine::objectCreated, &app, [url](QObject *obj, const QUrl &objUrl) {
                if (!obj && url == objUrl)
                    QCoreApplication::exit(-1);
            }, Qt::QueuedConnection);

    engine.load(url);

    return QGuiApplication::exec();
}
