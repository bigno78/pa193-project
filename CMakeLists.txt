cmake_minimum_required (VERSION 3.8)

project (parse CXX)

option(USE_SANITIZERS OFF)
option(USE_FUZZING OFF)

if(USE_FUZZING)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_subdirectory("fuzzing/")
    else()
        message(WARNING "Fuzzing can be used only with clang!")
    endif()
endif()

set(SANITIZERS "")
if(USE_SANITIZERS)
    set(SANITIZERS "-fsanitize=address,undefined")
endif()

if(MSVC)
    set(WARNINGS /W4)
else()
    set(WARNINGS -Wall -Wextra -Wnull-dereference)
endif()

add_library(project_options INTERFACE)
target_compile_options(project_options INTERFACE ${SANITIZERS} ${WARNINGS})
target_link_options(project_options INTERFACE ${SANITIZERS})
target_include_directories(project_options INTERFACE "libs/" "src/")
target_compile_features(project_options INTERFACE cxx_std_17)

add_subdirectory ("src/")
add_subdirectory("test/")
