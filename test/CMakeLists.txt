
add_executable (parser-test "test-main.cpp" "util-test.cpp")
target_link_libraries(parser-test project_options parser-lib)
