#pragma once

#include <string>
#include <cctype>
#include <vector>
#include <iostream>
#include <algorithm>
#include <set>
#include <regex>
#include <map>
#include <algorithm>

#include <string_view>

#include "json.hpp"
#include "utils.hpp"
#include <locale>
#include <codecvt>

// is this line the begining of bibliography entry - does it start with '['?
inline bool is_entry(const std::string& line) {
    size_t i = 0;
    while (i < line.size() && line[i] != '[') {
        if (!is_space(line[i])) {
            return false;
        }
        ++i;
    }
    
    ++i;
    if (i >= line.size() || line[i] == ']') {
        return false;
    }

    while(i < line.size()) {
        if (line[i] == ']') {
            return true;
        }
        ++i;
    }

    return false;
}

inline bool is_empty_line(const std::string& line) {
    return std::find_if(
                    line.begin(),
                    line.end(),
                    [](char c){ return !is_space(c); }
           ) == line.end();
}

/**
 * Parses the first line of a bibliography entry.
 * Returns the key (the stuff in []) and the start of the citation following it.
 */
inline std::pair<std::string, std::string> parse_entry(const std::string& line) {
    static std::regex reg(R"(\s*(\[.+?\])\s*(.*))");
    std::smatch res;
    if (!std::regex_match(line, res, reg)) {
        throw std::runtime_error("Something bad happened");
    }
    return { res[1], res[2] };
}


inline bool has_title(const std::vector<std::string>& data, size_t line_no) {
    static std::regex header_reg(R"(reference|bibliography)", std::regex_constants::icase);
    std::smatch match;
    
    size_t max_dist = std::min(line_no, (size_t) 5);
    for (size_t offset = 1; offset <= max_dist; ++offset) {
        size_t i = line_no - offset;
        if (std::regex_search(data[i], match, header_reg) && count_words(data[i]) <= 3) {
            return true;
        }
    }

    return false;
}

/**
 * Find the place in the document where the bibliography is.
 * 
 * It searches for lines starting with text in square brackets
 * and groups all such lines close to each other into one candidate bibliography.
 * Then the candidate with most lines is picked.
 */
inline std::vector<size_t> find_bibliography(const std::vector<std::string>& data) {
    size_t line_no = 0;
    
    size_t cutoff_distance = 50;
    size_t current_distance = cutoff_distance + 1;
    std::vector< std::vector<size_t> > candidates; 
    bool i_am_sure = false;

    for (const auto& line : data) {
        // we finished a candidate and we are sure its him
        if (current_distance > cutoff_distance && i_am_sure) {
            return candidates.back();
        }

        if (is_entry(line)) {
            if (current_distance > cutoff_distance) { // begin new candidate
                // if there is header above we are sure this is it
                i_am_sure = has_title(data, line_no);
                candidates.emplace_back();
            }
            candidates.back().push_back(line_no);
            current_distance = 0;
        }

        current_distance++;
        line_no++;
    }

    if (i_am_sure) {
        return candidates.back();
    }

    // we didn't find candidate with header, pick the one with most entries
    auto it = std::max_element(
                candidates.begin(),
                candidates.end(),
                [](const auto& a, const auto& b) {return a.size() < b.size();} );

    if (it == candidates.end()) {
        //std::wcout << " COULD NOT FIND ANY CANDIDATE FOR BIBLIOGRAPHY!!!!!\n";
        return {};
    }

    return *it;
}

inline nlohmann::json parse_bibliography(const std::vector<std::string>& data) {
    auto entries = find_bibliography(data);
    if (entries.empty()) {
        return {};
    }

    nlohmann::json res;
    for (size_t i = 0; i < entries.size(); ++i) {
        size_t j = entries[i];
        auto [ key, citation ] = parse_entry(data[j]);
        j++;
        while(true) {
            // decide if we should end this citation
            if (i+1 < entries.size()) { // there is one more entry
                if (j >= entries[i+1] || is_empty_line(data[j])) break;
            } else { // we are parsing last entry
                if (j >= data.size() || is_empty_line(data[j])) break;
            }
            append_line(citation, join_columns(data[j]));
            j++;
        }
        // convert to std::string
        //using convert_type = std::codecvt_utf8<wchar_t>;
        //std::wstring_convert<convert_type, wchar_t> converter;
        //std::string string_key = converter.to_bytes(key);
        //std::string string_cit = converter.to_bytes(citation);
        res[key] = citation;
    }

    return res;
}