add_library(QtMarkdownParser SHARED
        Document.cpp Document.h
        Token.cpp Token.h
        Parser.cpp Parser.h
        Visitor.cpp Visitor.h
        PieceTable.cpp PieceTable.h
        Text.cpp Text.h)
target_compile_definitions(QtMarkdownParser PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownParser PUBLIC Qt${QT_VERSION_MAJOR}::Core magic_enum::magic_enum)
target_include_directories(QtMarkdownParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)