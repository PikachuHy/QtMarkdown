add_library(QtMarkdownRender SHARED
        Render.cpp Render.h
        Instruction.cpp Instruction.h)
target_compile_definitions(QtMarkdownRender PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownRender PUBLIC Qt${QT_VERSION_MAJOR}::Gui QtMarkdownParser clatexmath)