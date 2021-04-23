
#add_executable(fuzz-bib "bibliography-target.cpp")
#target_link_libraries(fuzz-bib PRIVATE project_options -fsanitize=fuzzer,undefined,address)
#target_compile_options(fuzz-bib PRIVATE -fsanitize=fuzzer,undefined,address)

set(FLAGS "")

if(ASAN_FUZZ)
    # fuzz only 32 binaries with asan
    message("Asan fuzzing detected. Binary will be compiled in 32bit mode.")
    set(FLAGS "-fsanitize=address" "-m32")
endif()

add_executable(fuzz-bib "biblio-afl.cpp")

target_link_libraries(fuzz-bib PRIVATE project_options)

target_compile_options(fuzz-bib PRIVATE ${FLAGS})
target_link_options(fuzz-bib PRIVATE ${FLAGS})

# copy the test cases to the build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/testcases DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
