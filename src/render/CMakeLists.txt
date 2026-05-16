if (BUILD_STATIC)
    add_library(QtMarkdownRender STATIC)
else()
    add_library(QtMarkdownRender SHARED)
endif ()
target_include_directories(
        QtMarkdownRender PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../editor>
        $<INSTALL_INTERFACE:include>
        $<INSTALL_INTERFACE:include/QtMarkdownRender>
)
target_sources(QtMarkdownRender PRIVATE
        Element.cpp Element.h
        Cell.cpp Cell.h
        Render.cpp Render.h
        Instruction.cpp Instruction.h
        StringUtil.cpp StringUtil.h
        FontMetricsProvider.h
        DefaultFontMetrics.h)
target_compile_definitions(QtMarkdownRender PRIVATE -DQtMarkdownRender_LIBRARY)
target_link_libraries(QtMarkdownRender PUBLIC QtMarkdownParser microtex)
install(
        TARGETS QtMarkdownRender
        EXPORT QtMarkdownRender
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)

markdown_install_headers(QtMarkdownRender PREFIX render
        HEADERS Element.h Cell.h Render.h Instruction.h StringUtil.h mddef.h
        FontMetricsProvider.h
        )
