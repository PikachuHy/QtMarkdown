//
// Created by PikachuHy on 2021/11/9.
//
#ifndef _MSC_VER
#include <cxxabi.h>
#include <libunwind.h>
#endif
#include <cstdio>
#include <cstdlib>
//#define BOOST_STACKTRACE_GNU_SOURCE_NOT_REQUIRED
//#define _GNU_SOURCE
#include <boost/stacktrace.hpp>
#include <iostream>
#include "debug.h"
void Backtrace::backtrace() {
  std::cout << boost::stacktrace::stacktrace();
#ifndef _MSC_VER
  unw_cursor_t cursor;
  unw_context_t context;

  // Initialize cursor to current frame for local unwinding.
  unw_getcontext(&context);
  unw_init_local(&cursor, &context);

  // Unwind frames one by one, going up the frame stack.
  while (unw_step(&cursor) > 0) {
    unw_word_t offset, pc;
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    if (pc == 0) {
      break;
    }
    std::printf("0x%llx:", pc);

    char sym[256];
    if (unw_get_proc_name(&cursor, sym, sizeof(sym), &offset) == 0) {
      char* nameptr = sym;
      int status;
      char* demangled = abi::__cxa_demangle(sym, nullptr, nullptr, &status);
      if (status == 0) {
        nameptr = demangled;
      }
      std::printf(" (%s+0x%llx)\n", nameptr, offset);
      std::free(demangled);
    } else {
      std::printf(" -- error: unable to obtain symbol name for this frame\n");
    }
  }
#endif
}