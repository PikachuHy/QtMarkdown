option(BUILD_WITH_BOOST "build with boost" OFF)
if (BUILD_WITH_BOOST)
    add_definitions(-D_GNU_SOURCE=1)
    find_package(Boost REQUIRED)
    add_definitions(-DBUILD_WITH_BOOST)
endif ()
if (BUILD_STATIC)
    add_library(QtMarkdownParser OBJECT)
else()
    add_library(QtMarkdownParser SHARED)
endif ()
target_include_directories(
        QtMarkdownParser PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
)
target_sources(QtMarkdownParser PRIVATE
        ../debug.cpp ../debug.h
        Document.cpp Document.h
        Token.cpp Token.h
        Parser.cpp Parser.h
        Visitor.cpp Visitor.h
        PieceTable.cpp PieceTable.h
        Text.cpp Text.h
        Node.h
        ParserDetail.h
        parsers/HeaderParser.cpp
        parsers/CodeBlockParser.cpp
        parsers/CheckboxListParser.cpp
        parsers/UnorderedListParser.cpp
        parsers/OrderedListParser.cpp
        parsers/QuoteBlockParser.cpp
        parsers/LatexBlockParser.cpp
        parsers/ParagraphParser.cpp
        parsers/ImageParser.cpp
        parsers/LinkParser.cpp
        parsers/InlineCodeParser.cpp
        parsers/InlineLatexParser.cpp
        parsers/SemanticTextParser.cpp
        nodes/Header.h
        nodes/Paragraph.h
        nodes/CheckboxList.h
        nodes/UnorderedList.h
        nodes/OrderedList.h
        nodes/QuoteBlock.h
        nodes/ItalicText.h
        nodes/BoldText.h
        nodes/ItalicBoldText.h
        nodes/StrickoutText.h
        nodes/Image.h
        nodes/Link.h
        nodes/CodeBlock.h
        nodes/InlineCode.h
        nodes/InlineLatex.h
        nodes/Hr.h
        nodes/Table.h
        nodes/Lf.h
        nodes/LatexBlock.h LatexBlock.cpp
        nodes/ListNode.h)
target_compile_definitions(QtMarkdownParser PRIVATE -DQtMarkdown_LIBRARY)
target_link_libraries(QtMarkdownParser PUBLIC magic_enum::magic_enum)
if (BUILD_WITH_BOOST)
    target_link_libraries(QtMarkdownParser PUBLIC Boost::boost)
endif ()
target_include_directories(QtMarkdownParser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
install(
        TARGETS QtMarkdownParser
        EXPORT QtMarkdownParser
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
)
markdown_install_headers(QtMarkdownParser PREFIX parser HEADERS
        Document.h Token.h Parser.h Visitor.h PieceTable.h Text.h mddef.h IBufferProvider.h
        Node.h
        nodes/Header.h nodes/Paragraph.h nodes/CheckboxList.h
        nodes/UnorderedList.h nodes/OrderedList.h nodes/QuoteBlock.h
        nodes/ItalicText.h nodes/BoldText.h nodes/ItalicBoldText.h
        nodes/StrickoutText.h nodes/Image.h nodes/Link.h
        nodes/CodeBlock.h nodes/InlineCode.h nodes/InlineLatex.h
        nodes/Hr.h nodes/Table.h nodes/Lf.h nodes/LatexBlock.h
        nodes/ListNode.h
        QtMarkdownParser
        )
