#pragma once

#include <string>
#include <cctype>
#include <regex>
#include <string_view>
#include <sstream>
#include <cassert>


// Wrapper functions since the std functions need unsigned char.
inline bool is_space(char c) { return std::isspace(static_cast<unsigned char>(c)); }
inline bool is_digit(char c) { return std::isdigit(static_cast<unsigned char>(c)); }
inline bool is_alpha(char c) { return std::isalpha(static_cast<unsigned char>(c)); }
inline bool is_alnum(char c) { return std::isalnum(static_cast<unsigned char>(c)); }
inline bool is_upper(char c) { return std::isupper(static_cast<unsigned char>(c)); }

inline char to_lower(char c) { return static_cast<char>(std::tolower(static_cast<unsigned char>(c))); }

inline void to_lower(std::string& str) {
    std::transform(str.begin(), str.end(), str.begin(),
                    [](unsigned char c){ return to_lower(c); });
}


inline bool is_pagebreak(const std::string& line) {
    return std::any_of(line.begin(), line.end(), [](char c){ return c == '\f'; });
}


inline size_t count_words(const std::string& line) {
    if (line.empty()) {
        return 0;
    }
    size_t i = 0;
    size_t count = 0;
    while(true) {
        while(i < line.size() && is_space(line[i])) {
            ++i;
        }
        if (i >= line.size()) return count;
        count++;
        while(i < line.size() && !is_space(line[i])) {
            ++i;
        }
        if (i >= line.size()) return count;
    }
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

struct Column {
    size_t start;
    std::string data;
};

inline std::vector<Column> split_line_into_columns(const std::string& line) {
    std::vector<Column> cols;
    size_t i = 0;
    size_t n = line.size();

    while (true) {
        // search for the beginning of a column - first non-space character
        while (i < n && is_space(line[i])) {
            ++i;
        }

        if (i >= n) {
            return cols;
        }

        Column col = { i, {} };

        // read till the end of the column - two consecutive spaces
        int spaces = 0;
        while (i < n && spaces < 2) {
            if (is_space(line[i]))
                ++spaces;
            else
                spaces = 0;
            ++i;
        }

        if (i >= n) {
            col.data = line.substr(col.start, i - col.start);
            cols.push_back(col);
            return cols;
        }

        col.data = line.substr(col.start, i - 2 - col.start);
        cols.push_back(col);
    }

    assert(false);
}


inline bool is_title(const std::string& line, const std::regex& reg) {
    std::vector<Column> cols = split_line_into_columns(line);
    if (cols.size() > 2 || cols.size() == 0) {
        return false;
    }

    std::string* title = nullptr;
    if (cols.size() == 2) { // there is a section number
        // validate that it really is a section number
        if (count_words(cols[0].data) != 1 || 
            !std::any_of(cols[0].data.begin(), cols[0].data.end(), is_digit))
        {
            return false;
        }
        title = &cols[1].data;
    } else {
        title = &cols[0].data;
    }

    std::smatch match;
    return count_words(*title) <= 3 &&
           std::regex_search(*title, match, reg);
}

