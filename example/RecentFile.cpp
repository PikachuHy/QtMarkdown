//
// Created by PikachuHy on 2021/11/22.
//

#include "RecentFile.h"
void RecentFile::setPath(QString path) {
  if (path == m_path) return;
  m_path = path;
  emit pathChanged();
}
void RecentFile::setTitle(QString title) {
  if (title == m_title) return;
  m_title = title;
  emit titleChanged();
}
