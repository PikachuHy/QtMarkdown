//
// Created by PikachuHy on 2021/11/2.
//

#ifndef QTMARKDOWN_DEBUG_H
#define QTMARKDOWN_DEBUG_H
#include <QDebug>
// clang-format off
// 只有文件名
//#define DEBUG qDebug().noquote() << "[debug]" << __FUNCTION__ << QString(__FILE_NAME__) + QString(":") + QString::number(__LINE__)
// 文件绝对路径
#define DEBUG qDebug().noquote() << "[debug]" << __FUNCTION__ << QString(__FILE__) + QString(":") + QString::number(__LINE__)
void backtrace();

#if !defined(ASSERT)
#  if defined(QT_NO_DEBUG) && !defined(QT_FORCE_ASSERTS)
#    define ASSERT(cond) static_cast<void>(false && (cond))
#  else
//#    define ASSERT(cond) ((cond) ? static_cast<void>(0) : qDebug().noquote() qt_assert(#cond, __FILE__, __LINE__))
#    define ASSERT(cond) \
do {\
if ((cond)) {} else {    \
backtrace();                         \
qDebug().noquote() << "[assert]" << #cond << __FUNCTION__ << QString(__FILE__) + QString(":") + QString::number(__LINE__);\
qt_assert(#cond, __FILE__, __LINE__);\
}\
} while (0)
#  endif
#endif
#endif //QTMARKDOWN_DEBUG_H
