#pragma once

#include <string>
#include <cctype>
#include <vector>
#include <regex>
#include <sstream>
#include <optional>
#include <iostream>

#include "utils.hpp"
#include "json.hpp"

namespace js = nlohmann;

inline js::json parse_toc(const std::vector<std::string>& data) {
    size_t start = 0;
    for (size_t i = 0; i < 500; ++i) {
        if (std::any_of(data[i].begin(), data[i].end(), [](char c){ return c == '\f'; })) {
            for (size_t j = start; j < i; j++) {
                std::cout << data[j] << "\n";
            }
            std::cout << "---------------------------------------------------------------------------------\n";
            start = i + 1;
        }
    }
    return {};
}
