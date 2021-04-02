#pragma once

#include <string>
#include <cctype>
#include <regex>
#include <string_view>
#include <cwctype>
#include <sstream>

//isspace() workaround
inline bool is_space(char c) {
    return std::isspace(static_cast<unsigned char>(c));
}

inline bool is_digit(char c) {
    return std::isdigit(static_cast<unsigned char>(c));
}

size_t count_words(const std::string& line) {
    std::stringstream s(line);
    std::string tok;
    size_t count = 0;
    while (s >> tok) {
        count++;
    }
    return count;
}

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
        while(i < line.size() && is_space(line[i])) {
            ++i;
        }
        if (i >= line.size()) {
            break;
        }
        res += sep;
        while(i < line.size() && !is_space(line[i])) {
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
                [](char c){ return !is_space(c); } );
    
    std::string res;
    std::regex_replace(
            std::back_inserter(res),
            it, line.end(), reg, " "
    );
    
    return res;
}
