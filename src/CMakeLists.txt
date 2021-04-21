
add_library(parser-lib "parser.cpp" "versions.cpp")
target_link_libraries(parser-lib project_options)

add_executable (parser "main.cpp")
target_link_libraries(parser project_options parser-lib)


