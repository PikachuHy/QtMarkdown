cmake_minimum_required(VERSION 3.6)
project(QtMarkdownParser)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options("/utf-8")
endif()
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
        Core
        REQUIRED)
add_library(QtMarkdownParser SHARED Document.cpp Document.h Token.cpp Token.h Parser.cpp Parser.h Visitor.cpp Visitor.h)
target_compile_definitions(QtMarkdownParser PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownParser PUBLIC Qt${QT_VERSION_MAJOR}::Core)
target_include_directories(QtMarkdownParser INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
        Widgets
        Concurrent
        REQUIRED)
add_library(QtMarkdownEditor SHARED Editor.cpp)
target_compile_definitions(QtMarkdownEditor PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownEditor PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Concurrent
        QtMarkdownParser)
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_LIST_DIR})
    add_executable(QtMarkdown main.cpp)
    target_link_libraries(QtMarkdown PRIVATE QtMarkdownParser)
    add_executable(QtMarkdownEditorMain EditorMain.cpp)
    target_link_libraries(QtMarkdownEditorMain PRIVATE QtMarkdownEditor)
    add_subdirectory(test)
endif()