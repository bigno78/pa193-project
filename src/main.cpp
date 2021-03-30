#include <iostream>
#include <fstream>

#include "parser.hpp"
#include "json.hpp"

namespace js = nlohmann;



int main(int argc, char** argv) {
	/* 
	// Example json 
	js::json j;
	j["something"] = 25;
	j["pole"] = js::json::array();
	j["pole"].push_back(5);
	j["pole"].push_back("abs");
	j["ab"] = js::json::array({ 10, 20, 30, "work?", js::json::object( { { "key", "val" } } ) });
	*/
	
	if (argc > 1) {
		// go through all the arguments, assume they are files and parse them
		for (int i = 1; i < argc; ++i) {
			std::ifstream file{ argv[i] };
			if (!file) {
				std::cerr << "Failed to open file: " << argv[i] << "\n";
				continue;
			}
			auto j = parse_document(file);
			std::cout << j.dump(4) << "\n";
		}
	} else { // no arguments -> take input from std::cin
		auto j = parse_document(std::cin);
		std::cout << j.dump(4) << "\n";
	}
}