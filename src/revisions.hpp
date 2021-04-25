#pragma once

#include <cctype>
#include <optional>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "json.hpp"
#include "utils.hpp"

struct RevisionsTable {
    enum type{ VERSION, DATE, DESCRIPTION };
    std::array<int, 3> mapping{-1, -1, -1};
    std::vector<std::string> header;
    std::vector<std::vector<std::string> > data;

    void append_row(const std::vector<Column>& row) {
        data.emplace_back();
        for (const auto& col : row) {
            data.back().push_back(col.data);
        }
    }
};

inline bool infer_mapping(RevisionsTable& table) {
    const auto& row = table.data.front();
    std::vector<bool> taken(row.size(), false);
    int infered = 0;

    size_t digit_count = std::count_if(row[0].begin(), row[0].end(), is_digit);
    if (digit_count < 5 && row[0].size() < 12) {
        table.mapping[RevisionsTable::VERSION] = 0;
        taken[0] = true;
        infered++;
    }

    for (size_t i = 0; i < row.size(); ++i) {
        if (std::count_if(row[i].begin(), row[i].end(), is_digit) >= 5 &&
            row[i].size() < 20) {
            table.mapping[RevisionsTable::DATE] = static_cast<RevisionsTable::type>(i);
            taken[i] = true;
            infered++;
            break;
        }
    }

    for (size_t i = 0; i < row.size(); ++i) {
        if (!taken[i]) {
            if (table.mapping[RevisionsTable::DESCRIPTION] == -1) {
                table.mapping[RevisionsTable::DESCRIPTION] = static_cast<RevisionsTable::type> (i);
                infered++;
            } else if (row[table.mapping[RevisionsTable::DESCRIPTION]].size() <
                       row[i].size()) {
                table.mapping[RevisionsTable::DESCRIPTION] = static_cast<RevisionsTable::type> (i);
            }
        }
    }

    return infered >= 2;
}

inline std::string parse_version(const std::string& version_str) {
    size_t i = version_str.size() - 1;
    size_t max = static_cast<size_t>(-1);
    while (i != max && !is_space(version_str[i])) {
        --i;
    }
    if (i + 1 < version_str.size() && version_str[i + 1] == 'v') {
        i++;
    }
    std::string ver = version_str.substr(i + 1);
    /*if (ver.size() != 1 && !std::any_of(ver.begin(), ver.end(), is_digit)) {
        throw std::runtime_error("Invalid version string!");
    }*/
    return ver;
}

inline std::smatch extract_date_string(const std::string& str) {
    // match date with year at the end
    const static std::regex date_reg_1(
        R"((\d{1,2})\W([a-zA-Z]+|\d{1,2})\W(\d{4}))");
    
    // match date with year at the beginning, dont allow the month to be textual
    // in this case
    const static std::regex date_reg_2(R"((\d{4})\W(\d{1,2})\W(\d{1,2}))");
    
    std::smatch match;

    if (std::regex_search(str, match, date_reg_1) ||
        std::regex_search(str, match, date_reg_2)) {
        return match;
    }

    throw std::runtime_error("Cant extract date from: " + str);
}

inline std::string parse_date_from_match(const std::smatch& match) {
    static std::map<std::string, std::string> months{
        {"january", "01"},  {"jan", "01"},  {"february", "02"}, {"feb", "02"},
        {"march", "03"},     {"mar", "03"},  {"april", "04"},    {"apr", "04"},
        {"may", "05"},       {"may", "05"},  {"june", "06"},     {"jun", "06"},
        {"july", "07"},      {"jul", "07"},  {"august", "08"},   {"aug", "08"},
        {"september", "09"}, {"sept", "09"}, {"october", "10"},  {"oct", "10"},
        {"november", "11"},  {"nov", "11"},  {"december", "12"}, {"dec", "12"},
    };

    std::string day, month, year;

    // find the year
    if (match[3].length() == 4) {
        year = match[3];
        day = match[1];
        month = match[2];
    } else if (match[1].length() == 4) {
        year = match[1];
        day = match[3];
        month = match[2];
    } else {
        std::cout << "SIZE: " << match.size() << "\n";
        std::cout << match[1] << "|" << match[2] << "|" << match[3] << "\n";
        throw std::runtime_error("Date: can't find year " + match[0].str());
    }

    if (is_alpha(month[0])) {
        to_lower(month);
        if (months.count(month) > 0) {
            month = months[month];
        } else {
            throw std::runtime_error("No month found: " + month);
        }
        return year + "-" + month + "-" + day;
    }

    if (std::stoul(month) > 12) {
        std::swap(day, month);
    }

    return year + "-" + month + "-" + day;
}

