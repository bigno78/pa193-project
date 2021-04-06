#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>


#include "parser.hpp"
#include "json.hpp"

namespace js = nlohmann;

std::string get_filename_from_path(const std::string& path) {
	auto n = path.rfind('/');
	if (n != std::string::npos) {
		return path.substr(n + 1, path.size() - n);
	}
	n = path.rfind('\\');
	if (n != std::string::npos) {
		return path.substr(n + 1, path.size() - n);
	}
	return path;
}

void replace_suffix(std::string& file, const std::string& new_suffix) {
	auto n = file.rfind('.');
	if (n != std::string::npos) {
		file.erase(n);
	}
	file.append(new_suffix);
}

std::string get_output_filename(const std::string out_path, const std::string& in_filename) {
	std::string path = out_path.empty() ? "" : out_path + "/";
	auto file = get_filename_from_path(in_filename);
	replace_suffix(file, ".json");
	return path + file;
}

int main(int argc, char** argv) {
	std::vector<std::string> input_files;
	std::string out_path = "";

	for (int i = 1; i < argc; ++i) {
		std::string arg { argv[i] };
		if (arg == "-d") {
			if (i + 1 >= argc) {
				std::cerr << "Error: -d requires an argument\n";
				std::exit(1);
			}
			++i;
			out_path = argv[i];
		} else {
			input_files.push_back(argv[i]);
		}
	}
	
	if (input_files.size() > 0) {
		for (const auto& filename : input_files) {
			std::ifstream in_file{ filename };
			if (!in_file) {
				std::cerr << "Failed to open file: " << filename << "\n";
				continue;
			}
			std::string out_filename =  get_output_filename(out_path, filename); 
			std::ofstream out_file{ out_filename };
			if (!out_file) {
				std::cerr << "Failed to create file: " << out_filename << "\n";
				continue;
			}
			auto j = parse_document(in_file);
			out_file << std::setw(4) << j << std::endl;
			//std::cout << j.dump(4) << "\n";
		}
	} else { // no input files -> take input from std::cin
		//auto j = parse_document(std::cin);
		//std::cout << j.dump(4) << "\n";
	}
}