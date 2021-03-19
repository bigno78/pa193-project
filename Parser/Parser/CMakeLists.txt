# CMakeList.txt : CMake project for Parser, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (Parser "parser.cpp" "parser.h")
add_executable (testerin "tests.cpp")

# TODO: Add tests and install targets if needed.
