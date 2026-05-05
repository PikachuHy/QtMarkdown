add_library(QtMarkdownPlatform STATIC)
target_include_directories(QtMarkdownPlatform PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../..>
)
target_sources(QtMarkdownPlatform PRIVATE
    QtAdapters.h
    QtImageProvider.h
    QtFontMetricsProvider.h
    QtLatexPlatform.h
)
target_link_libraries(QtMarkdownPlatform PUBLIC
    Qt${QT_VERSION_MAJOR}::Gui
    microtex-qt
    microtex
)

install(
    TARGETS QtMarkdownPlatform
    EXPORT QtMarkdownPlatform
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

markdown_install_headers(QtMarkdownPlatform PREFIX platform/qt
    HEADERS QtAdapters.h QtImageProvider.h QtFontMetricsProvider.h QtLatexPlatform.h
)
