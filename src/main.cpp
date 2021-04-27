#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <iomanip>
#include <filesystem>
#include <stdexcept>

#include "parser.hpp"
#include "json.hpp"
#include "cmd.hpp"
#include "pprint.hpp"

namespace fs = std::filesystem;
namespace js = nlohmann;

fs::path get_output_filename(const fs::path& out_dir, const fs::path& in_file) {
	fs::path out_file = out_dir;
	out_file = out_file / in_file.filename();
	out_file.replace_extension("json");
	return out_file;
}

int main(int argc, char** argv) {
	CmdOptions opts = process_args(argc, argv);
	
	
	if (opts.input_files.size() > 0) {
		for (const auto& filename : opts.input_files) {
			std::ifstream in_file{ filename };
			if (!in_file) {
				std::cerr << "Failed to open file: " << filename << "\n";
				return 1;
			}
			fs::path out_filename = get_output_filename(opts.output_path, filename); 
			std::ofstream out_file{ out_filename };
			if (!out_file) {
				std::cerr << "Failed to create file: " << out_filename << "\n";
				return 1;
			}
			auto j = parse_document(in_file, opts.sections);
			try {
				out_file << std::setw(4) << j << std::endl;
			} catch (std::exception& /*unused*/) {
				std::cout << "An exception occured when writing data!\n";
				std::cout << "The input data was most likely non-UTF8 chracters.\n";
				return 1;
			}
			if (opts.prety_print) {
				pprint(j, opts.sections, opts.max_width);
			}
		}
	} else { // no input files -> take input from std::cin
		auto j = parse_document(std::cin, opts.sections);
		std::ofstream out_file{ opts.output_path };
		if (!out_file) {
			std::cerr << "Failed to create file: " << opts.output_path << "\n";
			return 1;
		} 
		try {
			out_file << std::setw(4) << j << std::endl;
		} catch (std::exception& /*unused*/) {
			std::cout << "An exception occured when writing data!\n";
			std::cout << "The input data was most likely contains non-UTF8 chracters.\n";
			return 1;
		}
		if (opts.prety_print) {
				pprint(j, opts.sections, opts.max_width);
			}
	}
}