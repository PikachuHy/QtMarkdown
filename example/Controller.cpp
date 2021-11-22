//
// Created by PikachuHy on 2021/11/22.
//

#include "Controller.h"

#include "RecentFile.h"
#include "Settings.h"
#include "debug.h"
QList<QObject*> Controller::recentOpenFiles() const {
  QList<QObject*> list;
  QStringList files = Settings::instance()->recentOpenFiles;
  for (int i = 0; i < files.size(); i++) {
    auto arr = files[i].split(":");
    QString path;
    QString title;
    if (arr.empty()) continue;
    if (arr.size() == 1) {
      path = arr[0];
    } else {
      path = arr[0];
      title = arr[1];
    }
    if (path.isEmpty()) continue;
    list.emplace_back(new RecentFile(path, title));
  }
  return list;
}
void Controller::addRecentOpenFile(QString path, QString title) {
  QStringList files = Settings::instance()->recentOpenFiles;
  for (int i = 0; i < files.size(); ++i) {
    auto items = files[i].split(":");
    if (items.empty()) continue;
    auto itemPath = items[0];
    if (itemPath == path) {
      files.removeAt(i);
      break;
    }
  }
  files.append(path + ":" + title);
  Settings::instance()->recentOpenFiles = files;
}
void Controller::clearRecentFiles() { Settings::instance()->recentOpenFiles = QStringList(); }
