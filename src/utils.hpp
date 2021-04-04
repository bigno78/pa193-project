#pragma once

#include <string>
#include <cctype>
#include <regex>
#include <string_view>
#include <sstream>

/**
 * Wrapper function for std::isspace.
 * Used since std::isspace is valid only for unsigned char.
 */
inline bool is_space(char c) {
    return std::isspace(static_cast<unsigned char>(c));
}

/**
 * Wrapper function for std::isdigit.
 * Used since std::isdigit is valid only for unsigned char.
 */
inline bool is_digit(char c) {
    return std::isdigit(static_cast<unsigned char>(c));
}

inline bool is_alpha(char c) {
    return std::isalpha(static_cast<unsigned char>(c));
}

/**
 * Counts the number of words in a line.
 * A word is a continues string of alphanumeric characters without any whitespace.
 */
inline size_t count_words(const std::string& line) {
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
 * Join all columns in the <line> with only one space.
 * Columns are strings of characters separated by two or more spaces.
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

inline bool is_empty_line(const std::string& line) {
    return std::find_if(
        line.begin(),
        line.end(),
        [](char c) { return !is_space(c); }
    ) == line.end();
}

const std::string WHITESPACE = " \n\r\t\f\v";

// Left trim
static inline void ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// Right trim
static inline void rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static inline void trim(std::string& s) {
    ltrim(s);
    rtrim(s);
}
