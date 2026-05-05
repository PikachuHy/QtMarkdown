//
// Created by PikachuHy on 2021/11/2.
//

#ifndef QTMARKDOWN_DEBUG_H
#define QTMARKDOWN_DEBUG_H
#include <cstdlib>
#include <iostream>
// clang-format off
// 只有文件名
//#define DEBUG std::cout << "[debug]" << __FUNCTION__ << " " << __FILE_NAME__ << ":" << __LINE__
// 文件绝对路径
#define DEBUG std::cout << "[debug] " << __FUNCTION__ \
    << " " << __FILE__ << ":" << __LINE__ << " "

class Backtrace {
public:
static void backtrace();
};

#if !defined(ASSERT)
#define ASSERT(cond) \
do {\
if ((cond)) {} else {    \
Backtrace::backtrace();                         \
std::cerr << "[assert] " << #cond << " " << __FUNCTION__ \
<< " " << __FILE__ << ":" << __LINE__ << std::endl;\
std::abort();\
}\
} while (0)
#endif
#endif //QTMARKDOWN_DEBUG_H
