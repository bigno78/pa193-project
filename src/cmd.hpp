#pragma once

#include <filesystem>
#include <set>
#include <vector>

#include "types.hpp"

struct CmdOptions {
    std::vector<std::filesystem::path> input_files;
    std::filesystem::path output_path;
    std::set<SectionType> sections;
    bool prety_print = false;
    size_t max_width = 80;
};

CmdOptions process_args(int arg_count, char **args);
