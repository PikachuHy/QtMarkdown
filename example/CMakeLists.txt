find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Quick Qml Widgets Concurrent REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Quick Qml Widgets Concurrent REQUIRED)
if (QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(QtMarkdownParserExample
            MANUAL_FINALIZATION
            QtMarkdownParserExample.cpp
            ../assets/md.qrc
            )
    qt_add_executable(QtWidgetMarkdownEditorExample
            MANUAL_FINALIZATION
            QtWidgetMarkdownEditorExample.cpp
            QtWidgetMarkdownEditor.cpp QtWidgetMarkdownEditor.h
            ../assets/md.qrc
            )
    qt_add_executable(QtQuickMarkdownEditorExample
            MANUAL_FINALIZATION
            QtQuickMarkdownEditorExample.cpp
            QtQuickMarkdownEditor.cpp QtQuickMarkdownEditor.h
            ../assets/md.qrc
            qml.qrc
            )
else ()
    add_executable(QtMarkdownParserExample QtMarkdownParserExample.cpp ../assets/md.qrc)
    add_executable(QtWidgetMarkdownEditorExample QtWidgetMarkdownEditorExample.cpp)
    add_executable(QtQuickMarkdownEditorMain
            QtQuickMarkdownEditorExample.cpp
            ../assets/md.qrc qml.qrc
            QtQuickMarkdownEditor.cpp QtQuickMarkdownEditor.h
            )
endif ()
target_link_libraries(QtMarkdownParserExample PRIVATE QtMarkdownParser)
set_target_properties(QtMarkdownParserExample PROPERTIES AUTORCC ON)
set_target_properties(QtWidgetMarkdownEditorExample PROPERTIES AUTORCC ON)
target_link_libraries(QtWidgetMarkdownEditorExample PRIVATE Qt${QT_VERSION_MAJOR}::Widgets QtMarkdownEditor)
set_target_properties(QtQuickMarkdownEditorExample PROPERTIES AUTORCC ON)
set_target_properties(QtQuickMarkdownEditorExample PROPERTIES AUTOMOC ON)
target_link_libraries(QtQuickMarkdownEditorExample PRIVATE QtMarkdownEditor
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml)

if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QtMarkdownParserExample)
    qt_finalize_executable(QtWidgetMarkdownEditorExample)
    qt_finalize_executable(QtQuickMarkdownEditorExample)
endif ()