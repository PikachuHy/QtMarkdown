add_library(QtMarkdownEditor SHARED
        Cursor.cpp Cursor.h
        Editor.cpp Editor.h
        Render.cpp Render.h
        EditorDocument.cpp EditorDocument.h
        ../assets/md.qrc)
set_target_properties(QtMarkdownEditor PROPERTIES AUTORCC ON)
target_compile_definitions(QtMarkdownEditor PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownEditor PUBLIC
        clatexmath
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Concurrent
        QtMarkdownParser)
target_include_directories(QtMarkdownEditor INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
add_library(QtQuickMarkdownItem SHARED
        QtQuickMarkdownItem.cpp QtQuickMarkdownItem.h
        )
target_compile_definitions(QtQuickMarkdownItem PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtQuickMarkdownItem PUBLIC QtMarkdownEditor
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Qml
        )
target_include_directories(QtQuickMarkdownItem INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})