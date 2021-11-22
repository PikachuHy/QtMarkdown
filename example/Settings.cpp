//
// Created by PikachuHy on 2021/11/22.
//

#include "Settings.h"

#include <QStandardPaths>
const char Settings::KEY_RECENT_OPEN_FILES[] = "file/recent_open_files";
Settings *Settings::instance() {
  static Settings singleton;
  return &singleton;
}

QString Settings::configStorePath() { return settings()->fileName(); }

Settings::Settings() {}

QSettings *Settings::settings() {
  static QSettings ret(
      QString("%1/config.ini").arg(QStandardPaths::standardLocations(QStandardPaths::AppConfigLocation).first()),
      QSettings::IniFormat);
  return &ret;
}
