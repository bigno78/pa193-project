#pragma once

#include <iostream>
#include <string>
#include <vector>

inline std::vector<std::string> load_data() {
    // supposedly makes io faster
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(NULL);

    std::vector<std::string> data;
    std::string line;

    while(std::getline(std::cin, line)) {
        data.emplace_back(std::move(line));
    }

    return data;
}