inline std::string extract_date_from_description(std::string& desc) {
    std::smatch match;
    try {
        match = extract_date_string(desc);
    } catch (std::runtime_error&) {
        return "";
    }

    std::string date = parse_date_from_match(match);

    auto begin = match[0].first;
    auto end = match[0].second;

    if (begin == desc.begin()) {
        desc.erase(begin, end);
        if (!desc.empty() && desc[0] == ':') {
            desc.erase(0, 1);
        }
        ltrim(desc);
    } else if (begin != desc.begin() && end != desc.end()) {
        --begin;
        if (*begin == '(' && *end == ')') {
            desc.erase(begin, ++end);
        }
        desc = join_columns(desc);
    }

    return date;
}

inline std::string parse_date(const std::string& str) {
    std::smatch match = extract_date_string(str);
    return parse_date_from_match(match);
}

inline nlohmann::json read_revision_table(RevisionsTable& table) {
    if (table.header.empty() && !infer_mapping(table)) {
        throw std::runtime_error("No mapping!");
    }

    nlohmann::json j = nlohmann::json::array({});
    for (size_t i = 0; i < table.data.size(); ++i) {
        const auto& row = table.data[i];
        j.push_back(
            nlohmann::json{{"version", ""}, {"date", ""}, {"description", ""}});

        if (table.mapping[RevisionsTable::VERSION] != -1) {
            j.back()["version"] =
                parse_version(row[table.mapping[RevisionsTable::VERSION]]);
        }
        if (table.mapping[RevisionsTable::DATE] != -1) {
            j.back()["date"] =
                parse_date(row[table.mapping[RevisionsTable::DATE]]);
        }
        if (table.mapping[RevisionsTable::DESCRIPTION] != -1) {
            std::string desc = row[table.mapping[RevisionsTable::DESCRIPTION]];
            if (table.mapping[RevisionsTable::DATE] == -1) {
                j.back()["date"] = extract_date_from_description(desc);
            }
            j.back()["description"] = desc;
        }
    }

    if (table.mapping[RevisionsTable::DATE] == -1) {
        bool found = false;
        for (const auto& obj : j) {
            if (obj["date"] == "") {
                found = true;
                break;
            }
        }
        if (found) {
            for (auto& obj : j) {
                obj["date"] = "";
            }
        }
    }

    return j;
}

/**
 * Determines if the provided line contains a header of the revisions table
 *
 * Rules:
 *  - the header has at least 2 columns (since the table must have at least two
 * columns)
 *  - the table typically contains columns 'version', 'date' and 'description'
 * so we have to succesfully find at least two of those
 *  - ??? the header probably contains no numbers ???
 */
inline bool parse_revisions_header(const std::vector<Column>& cols,
                                   RevisionsTable& table) {
    if (cols.size() < 2 || cols.size() > 4) {
        return false;
    }

    table.header.clear();
    table.mapping = {-1, -1, -1};

    const static std::regex ver_reg(R"(version|revision|ver|rev)", std::regex_constants::icase);
    const static std::regex date_reg(R"(date)", std::regex_constants::icase);
    const static std::regex desc_reg(R"(description|change|subject)",
                                     std::regex_constants::icase);
    std::smatch match;

    int matched = 0;

    for (size_t i = 0; i < cols.size(); ++i) {
        const Column& caption = cols[i];
        if (count_words(caption.data) > 4) {
            continue;
        }

        if (table.mapping[RevisionsTable::VERSION] == -1 &&
            std::regex_search(caption.data, match, ver_reg) &&
            (match.length() > 3 || caption.data.size() == 3)) 
        {
            // the last condition makes sure that if we match only "rev" or "ver"
            // then its not part of a bigger word
            matched++;
            table.mapping[RevisionsTable::VERSION] = static_cast<RevisionsTable::type>(i);
        } else if (table.mapping[RevisionsTable::DATE] == -1 &&
                   std::regex_search(caption.data, match, date_reg)) 
        {
            matched++;
            table.mapping[RevisionsTable::DATE] = static_cast<RevisionsTable::type>(i);
        } else if (table.mapping[RevisionsTable::DESCRIPTION] == -1 &&
                   std::regex_search(caption.data, match, desc_reg))
        {
            matched++;
            table.mapping[RevisionsTable::DESCRIPTION] = static_cast<RevisionsTable::type>(i);
        }
    }

    // we didn't match enough :(
    // let's rollback
    if (matched < 2) {
        table.mapping = {-1, -1, -1};
        return false;
    }

    for (const auto& caption : cols) {
        table.header.push_back(caption.data);
    }

    return true;
}

