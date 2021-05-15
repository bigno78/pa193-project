#include <map>
#include <iostream>

#include "cmd.hpp"

bool starts_with(const std::string& str, const std::string& prefix) {
    if (prefix.size() > str.size()) {
        return false;
    }
    for (size_t i = 0; i < prefix.size(); i++) {
        if (str[i] != prefix[i]) {
            return false;
        }
    }
    return true;
}

std::map<std::string, SectionType> switch_to_section {
    { "-t", SectionType::title }, { "--title", SectionType::title },
    { "-c", SectionType::contents }, { "--contents", SectionType::contents },
    { "-b", SectionType::bibliography }, { "--bibliography", SectionType::bibliography },
    { "-r", SectionType::revisions }, { "--revisions", SectionType::revisions },
    { "-v", SectionType::versions }, { "--versions", SectionType::versions },
};

const char* help = R"(Parser of security certificates

usage: ./parser [ options ] [ file ... ]

Options:
    -o <path>           - if no files are provided the name of the output file, otherwise 
                          the directory where the output files should be stored
    -p, --pprint        - pretty print the parsed sections to the standard ouput
    -w <number>         - the maximum width for pretty printing, by defaul 80
    -t, --title         - parse only the title
    -r, --revisions     - parse only revisions
    -b, --bibliography  - parse only bibliography
    -c, --contents      - parse only table of contents
    -v, --versions      - parse only versions
    -h, --help          - view this help message

)";

void print_usage() {
	std::cerr << help;
}

CmdOptions process_args(int arg_count, char **args) {
    CmdOptions opts;
    for (int i = 1; i < arg_count; ++i) {
        std::string arg = args[i];
        if (starts_with(arg, "-")) {
            if (arg == "-h" || arg == "--help") {
                print_usage();
                std::exit(0);
            } else if (arg == "-o") {
                if (i + 1 >= arg_count) {
                    std::cerr << "-o requires an argument\n";
                    std::exit(1);    
                }
                opts.output_path = args[++i];
            } else if (arg == "-w") {
                if (i + 1 >= arg_count) {
                    std::cerr << "-w requires an argument\n";
                    std::exit(1);    
                }
                int w;
                try {
                    w = std::stoi(args[++i]);
                } catch (std::out_of_range&) {
                    std::cerr << "Outside of range of int\n";
                    std::exit(1);
                } catch (...) {
                    std::cerr << "Invalid width\n";
                    std::exit(1);    
                }
                if (w <= 0) {
                    std::cerr << "Width must be positive!\n";
                    std::exit(1);
                }
                opts.max_width = w;
            } else if (arg == "-p" || arg == "--pprint") {
                opts.prety_print = true;
            } else if (switch_to_section.count(arg) > 0) {
                opts.sections.insert(switch_to_section[arg]);
            } else {
                std::cerr << "Unknown switch: " << arg << "\n";
                std::exit(1);
            }
        } else {
            opts.input_files.push_back(arg);
        }
    }

    if (opts.sections.empty()) {
        opts.sections.insert( 
            { SectionType::title, SectionType::versions, 
              SectionType::revisions, SectionType::contents,
              SectionType::bibliography } 
        );
    }

    if (opts.input_files.empty() && opts.output_path.empty()) {
        opts.output_path = "out.json";
    }

    return opts;
}
