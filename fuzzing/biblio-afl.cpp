#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>

#include "../src/bibliography.hpp"

int main(int argc, char **argv) {
    // supposedly makes io faster
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    std::vector<std::string> data;
    std::string line;

    while(std::getline(std::cin, line)) {
        data.emplace_back(std::move(line));
    }

    parse_bibliography(data);
}
