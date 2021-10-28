if (QT_VERSION_MAJOR EQUAL 6)
    qt_add_executable(QtMarkdownParserExample
            MANUAL_FINALIZATION
            QtMarkdownParserExample.cpp
            ../assets/md.qrc
            )
    qt_add_executable(QtWidgetMarkdownEditorExample
            MANUAL_FINALIZATION
            QtWidgetMarkdownEditorExample.cpp
            )
    qt_add_executable(QtQuickMarkdownEditorExample
            MANUAL_FINALIZATION
            QtQuickMarkdownEditorExample.cpp qml.qrc
            QtQuickMarkdownItemPlugin.cpp QtQuickMarkdownItemPlugin.h
            )
else ()
    add_executable(QtMarkdownParserExample QtMarkdownParserExample.cpp ../assets/md.qrc)
    add_executable(QtWidgetMarkdownEditorExample QtWidgetMarkdownEditorExample.cpp)
    add_executable(QtQuickMarkdownEditorMain
            QtQuickMarkdownEditorExample.cpp qml.qrc
            QtQuickMarkdownItemPlugin.cpp QtQuickMarkdownItemPlugin.h
            )
endif ()
target_link_libraries(QtMarkdownParserExample PRIVATE QtMarkdownParser)
set_target_properties(QtMarkdownParserExample PROPERTIES AUTORCC ON)
target_link_libraries(QtWidgetMarkdownEditorExample PRIVATE QtMarkdownEditor)
set_target_properties(QtQuickMarkdownEditorExample PROPERTIES AUTORCC ON)
target_link_libraries(QtQuickMarkdownEditorExample PRIVATE QtMarkdownEditor
        QtQuickMarkdownItem
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        )
if (QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(QtMarkdownParserExample)
    qt_finalize_executable(QtWidgetMarkdownEditorExample)
    qt_finalize_executable(QtQuickMarkdownEditorExample)
endif ()