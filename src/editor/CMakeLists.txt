add_library(QtMarkdownEditorCore SHARED
        Editor.cpp Editor.h
        Document.cpp Document.h
        CursorCoord.cpp CursorCoord.h
        Cursor.cpp Cursor.h)
target_compile_definitions(QtMarkdownEditorCore PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownEditorCore PUBLIC QtMarkdownRender)
add_library(QtQuickMarkdownEditor SHARED
        QtQuickMarkdownEditor.cpp QtQuickMarkdownEditor.h
        )
target_compile_definitions(QtQuickMarkdownEditor PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtQuickMarkdownEditor PUBLIC
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        QtMarkdownEditorCore)
set_target_properties(QtQuickMarkdownEditor PROPERTIES AUTORCC ON)
set_target_properties(QtQuickMarkdownEditor PROPERTIES AUTOMOC ON)
add_library(QtWidgetMarkdownEditor SHARED
        QtWidgetMarkdownEditor.cpp QtWidgetMarkdownEditor.h
        )
target_compile_definitions(QtWidgetMarkdownEditor PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtWidgetMarkdownEditor PUBLIC Qt${QT_VERSION_MAJOR}::Widgets QtMarkdownEditorCore)
set_target_properties(QtWidgetMarkdownEditor PROPERTIES AUTORCC ON)
set_target_properties(QtWidgetMarkdownEditor PROPERTIES AUTOMOC ON)
