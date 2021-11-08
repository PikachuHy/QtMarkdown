add_library(QtMarkdownEditor SHARED
        Editor.cpp Editor.h
        Document.cpp Document.h
        Cursor.cpp Cursor.h)
target_compile_definitions(QtMarkdownEditor PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownEditor PUBLIC QtMarkdownRender)