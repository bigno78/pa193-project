#pragma once

#include <string>
#include <cctype>
#include <regex>
#include <string_view>
#include <cwctype>

/**
 * Append line at the end of json entry using the rules:
 *  - if the previous line ends in '-' don't add a space
 *  - otherwise add a space
 */
inline void append_line(std::wstring& s, const std::wstring& line) {
    if (!s.empty() && s.back() != '-') {
        s += L" ";
    }
    s += line;
}

/**
 * Replace all string of spaces longer then 1 by a single space.
 */
inline std::wstring join_columns(const std::wstring& line) {
    std::wstring res;
    size_t i = 0;

    std::wstring sep = L"";
    while(i < line.size()) {
        while(i < line.size() && std::iswspace(line[i])) {
            ++i;
        }
        if (i >= line.size()) {
            break;
        }
        res += sep;
        while(i < line.size() && !std::iswspace(line[i])) {
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
inline std::wstring join_columns_regex(const std::wstring& line) {
    static std::wregex reg{ LR"(\s\s\s*)" };
    
    auto it = find_if(
                line.begin(),
                line.end(),
                [](char c){ return !std::iswspace(c); } );
    
    std::wstring res;
    std::regex_replace(
            std::back_inserter(res),
            it, line.end(), reg, L" "
    );
    
    return res;
}
