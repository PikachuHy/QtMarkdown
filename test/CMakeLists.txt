enable_testing()
find_package(GTest CONFIG REQUIRED)
add_executable(test_parser test_parser.cpp)
target_link_libraries(test_parser PRIVATE QtMarkdownParser GTest::gtest GTest::gtest_main)
add_test(NAME test_parser COMMAND test_parser)
find_package(Qt6 COMPONENTS Test REQUIRED)

add_executable(test_editor test_editor.cpp)
target_link_libraries(test_editor PRIVATE QtMarkdownEditorCore
        GTest::gtest
        )
add_test(NAME test_editor COMMAND test_editor)
set_target_properties(test_editor PROPERTIES AUTORCC ON)
set_target_properties(test_editor PROPERTIES AUTOMOC ON)

add_executable(test_render test_render.cpp)
target_link_libraries(test_render PRIVATE QtMarkdownRender
        GTest::gtest
        )
add_test(NAME test_render COMMAND test_render)
set_target_properties(test_render PROPERTIES AUTORCC ON)
set_target_properties(test_render PROPERTIES AUTOMOC ON)

add_executable(test_stringutil test_stringutil.cpp)
target_link_libraries(test_stringutil PRIVATE QtMarkdownRender
        GTest::gtest GTest::gtest_main
        )
add_test(NAME test_stringutil COMMAND test_stringutil)