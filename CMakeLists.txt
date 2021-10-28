cmake_minimum_required(VERSION 3.6)
project(QtMarkdown)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options("/utf-8")
endif()
include(FetchContent)
message(STATUS "add dependency: Magic Enum C++")
FetchContent_Declare(
        MagicEnum
        GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
        GIT_PROGRESS ON
        GIT_SHALLOW ON
)
FetchContent_MakeAvailable(MagicEnum)
message(STATUS "add dependency: cLaTeXMath")
FetchContent_Declare(
        cLaTeXMath
        GIT_REPOSITORY https://github.com/PikachuHy/cLaTeXMath.git
        GIT_PROGRESS ON
        GIT_SHALLOW ON
        GIT_TAG openmath-fix
)
set(QT ON)
set(HAVE_LOG OFF)
FetchContent_MakeAvailable(cLaTeXMath)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Qml REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Qml REQUIRED)
add_library(QtMarkdownParser SHARED
        Document.cpp Document.h
        Token.cpp Token.h
        Parser.cpp Parser.h
        Visitor.cpp Visitor.h
        )
target_compile_definitions(QtMarkdownParser PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownParser PUBLIC Qt${QT_VERSION_MAJOR}::Core magic_enum::magic_enum)
target_include_directories(QtMarkdownParser INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
        Widgets
        Concurrent
        REQUIRED)
add_library(QtMarkdownEditor SHARED
        Cursor.cpp Cursor.h
        Editor.cpp Editor.h
        Render.cpp Render.h
        md.qrc)
set_target_properties(QtMarkdownEditor PROPERTIES AUTORCC ON)
target_compile_definitions(QtMarkdownEditor PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownEditor PUBLIC
        clatexmath
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Concurrent
        QtMarkdownParser)
add_library(QtQuickMarkdownItem SHARED QtQuickMarkdownItem.cpp QtQuickMarkdownItem.h

        )
target_compile_definitions(QtQuickMarkdownItem PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtQuickMarkdownItem PUBLIC QtMarkdownEditor
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        )
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_LIST_DIR})
    if (ANDROID)
        message(STATUS "andorid abi ${CMAKE_ANDROID_ARCH_ABI}")
        set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/android-build/libs/${CMAKE_ANDROID_ARCH_ABI}/)
        set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/android-build/libs/${CMAKE_ANDROID_ARCH_ABI}/)
    endif ()
    if (QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(QtMarkdown
            MANUAL_FINALIZATION
            main.cpp
            )
    else()
        add_executable(QtMarkdown main.cpp)
    endif()
    target_link_libraries(QtMarkdown PRIVATE QtMarkdownParser)
    if (QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(QtMarkdownEditorMain
            MANUAL_FINALIZATION
            EditorMain.cpp
            )
    else()
    add_executable(QtMarkdownEditorMain EditorMain.cpp)
    endif()
    target_link_libraries(QtMarkdownEditorMain PRIVATE QtMarkdownEditor)
    if (QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(QtQuickMarkdownEditorMain
            MANUAL_FINALIZATION
            Cursor.cpp Cursor.h
            QtQuickMarkdown.cpp qml.qrc
            QtQuickMarkdownItemPlugin.cpp QtQuickMarkdownItemPlugin.h
            )
    else()
        add_executable(QtQuickMarkdownEditorMain
                Cursor.cpp Cursor.h
                QtQuickMarkdown.cpp qml.qrc
                QtQuickMarkdownItemPlugin.cpp QtQuickMarkdownItemPlugin.h
                )
    endif()
    set_target_properties(QtQuickMarkdownEditorMain PROPERTIES AUTORCC ON)
    target_link_libraries(QtQuickMarkdownEditorMain PRIVATE QtMarkdownEditor
            QtQuickMarkdownItem
            Qt${QT_VERSION_MAJOR}::Quick
            Qt${QT_VERSION_MAJOR}::Qml
            )
    option(BUILD_TEST "build unit test" OFF)
    if (BUILD_TEST)
        add_subdirectory(test)
    endif()
    if (QT_VERSION_MAJOR EQUAL 6)
        qt_finalize_executable(QtMarkdown)
        qt_finalize_executable(QtMarkdownEditorMain)
        qt_finalize_executable(QtQuickMarkdownEditorMain)
    endif ()
endif()
