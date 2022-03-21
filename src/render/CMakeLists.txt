find_package(microtex REQUIRED)
if (BUILD_STATIC)
    add_library(QtMarkdownRender STATIC)
else()
    add_library(QtMarkdownRender SHARED)
endif ()
target_include_directories(
        QtMarkdownParser PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:include/QtMarkdownRender>
)
target_sources(QtMarkdownRender PRIVATE
        Element.cpp Element.h
        Cell.cpp Cell.h
        Render.cpp Render.h
        Instruction.cpp Instruction.h
        StringUtil.cpp StringUtil.h
        ../../assets/md.qrc)
target_compile_definitions(QtMarkdownRender PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownRender PUBLIC Qt${QT_VERSION_MAJOR}::Gui QtMarkdownParser microtex::microtex)
set_target_properties(QtMarkdownRender PROPERTIES AUTORCC ON)
set_target_properties(QtMarkdownRender PROPERTIES AUTOMOC ON)
install(
        TARGETS QtMarkdownRender
        EXPORT QtMarkdownRender
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)

markdown_install_headers(QtMarkdownRender PREFIX render
        HEADERS Element.h Cell.h Render.h Instruction.h StringUtil.h mddef.h
        )