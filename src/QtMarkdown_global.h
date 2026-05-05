//
// Created by pikachu on 2021/5/10.
//

#ifndef QTMARKDOWNPARSER_QTMARKDOWN_GLOBAL_H
#define QTMARKDOWNPARSER_QTMARKDOWN_GLOBAL_H

// Platform-independent export macros
#if defined(_WIN32) || defined(_WIN64)
#  if defined(BUILD_STATIC)
#    define QTMARKDOWNSHARED_EXPORT
#  elif defined(QtMarkdown_LIBRARY)
#    define QTMARKDOWNSHARED_EXPORT __declspec(dllexport)
#  else
#    define QTMARKDOWNSHARED_EXPORT __declspec(dllimport)
#  endif
#elif defined(__GNUC__) || defined(__clang__)
#  if defined(BUILD_STATIC)
#    define QTMARKDOWNSHARED_EXPORT
#  elif defined(QtMarkdown_LIBRARY)
#    define QTMARKDOWNSHARED_EXPORT __attribute__((visibility("default")))
#  else
#    define QTMARKDOWNSHARED_EXPORT
#  endif
#else
#  define QTMARKDOWNSHARED_EXPORT
#endif

#if defined(_WIN32) || defined(_WIN64)
#  if defined(QtWidgetMarkdownEditor_LIBRARY)
#    define QTWIDGETMARKDOWNEDITOR_EXPORT __declspec(dllexport)
#  else
#    define QTWIDGETMARKDOWNEDITOR_EXPORT __declspec(dllimport)
#  endif
#elif defined(__GNUC__) || defined(__clang__)
#  if defined(QtWidgetMarkdownEditor_LIBRARY)
#    define QTWIDGETMARKDOWNEDITOR_EXPORT __attribute__((visibility("default")))
#  else
#    define QTWIDGETMARKDOWNEDITOR_EXPORT
#  endif
#else
#  define QTWIDGETMARKDOWNEDITOR_EXPORT
#endif

#if defined(_WIN32) || defined(_WIN64)
#  if defined(QtQuickMarkdownEditor_LIBRARY)
#    define QTQUICKMARKDOWNEDITOR_EXPORT __declspec(dllexport)
#  else
#    define QTQUICKMARKDOWNEDITOR_EXPORT __declspec(dllimport)
#  endif
#elif defined(__GNUC__) || defined(__clang__)
#  if defined(QtQuickMarkdownEditor_LIBRARY)
#    define QTQUICKMARKDOWNEDITOR_EXPORT __attribute__((visibility("default")))
#  else
#    define QTQUICKMARKDOWNEDITOR_EXPORT
#  endif
#else
#  define QTQUICKMARKDOWNEDITOR_EXPORT
#endif

#endif  // QTMARKDOWNPARSER_QTMARKDOWN_GLOBAL_H
