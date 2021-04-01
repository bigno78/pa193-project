#pragma once

#include <string>
#include <cctype>
#include <vector>
#include <regex>
#include <sstream>

#include "utils.hpp"
#include "json.hpp"

class RevisionsParser {
    const std::vector<std::string>& data;
    
public:
    RevisionsParser(const std::vector<std::string>& data) : data(data) {}

    nlohmann::json parse() {
        static std::regex title_reg(R"(revision|evolution)", std::regex_constants::icase);
        size_t limit = 200;
        std::smatch match;
        for (size_t i = 0; i < limit; ++i) {
            if (std::regex_search(data[i], match, title_reg) && count_words(data[i]) <= 3) {
                for (int j = 0; j < 5; ++j) {
                    std::cout << "Line " << i + 1 << ": " << data[i] << "\n";
                    ++i;
                }
                std::wcout << "-----------------------------------------------\n";
            }
        }
        return {};
    }

};
