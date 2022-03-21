if (BUILD_STATIC)
    add_library(QtMarkdownEditorCore STATIC)
else()
    add_library(QtMarkdownEditorCore SHARED)
    target_compile_definitions(QtMarkdownEditorCore PRIVATE -DQtMarkdown_LIBRARY)
endif ()
target_include_directories(
        QtMarkdownEditorCore PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:include/QtMarkdownEditorCore>
)
target_sources(QtMarkdownEditorCore PRIVATE
        Editor.cpp Editor.h
        Document.cpp Document.h
        Command.cpp Command.h
        CursorCoord.cpp CursorCoord.h
        Cursor.cpp Cursor.h)
target_link_libraries(QtMarkdownEditorCore PUBLIC QtMarkdownRender)
if (BUILD_STATIC)
    add_library(QtQuickMarkdownEditor STATIC)
else ()
    add_library(QtQuickMarkdownEditor SHARED)
    target_compile_definitions(QtQuickMarkdownEditor PRIVATE -DQtMarkdown_LIBRARY)
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
    target_compile_definitions(QtWidgetMarkdownEditor PRIVATE -DQtMarkdown_LIBRARY)
endif ()

target_sources(QtWidgetMarkdownEditor PRIVATE QtWidgetMarkdownEditor.cpp QtWidgetMarkdownEditor.h)
target_link_libraries(QtWidgetMarkdownEditor PUBLIC Qt${QT_VERSION_MAJOR}::Widgets QtMarkdownEditorCore)
set_target_properties(QtWidgetMarkdownEditor PROPERTIES AUTORCC ON)
set_target_properties(QtWidgetMarkdownEditor PROPERTIES AUTOMOC ON)

install(
        TARGETS QtMarkdownEditorCore
        EXPORT QtMarkdownEditorCore
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

markdown_install_headers(QtMarkdownEditorCore PREFIX editor HEADERS Command.h Cursor.h CursorCoord.h Document.h Editor.h mddef.h)
markdown_install_headers(QtWidgetMarkdownEditor PREFIX editor HEADERS QtWidgetMarkdownEditor.h)
markdown_install_headers(QtQuickMarkdownEditor PREFIX editor HEADERS QtQuickMarkdownEditor.h)
