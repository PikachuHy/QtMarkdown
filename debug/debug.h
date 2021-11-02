//
// Created by PikachuHy on 2021/11/2.
//

#ifndef QTMARKDOWN_DEBUG_H
#define QTMARKDOWN_DEBUG_H
#include <QDebug>
#define DEBUG qDebug().noquote() << "[debug]" << __FUNCTION__ << QString(__FILE_NAME__) + QString(":") + QString::number(__LINE__)

#endif //QTMARKDOWN_DEBUG_H
