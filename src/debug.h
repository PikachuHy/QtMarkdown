//
// Created by PikachuHy on 2021/11/2.
//

#ifndef QTMARKDOWN_DEBUG_H
#define QTMARKDOWN_DEBUG_H
#include <cstdlib>
#include <iostream>

// Debug logging — off by default. Define QTMARKDOWN_ENABLE_DEBUG to enable.
#ifdef QTMARKDOWN_ENABLE_DEBUG
#define DEBUG std::cout << "[debug] " << __FUNCTION__ \
    << " " << __FILE__ << ":" << __LINE__ << " "
#else
#define DEBUG if (true) {} else std::cout
#endif

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
