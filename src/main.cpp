#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <filesystem>

#include "parser.hpp"
#include "json.hpp"

namespace fs = std::filesystem;
namespace js = nlohmann;


fs::path get_output_filename(const fs::path& out_dir, const fs::path& in_file) {
	fs::path out_file = out_dir;
	out_file = out_file / in_file.filename();
	out_file.replace_extension("json");
	return out_file;
}

const std::string help = R"(Parser of security certificates

usage: %s [ -o <path> ] [ file ... ]

Options:
    -o path    - if no files are provided the name of the output file, otherwise 
                 the directory where the output files should be stored

)";

void print_usage(const char* executable_name) {
	printf(help.c_str(), executable_name);
}

int main(int argc, char** argv) {
	std::vector<fs::path> input_files;
	fs::path out_path;

	for (int i = 1; i < argc; ++i) {
		std::string arg { argv[i] };
		if (arg == "-o") {
			if (i + 1 >= argc) {
				std::cerr << "Error: -d requires an argument\n";
				std::exit(1);
			}
			++i;
			out_path = argv[i];
		} else if (arg == "-h" || arg == "--help") {
			print_usage(argv[0]);
			return 0;
		} else {
			input_files.push_back(argv[i]);
		}
	}

	if (input_files.empty() && out_path.empty()) {
		out_path = "out.json";
	}
	
	if (input_files.size() > 0) {
		for (const auto& filename : input_files) {
			std::ifstream in_file{ filename };
			if (!in_file) {
				std::cerr << "Failed to open file: " << filename << "\n";
				return 1;
			}
			fs::path out_filename = get_output_filename(out_path, filename); 
			std::ofstream out_file{ out_filename };
			if (!out_file) {
				std::cerr << "Failed to create file: " << out_filename << "\n";
				return 1;
			}
			auto j = parse_document(in_file);
			out_file << std::setw(4) << j << std::endl;
			//std::cout << j.dump(4) << "\n";
		}
	} else { // no input files -> take input from std::cin
		auto j = parse_document(std::cin);
		std::ofstream out_file{ out_path };
		if (!out_file) {
			std::cerr << "Failed to create file: " << out_path << "\n";
			return 1;
		} 
		out_file << std::setw(4) << j << std::endl;
		//std::cout << j.dump(4) << "\n";
	}
}