//
// Created by PikachuHy on 2021/11/22.
//

#ifndef QTMARKDOWN_CONTROLLER_H
#define QTMARKDOWN_CONTROLLER_H
#include <QObject>
class Controller : public QObject {
  Q_OBJECT
 public:
  Q_INVOKABLE QList<QObject*> recentOpenFiles() const;
  Q_INVOKABLE void addRecentOpenFile(QString path, QString title);
  Q_INVOKABLE void clearRecentFiles();

 private:
};

#endif  // QTMARKDOWN_CONTROLLER_H
