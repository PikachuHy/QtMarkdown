enable_testing()
add_executable(test_parser test_parser.cpp)
target_link_libraries(test_parser PRIVATE QtMarkdownParser doctest::doctest)
# fatal error C1189: #error:  The C++ Standard Library forbids macroizing the keyword "private".
# Enable warning C4005 to find the forbidden define.
target_compile_definitions(test_parser PRIVATE -D_XKEYCHECK_H)
add_test(NAME test_parser COMMAND test_parser)
find_package(Qt6 COMPONENTS Test REQUIRED)

add_executable(test_editor test_editor.cpp)
target_link_libraries(test_editor PRIVATE QtMarkdownEditorCore
doctest::doctest
        )
add_test(NAME test_editor COMMAND test_editor)
target_compile_definitions(test_editor PRIVATE -D_XKEYCHECK_H)
set_target_properties(test_editor PROPERTIES AUTORCC ON)
set_target_properties(test_editor PROPERTIES AUTOMOC ON)

add_executable(test_render test_render.cpp)
target_link_libraries(test_render PRIVATE QtMarkdownRender
doctest::doctest
        )
add_test(NAME test_render COMMAND test_render)
target_compile_definitions(test_render PRIVATE -D_XKEYCHECK_H)
set_target_properties(test_render PROPERTIES AUTORCC ON)
set_target_properties(test_render PROPERTIES AUTOMOC ON)

add_executable(test_stringutil test_stringutil.cpp)
target_compile_definitions(test_stringutil PRIVATE -D_XKEYCHECK_H)
target_link_libraries(test_stringutil PRIVATE QtMarkdownRender
doctest::doctest
        )
add_test(NAME test_stringutil COMMAND test_stringutil)


add_executable(test_insert_newline test_insert_newline.cpp)
target_compile_definitions(test_insert_newline PRIVATE -D_XKEYCHECK_H)
target_link_libraries(test_insert_newline PRIVATE QtMarkdownEditorCore
doctest::doctest
        )
add_test(NAME test_insert_newline COMMAND test_insert_newline)

