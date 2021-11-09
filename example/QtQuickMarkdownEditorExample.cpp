//
// Created by pikachu on 5/22/2021.
//

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include "editor/QtQuickMarkdownEditor.h"
using md::editor::QtQuickMarkdownEditor;
int main(int argc, char *argv[]) {
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
  QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
  QGuiApplication app(argc, argv);
  QQuickStyle::setStyle("macOS");
  QQmlApplicationEngine engine;
  qmlRegisterType<QtQuickMarkdownEditor>("QtMarkdown", 1, 0, "QtQuickMarkdownEditor");
  const QUrl url(QStringLiteral("qrc:/main.qml"));
  QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
                if (!obj && url == objUrl)
                    QCoreApplication::exit(-1);
            }, Qt::QueuedConnection);

    engine.load(url);

    return QGuiApplication::exec();
}
