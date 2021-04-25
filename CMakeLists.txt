cmake_minimum_required (VERSION 3.8)

project (parse CXX)

option(SANITIZERS OFF)
option(FUZZ OFF)

if(FUZZ)
    #if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        add_subdirectory("fuzzing/")
    #else()
    #    message(WARNING "Fuzzing can be used only with clang!")
    #endif()
endif()

set(SANITIZER_FLAGS "")
if(SANITIZERS)
    if (FUZZ)
        message(WARNING "Sanitizers cant be enabled globally for fuzzing! Use ASAN_FUZZING=ON.")
    else()
        set(SANITIZER_FLAGS "-fsanitize=address,undefined")
    endif()
endif()

if(MSVC)
    set(WARNINGS /W4)
else()
    set(WARNINGS -Wall -Wextra -Wnull-dereference)
endif()

add_library(project_options INTERFACE)
target_compile_options(project_options INTERFACE ${SANITIZER_FLAGS} ${WARNINGS})
target_link_options(project_options INTERFACE ${SANITIZER_FLAGS})
target_include_directories(project_options INTERFACE "libs/" "src/")
target_compile_features(project_options INTERFACE cxx_std_17)

add_subdirectory ("src/")
add_subdirectory("test/")
