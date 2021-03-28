#pragma once

#include <string>
#include <cctype>
#include <vector>
#include <iostream>
#include <algorithm>
#include <set>
#include <regex>
#include <map>

#include <string_view>

#include "json.hpp"
#include "utils.hpp"

// is this line the begining of bibliography entry - does it start with '['?
inline bool is_entry(const std::string& line) {
    size_t i = 0;
    while (i < line.size()) {
        if (line[i] == '[') {
            return true;
        }
        if (!isspace(line[i])) {
            break;
        }
        ++i;
    }
    return false;
}

inline bool is_empty_line(const std::string& line) {
    return std::find_if(
                    line.begin(),
                    line.end(),
                    [](char c){ return !std::isspace(c); }
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

/*
inline void add_refs(const std::string& s, std::set<std::string>& ref_set) {
    //static std::regex leading_ref_reg(R"(\s*\[(.+?)\].*)");
    static std::regex ref_reg(R"(\[(.+?)\])");

    auto start = std::sregex_iterator(s.begin(), s.end(), ref_reg);
    auto end = std::sregex_iterator{};

    for (auto it = start; it != end; ++it) {
        auto match = *it;
        ref_set.insert(match.str());
    }
}

inline std::string get_ref(const std::string& s) {
    static std::regex leading_ref_reg(R"(\s*\[(.+?)\].*)");
    
    std::smatch res;
    if (std::regex_match(s, res, leading_ref_reg)) {
        return res[1];
    }
    return {};
}
*/

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
    
    for (const auto& line : data) {
        if (is_entry(line)) {
            if (current_distance > cutoff_distance) {
                candidates.emplace_back();
            }
            candidates.back().push_back(line_no);
            current_distance = 0;
        }
        current_distance++;
        line_no++;
    }

    auto it = std::max_element(
                candidates.begin(),
                candidates.end(),
                [](const auto& a, const auto& b) {return a.size() < b.size();} );

    if (it == candidates.end()) {
        std::cout << " COULD NOT FIND ANY CANDIDATE FOR BIBLIOGRAPHY!!!!!\n";
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
        res[key] = citation;
    }

    return res;
}