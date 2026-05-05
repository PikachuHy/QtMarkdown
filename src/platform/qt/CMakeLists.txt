add_library(QtMarkdownPlatform STATIC)
target_include_directories(QtMarkdownPlatform PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
)
target_sources(QtMarkdownPlatform PRIVATE
    QtAdapters.h
    QtImageProvider.h
    QtFontMetricsProvider.h
    QtLatexPlatform.h
    ../../../assets/md.qrc
)
set_target_properties(QtMarkdownPlatform PROPERTIES AUTORCC ON)
target_link_libraries(QtMarkdownPlatform PUBLIC
    Qt${QT_VERSION_MAJOR}::Gui
    microtex-qt
    microtex
)

if (BUILD_STATIC)
    add_library(QtQuickMarkdownEditor STATIC)
else ()
    add_library(QtQuickMarkdownEditor SHARED)
    target_compile_definitions(QtQuickMarkdownEditor PRIVATE -DQtQuickMarkdownEditor_LIBRARY)
endif ()
target_sources(QtQuickMarkdownEditor PRIVATE QtQuickMarkdownEditor.cpp QtQuickMarkdownEditor.h)
target_link_libraries(QtQuickMarkdownEditor PUBLIC
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        QtMarkdownEditorCore)
set_target_properties(QtQuickMarkdownEditor PROPERTIES AUTORCC ON)
set_target_properties(QtQuickMarkdownEditor PROPERTIES AUTOMOC ON)

if (BUILD_STATIC)
    add_library(QtWidgetMarkdownEditor STATIC)
else ()
    add_library(QtWidgetMarkdownEditor SHARED)
    target_compile_definitions(QtWidgetMarkdownEditor PRIVATE -DQtWidgetMarkdownEditor_LIBRARY)
endif ()
target_sources(QtWidgetMarkdownEditor PRIVATE QtWidgetMarkdownEditor.cpp QtWidgetMarkdownEditor.h)
target_link_libraries(QtWidgetMarkdownEditor PUBLIC Qt${QT_VERSION_MAJOR}::Widgets QtMarkdownEditorCore)
set_target_properties(QtWidgetMarkdownEditor PROPERTIES AUTORCC ON)
set_target_properties(QtWidgetMarkdownEditor PROPERTIES AUTOMOC ON)

install(
    TARGETS QtMarkdownPlatform
    EXPORT QtMarkdownPlatform
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(
    TARGETS QtWidgetMarkdownEditor
    EXPORT QtWidgetMarkdownEditor
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(
    TARGETS QtQuickMarkdownEditor
    EXPORT QtQuickMarkdownEditor
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

markdown_install_headers(QtMarkdownPlatform PREFIX platform/qt
    HEADERS QtAdapters.h QtImageProvider.h QtFontMetricsProvider.h QtLatexPlatform.h
)
markdown_install_headers(QtQuickMarkdownEditor PREFIX platform/qt
    HEADERS QtQuickMarkdownEditor.h
)
markdown_install_headers(QtWidgetMarkdownEditor PREFIX platform/qt
    HEADERS QtWidgetMarkdownEditor.h
)
