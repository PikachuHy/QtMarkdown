if (BUILD_STATIC)
    add_library(QtMarkdownRender STATIC)
else()
    add_library(QtMarkdownRender SHARED)
endif ()
target_sources(QtMarkdownRender PRIVATE
        Element.cpp Element.h
        Cell.cpp Cell.h
        Render.cpp Render.h
        Instruction.cpp Instruction.h
        StringUtil.cpp StringUtil.h
        ../../assets/md.qrc)
target_compile_definitions(QtMarkdownRender PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownRender PUBLIC Qt${QT_VERSION_MAJOR}::Gui QtMarkdownParser clatexmath)
set_target_properties(QtMarkdownRender PROPERTIES AUTORCC ON)
set_target_properties(QtMarkdownRender PROPERTIES AUTOMOC ON)