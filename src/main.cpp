#include <iostream>

#include "parser.hpp"
#include "json.hpp"

namespace js = nlohmann;

int main(int argc, char** argv) {
	js::json j;
	j["something"] = 25;
	j["pole"] = js::json::array();
	j["pole"].push_back(5);
	j["pole"].push_back("abs");
	j["ab"] = js::json::array({ 10, 20, 30, "work?", js::json::object( { { "key", "val" } } ) });

	std::cout << j << "\n";
}