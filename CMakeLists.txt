cmake_minimum_required (VERSION 3.8)

project (parse CXX)

if(MSVC)
  set(WARNINGS /W4)
else()
  set(WARNINGS -Wall -Wextra -Wnull-dereference)
endif()

add_library(project_options INTERFACE)
target_compile_options(project_options INTERFACE -std=c++11 ${WARNINGS})
target_include_directories(project_options INTERFACE "libs/" "src/")

add_subdirectory ("src/")
add_subdirectory("test/")
