//
// Created by pikachu on 2021/5/10.
//

#ifndef QTMARKDOWNPARSER_QTMARKDOWN_GLOBAL_H
#define QTMARKDOWNPARSER_QTMARKDOWN_GLOBAL_H

// Platform-independent export macros for each library.
// On Windows, each DLL needs its own dllexport/dllimport pair.
// On Linux/macOS, visibility("default") is harmless when shared.

#if defined(_WIN32) || defined(_WIN64)
#  define QTMARKDOWN_EXPORT_DECL(lib) \
    __declspec(dllexport)
#  define QTMARKDOWN_IMPORT_DECL(lib) \
    __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
#  define QTMARKDOWN_EXPORT_DECL(lib) \
    __attribute__((visibility("default")))
#  define QTMARKDOWN_IMPORT_DECL(lib)
#else
#  define QTMARKDOWN_EXPORT_DECL(lib)
#  define QTMARKDOWN_IMPORT_DECL(lib)
#endif

#ifdef BUILD_STATIC
#  define QTMARKDOWNPARSER_EXPORT
#  define QTMARKDOWNRENDER_EXPORT
#  define QTMARKDOWNEDITORCORE_EXPORT
#  define QTWIDGETMARKDOWNEDITOR_EXPORT
#  define QTQUICKMARKDOWNEDITOR_EXPORT
#else
#  ifdef QtMarkdownParser_LIBRARY
#    define QTMARKDOWNPARSER_EXPORT QTMARKDOWN_EXPORT_DECL(QtMarkdownParser)
#  else
#    define QTMARKDOWNPARSER_EXPORT QTMARKDOWN_IMPORT_DECL(QtMarkdownParser)
#  endif
#  ifdef QtMarkdownRender_LIBRARY
#    define QTMARKDOWNRENDER_EXPORT QTMARKDOWN_EXPORT_DECL(QtMarkdownRender)
#  else
#    define QTMARKDOWNRENDER_EXPORT QTMARKDOWN_IMPORT_DECL(QtMarkdownRender)
#  endif
#  ifdef QtMarkdownEditorCore_LIBRARY
#    define QTMARKDOWNEDITORCORE_EXPORT QTMARKDOWN_EXPORT_DECL(QtMarkdownEditorCore)
#  else
#    define QTMARKDOWNEDITORCORE_EXPORT QTMARKDOWN_IMPORT_DECL(QtMarkdownEditorCore)
#  endif
#  ifdef QtWidgetMarkdownEditor_LIBRARY
#    define QTWIDGETMARKDOWNEDITOR_EXPORT QTMARKDOWN_EXPORT_DECL(QtWidgetMarkdownEditor)
#  else
#    define QTWIDGETMARKDOWNEDITOR_EXPORT QTMARKDOWN_IMPORT_DECL(QtWidgetMarkdownEditor)
#  endif
#  ifdef QtQuickMarkdownEditor_LIBRARY
#    define QTQUICKMARKDOWNEDITOR_EXPORT QTMARKDOWN_EXPORT_DECL(QtQuickMarkdownEditor)
#  else
#    define QTQUICKMARKDOWNEDITOR_EXPORT QTMARKDOWN_IMPORT_DECL(QtQuickMarkdownEditor)
#  endif
#endif


#endif  // QTMARKDOWNPARSER_QTMARKDOWN_GLOBAL_H
