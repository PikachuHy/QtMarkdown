cmake_minimum_required(VERSION 3.20)
project(QtMarkdown
        VERSION 1.0.0
        DESCRIPTION "A simple markdown editor"
        )
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory(3rd/MicroTeX)
add_subdirectory(3rd/magic_enum)
if (MSVC)
    add_compile_options("/utf-8")
    if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_LIST_DIR})
        set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
        set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
    endif ()
endif ()
# find_package(magic_enum REQUIRED)
#option(USE_LOCAL_DEPS "use local dependency" ON)
#if (USE_LOCAL_DEPS)
#    add_subdirectory(deps/magic_enum)
#    set(QT ON)
#    set(HAVE_LOG OFF)
#    add_subdirectory(deps/cLaTeXMath)
#else()
#    include(FetchContent)
#    message(STATUS "add dependency: Magic Enum C++")
#    FetchContent_Declare(
#            MagicEnum
#            GIT_REPOSITORY https://github.com/Neargye/magic_enum.git
#            GIT_PROGRESS ON
#            GIT_SHALLOW ON
#    )
#    FetchContent_MakeAvailable(MagicEnum)
#    message(STATUS "add dependency: cLaTeXMath")
#    FetchContent_Declare(
#            cLaTeXMath
#            GIT_REPOSITORY https://github.com/PikachuHy/cLaTeXMath.git
#            GIT_PROGRESS ON
#            GIT_SHALLOW ON
#            GIT_TAG openmath-fix
#    )
#    set(QT ON)
#    set(HAVE_LOG OFF)
#    FetchContent_MakeAvailable(cLaTeXMath)
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Quick Qml Widgets Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Quick Qml Widgets Concurrent REQUIRED)
option(BUILD_STATIC "build static library" OFF)
if (BUILD_STATIC)
    add_definitions(-DBUILD_STATIC)
endif ()
include(MarkdownInstall.cmake)
add_subdirectory(src)
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_LIST_DIR})
    add_subdirectory(example)
endif ()
option(BUILD_TEST "build unit test" OFF)
if (BUILD_TEST)
    # add_subdirectory(deps/Catch2)
    add_subdirectory(test)
endif ()
