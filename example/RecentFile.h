//
// Created by PikachuHy on 2021/11/22.
//

#ifndef QTMARKDOWN_RECENTFILE_H
#define QTMARKDOWN_RECENTFILE_H
#include <QObject>
#include <QString>
class RecentFile : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
  Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
 public:
  RecentFile(QString path, QString title) : m_path(path), m_title(title) {}
  QString path() const { return m_path; }
  void setPath(QString path);
  QString title() const { return m_title; }
  void setTitle(QString title);
 signals:
  void pathChanged();
  void titleChanged();

 private:
  QString m_path;
  QString m_title;
};

#endif  // QTMARKDOWN_RECENTFILE_H