inline bool parse_revisions_table(const std::vector<std::string>& data,
                                  size_t start_pos,
                                  RevisionsTable& table) {
    size_t i = start_pos;
    size_t n = data.size();

    std::vector<Column> last_row;
    size_t max_dist = 3;
    size_t dist = 0;

    table.data.clear();

    while (true) {
        if (dist >= max_dist || i >= n) {
            // we want to return true if we find any rows at all
            if (!last_row.empty()) {
                table.append_row(last_row);
                return true;
            }
            return false;
        }

        auto row = split_line_into_columns(data[i]);

        if (!last_row.empty()) {
            if (row.size() == last_row.size()) {
                // we found a new row
                table.append_row(last_row);
                last_row = row;
                dist = 0;
            } else if (row.size() == 1) {
                // allow overlow in the last column
                if (row.back().start == last_row.back().start) {
                    append_line(last_row.back().data, row.back().data);
                } else {
                    dist++;
                    //std::cout << "skipping: " << data[i] << "\n";
                }
            } else {
                // we dont recognize this as a row
                dist++;
                //std::cout << "skipping: " << data[i] << "\n";
            }
        } else {
            if (row.size() >= 2 && row.size() <= 4) {
                if (!table.header.empty() &&
                    row.size() != table.header.size()) {
                    return false;
                }
                last_row = row;
                dist = 0;
            } else {
                dist++;
                //std::cout << "skipping: " << data[i] << "\n";
            }
        }

        i++;
    }
}

/**
 * Returns true iff the provided line is a title of the section containing the
 * revisions table
 *
 * Rules:
 *  - title should be reasonable short - at most 3 words
 *  - title should contain one of the keywords: 'revision', 'evolution',
 * 'version'
 */
inline bool is_revisions_title(const std::string& line) {
    static const std::regex title_reg(R"(revision|evolution|version)",
                                      std::regex_constants::icase);
    std::smatch match;

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

    return count_words(*title) <= 3 &&
           std::regex_search(*title, match, title_reg) &&
           std::all_of(title->begin(), title->end(), 
                       [](char c){ return is_alpha(c) || is_space(c); });
}

/**
 * Checks if above the line at index <line_index> there is a title of the
 * revisions section.
 */
inline bool has_revisions_title(const std::vector<std::string>& data,
                                size_t line_index) {
    size_t max_search_len = 6;
    size_t search_len = std::min(max_search_len, line_index);
    for (size_t i = 1; i <= search_len; ++i) {
        if (is_revisions_title(data[line_index - i])) {
            return true;
        }
    }
    return false;
}

inline bool parse_revisions_header(const std::string& line,
                                   RevisionsTable& table) {
    std::vector<Column> cols = split_line_into_columns(line);
    return parse_revisions_header(cols, table);
}

inline bool parse_revisions_table_from_title(
    const std::vector<std::string>& data,
    size_t start_pos,
    RevisionsTable& table) {
    size_t max_search_len = 6;
    size_t search_len = std::min(max_search_len, data.size() - start_pos);
    for (size_t i = 0; i < search_len; ++i) {
        size_t idx = start_pos + i;
        std::vector<Column> cols = split_line_into_columns(data[idx]);
        if (cols.size() > 1) {
            if (parse_revisions_header(data[idx], table)) {
                idx++;
            }
            return parse_revisions_table(data, idx, table);
        }
    }

    return false;
}

inline bool try_find_revisions(const std::vector<std::string>& data,
                               size_t start_pos,
                               RevisionsTable& table)
{
    size_t max_search_len = 15;
    size_t search_len = std::min(max_search_len, data.size() - start_pos);

    for (size_t i = 0; i < search_len; ++i) {
        size_t idx = start_pos + i;
        if (is_revisions_title(data[idx])) {
            std::cout << "Found title [" << idx + 1 << "]: " << data[idx]
                      << "\n";
            return parse_revisions_table_from_title(data, idx + 1, table);
        }
        if (parse_revisions_header(data[idx], table)) {
            std::cout << "Found header [" << idx + 1 << "]: " << data[idx]
                      << "\n";
            return parse_revisions_table(data, idx + 1, table);
        }
    }

    return false;
}

inline nlohmann::json parse_revisions(const std::vector<std::string>& data) {
    RevisionsTable table;

    size_t first_pages_count = 7;
    size_t last_pages_count = 7;

    for (size_t i = 0; i < data.size(); ++i) {
        if (is_pagebreak(data[i])) {
            if (first_pages_count-- == 0) {
                break;
            }
            if (try_find_revisions(data, i + 1, table)) {
                try {
                    return read_revision_table(table);
                } catch (std::runtime_error& /*e*/) {}
            }
        }
    }

    for (size_t i = data.size() - 1; i != static_cast<size_t>(-1); --i) {
        if (is_pagebreak(data[i])) {
            if (last_pages_count-- == 0) {
                break;
            }
            if (try_find_revisions(data, i + 1, table)) {
                try {
                    return read_revision_table(table);
                } catch (std::runtime_error&) {}
            }
        }
    }

    return nlohmann::json::array({});
}
