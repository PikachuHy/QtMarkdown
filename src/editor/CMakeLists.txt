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
        EditorRenderer.cpp EditorRenderer.h
        EditorInputHandler.cpp EditorInputHandler.h
        MarkdownSerializer.cpp MarkdownSerializer.h
        FileManager.cpp FileManager.h
        Document.cpp Document.h
        Command.cpp Command.h
        CursorCoord.cpp CursorCoord.h
        Cursor.cpp Cursor.h)
target_link_libraries(QtMarkdownEditorCore PUBLIC QtMarkdownRender)
target_link_libraries(QtMarkdownEditorCore PUBLIC QtMarkdownPlatform)

install(
        TARGETS QtMarkdownEditorCore
        EXPORT QtMarkdownEditorCore
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)

markdown_install_headers(QtMarkdownEditorCore PREFIX editor HEADERS Command.h Cursor.h CursorCoord.h Document.h Editor.h EditorInputHandler.h EditorRenderer.h MarkdownSerializer.h FileManager.h core/Types.h core/Event.h core/AbstractPainter.h core/Timer.h)
