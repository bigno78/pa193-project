#pragma once

#include <algorithm>
#include <cctype>
#include <codecvt>
#include <iostream>
#include <locale>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <vector>

#include "json.hpp"
#include "utils.hpp"

inline bool is_entry(const std::string& line) {
    // regular expressions are slow so first do a quick test
    // if it even makes sense to use one
    // for most lines this should fail fast
    size_t i = 0;
    while (i < line.size() && line[i] != '[') {
        if (!is_space(line[i]))
            return false;
        ++i;
    }

    static const std::regex reg(R"(\s*\[.+?\].*)");
    std::smatch match;
    return std::regex_match(line, match, reg);
}

/**
 * Parses the first line of a bibliography entry.
 * Returns the key (the stuff in []) and the start of the citation following it.
 */
inline std::pair<std::string, std::string> parse_entry(const std::string& line) {
    static std::regex reg(R"(\s*(\[.+?\])(.*))");
    std::smatch res;
    if (!std::regex_match(line, res, reg)) {
        throw std::runtime_error("Something bad happened");
    }
    return {res[1], res[2]};
}

inline size_t count_columns(const std::string& line) {
    return split_line_into_columns(line).size();
}

// is there a title above this line?
inline bool has_title(const std::vector<std::string>& data, size_t line_no) {
    static const std::regex title_reg(R"(reference|bibliography)",
                                 std::regex_constants::icase);

    size_t max_search_len = 7;
    size_t search_len = std::min(line_no, max_search_len);
    for (size_t offset = 1; offset <= search_len; ++offset) {
        size_t i = line_no - offset;
        if (is_title(data[i], title_reg)) {
            return true;
        }
    }

    return false;
}


/**
 * Find the place in the document where the bibliography is.
 *
 * It searches for lines starting with text in square brackets
 * and groups all such lines close to each other into one candidate
 * bibliography. Then the candidate with most lines is picked.
 */
inline std::vector<size_t> find_bibliography(const std::vector<std::string>& data) {
    size_t line_no = 0;

    size_t cutoff_distance = 50;
    size_t current_distance = cutoff_distance + 1;
    std::vector<std::vector<size_t>> candidates;
    int last_candidate_with_title = -1;

    for (const auto& line : data) {
        if (is_entry(line)) {
            if (current_distance > cutoff_distance) {  // begin new candidate
                // if there is header above we are sure this is it
                if (has_title(data, line_no)) {
                    last_candidate_with_title = candidates.size();
                }
                candidates.emplace_back();
            }
            candidates.back().push_back(line_no);
            current_distance = 0;
        }

        current_distance++;
        line_no++;
    }

    if (last_candidate_with_title != -1) {
        return candidates[last_candidate_with_title];
    }

    // we didn't find candidate with header, pick the one with most entries
    auto it = std::max_element(
        candidates.begin(), candidates.end(),
        [](const auto& a, const auto& b) { return a.size() < b.size(); });

    if (it == candidates.end()) {
        return {};
    }

    return *it;
}

inline nlohmann::json parse_candidate(const std::vector<std::string>& data,
                                      const std::vector<size_t>& entries)
{
    assert(!entries.empty());

    nlohmann::json res;
    for (size_t i = 0; i < entries.size(); ++i) {
        size_t j = entries[i];

        auto [key, citation] = parse_entry(data[j++]);
        citation = join_columns(citation);

        while (true) {
            if (j >= data.size()) {
                break;
            }
            if (is_empty_line(data[j])) {
                if (i + 1 < entries.size() && entries[i + 1] - 1 > j) {
                    size_t backtrack_line_no = entries[i + 1] - 1;
                    std::string remainder;
                    std::string temp;
                    while (!is_empty_line(data[backtrack_line_no])) {
                        temp = data[backtrack_line_no];
                        std::swap(remainder, temp);
                        append_line(remainder, temp);
                        backtrack_line_no--;
                    }
                    append_line(citation, join_columns(remainder));
                }
                break;
            }
            if (i + 1 < entries.size() && j >= entries[i + 1]) {
                break;
            }

            append_line(citation, join_columns(data[j]));
            j++;
        }

        res[key] = citation;
    }

    return res;
}

inline nlohmann::json parse_bibliography(const std::vector<std::string>& data) {
    if (data.empty()) {
        return {};
    }

    auto entries = find_bibliography(data);
    if (entries.empty()) {
        return {};
    }

    return parse_candidate(data, entries);
}