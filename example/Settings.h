//
// Created by PikachuHy on 2021/11/22.
//

#ifndef QTMARKDOWN_SETTINGS_H
#define QTMARKDOWN_SETTINGS_H
#include <QRect>
#include <QSettings>
#include <QString>
class Settings {
 public:
  static Settings *instance();
  QString configStorePath();
  static const char KEY_RECENT_OPEN_FILES[];

  template <const char *key>
  struct QStringRef {
    operator QString() const {
      // 解决中文乱码问题
      return QString::fromUtf8(settings()->value(key).toByteArray());
    }

    QStringRef &operator=(const QString &newValue) {
      settings()->setValue(key, newValue);
      return *this;
    }
  };

  template <const char *key>
  struct IntRef {
    operator int() const { return settings()->value(key).toInt(); }

    IntRef &operator=(const int &newValue) {
      settings()->setValue(key, newValue);
      return *this;
    }
  };

  template <const char *key>
  struct BoolRef {
    operator bool() const { return settings()->value(key).toBool(); }

    BoolRef &operator=(const bool &newValue) {
      settings()->setValue(key, newValue);
      return *this;
    }
  };

  template <const char *key>
  struct QRectRef {
    operator QRect() const { return settings()->value(key).toRect(); }

    QRectRef &operator=(const QRect &newValue) {
      settings()->setValue(key, newValue);
      return *this;
    }
  };
  template <const char *key>
  struct QStringListRef {
    operator QStringList() const { return settings()->value(key).toStringList(); }

    QStringListRef &operator=(const QStringList &newValue) {
      settings()->setValue(key, newValue);
      return *this;
    }
  };

  // 存储格式: 路径 + 英文冒号":" + 标题
  QStringListRef<KEY_RECENT_OPEN_FILES> recentOpenFiles;

  // Singleton, to be used by any part of the app
 private:
  static QSettings *settings();

  Settings();

  Settings(const Settings &other);

  Settings &operator=(const Settings &other);
};

#endif  // QTMARKDOWN_SETTINGS_H
