
#add_executable(fuzz-bib "bibliography-target.cpp")
#target_link_libraries(fuzz-bib PRIVATE project_options -fsanitize=fuzzer,undefined,address)
#target_compile_options(fuzz-bib PRIVATE -fsanitize=fuzzer,undefined,address)

add_executable(fuzz-bib "biblio-afl.cpp")
target_link_libraries(fuzz-bib PRIVATE project_options)
target_compile_options(fuzz-bib PRIVATE -O3 -g)
