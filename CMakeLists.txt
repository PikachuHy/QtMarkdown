cmake_minimum_required(VERSION 3.6)
project(QtMarkdown)
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
add_library(QtMarkdownEditor SHARED Editor.cpp md.qrc)
set_target_properties(QtMarkdownEditor PROPERTIES AUTORCC ON)
target_compile_definitions(QtMarkdownEditor PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownEditor PUBLIC
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Concurrent
        QtMarkdownParser)
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_LIST_DIR})
    if (ANDROID)
        message(STATUS "andorid abi ${CMAKE_ANDROID_ARCH_ABI}")
        set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/android-build/libs/${CMAKE_ANDROID_ARCH_ABI}/)
        set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/android-build/libs/${CMAKE_ANDROID_ARCH_ABI}/)
    endif ()
    qt_add_executable(QtMarkdown
            MANUAL_FINALIZATION
            main.cpp
            )
    target_link_libraries(QtMarkdown PRIVATE QtMarkdownParser)

    qt_add_executable(QtMarkdownEditorMain
            MANUAL_FINALIZATION
            EditorMain.cpp
            )
    target_link_libraries(QtMarkdownEditorMain PRIVATE QtMarkdownEditor)
    option(BUILD_TEST "build unit test" OFF)
    if (BUILD_TEST)
        add_subdirectory(test)
    endif()
    if (QT_VERSION_MAJOR EQUAL 6)
        qt_finalize_executable(QtMarkdown)
        qt_finalize_executable(QtMarkdownEditorMain)
    endif ()
endif()
