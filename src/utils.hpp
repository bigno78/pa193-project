#pragma once

#include <string>
#include <cctype>
#include <regex>
#include <string_view>

/**
 * Append line at the end of json entry using the rules:
 *  - if the previous line ends in '-' don't add a space
 *  - otherwise add a space
 */
inline void append_line(std::string& s, const std::string& line) {
    if (!s.empty() && s.back() != '-') {
        s += " ";
    }
    s += line;
}

/**
 * Replace all string of spaces longer then 1 by a single space.
 */
inline std::string join_columns(const std::string& line) {
    std::string res;
    size_t i = 0;

    std::string sep = "";
    while(i < line.size()) {
        while(i < line.size() && std::isspace(line[i])) {
            ++i;
        }
        if (i >= line.size()) {
            break;
        }
        res += sep;
        while(i < line.size() && !std::isspace(line[i])) {
            res += line[i];
            ++i;
        }
        if (i < line.size()) {
            sep = line[i];
        }
    }

    return res;
}

// slooooow
inline std::string join_columns_regex(const std::string& line) {
    static std::regex reg{ R"(\s\s\s*)" };
    
    auto it = find_if(
                line.begin(),
                line.end(),
                [](char c){ return !std::isspace(c); } );
    
    std::string res;
    std::regex_replace(
            std::back_inserter(res),
            it, line.end(), reg, " "
    );
    
    return res;
}
