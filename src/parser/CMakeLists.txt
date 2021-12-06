option(BUILD_WITH_BOOST "build with boost" OFF)
if (BUILD_WITH_BOOST)
    add_definitions(-D_GNU_SOURCE=1)
    find_package(Boost REQUIRED)
    add_definitions(-DBUILD_WITH_BOOST)
endif ()
if (BUILD_STATIC)
    add_library(QtMarkdownParser STATIC)
else()
    add_library(QtMarkdownParser SHARED)
endif ()
target_sources(QtMarkdownParser PRIVATE
        ../debug.cpp ../debug.h
        Document.cpp Document.h
        Token.cpp Token.h
        Parser.cpp Parser.h
        Visitor.cpp Visitor.h
        PieceTable.cpp PieceTable.h
        Text.cpp Text.h)
target_compile_definitions(QtMarkdownParser PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownParser PUBLIC Qt${QT_VERSION_MAJOR}::Core magic_enum::magic_enum)
if (BUILD_WITH_BOOST)
    target_link_libraries(QtMarkdownParser PUBLIC Boost::boost)
endif ()
target_include_directories(QtMarkdownParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)