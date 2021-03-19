
add_executable (parser "main.cpp" "parser.cpp")
target_link_libraries(parser project_options)

add_library(parser-lib "parser.cpp")
target_link_libraries(parser project_options)
