find_package(doctest REQUIRED)

enable_testing()
add_executable(test_parser test_parser.cpp)
target_link_libraries(test_parser PRIVATE QtMarkdownParser doctest::doctest)
# fatal error C1189: #error:  The C++ Standard Library forbids macroizing the keyword "private".
# Enable warning C4005 to find the forbidden define.
add_test(NAME test_parser COMMAND test_parser)
add_executable(test_editor test_editor.cpp)
target_link_libraries(test_editor PRIVATE QtMarkdownEditorCore
doctest::doctest
        )
target_include_directories(test_editor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
add_test(NAME test_editor COMMAND test_editor)

add_executable(test_render test_render.cpp)
target_link_libraries(test_render PRIVATE QtMarkdownRender
doctest::doctest
        )
add_test(NAME test_render COMMAND test_render)
set_target_properties(test_render PROPERTIES AUTORCC ON)
set_target_properties(test_render PROPERTIES AUTOMOC ON)
target_include_directories(test_render PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_stringutil test_stringutil.cpp)
target_link_libraries(test_stringutil PRIVATE QtMarkdownRender
doctest::doctest
        )
add_test(NAME test_stringutil COMMAND test_stringutil)


add_executable(test_insert_newline test_insert_newline.cpp)
target_link_libraries(test_insert_newline PRIVATE QtMarkdownEditorCore
doctest::doctest
        )
add_test(NAME test_insert_newline COMMAND test_insert_newline)

