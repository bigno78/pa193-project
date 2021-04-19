#pragma once

#include <string>
#include <cctype>
#include <vector>
#include <regex>
#include <sstream>
#include <optional>

#include "utils.hpp"
#include "json.hpp"

struct RevisionsTable {
    enum { VERSION, DATE, DESCRIPTION };
    std::array<int, 3> mapping { -1, -1, -1 };
    std::vector< std::string > header;
    std::vector< std::vector< std::string > > data;

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
    if (digit_count < 5 && row[0].size() < 10) {
        table.mapping[RevisionsTable::VERSION] = 0;
        taken[0] = true;
        infered++;
    }

    for (size_t i = 0; i < row.size(); ++i) {
        if (std::count_if(row[i].begin(), row[i].end(), is_digit) >= 5) {
            table.mapping[RevisionsTable::DATE] = i;
            taken[i] = true;
            infered++;
            break;
        }
    }

    for (size_t i = 0; i < row.size(); ++i) {
        if (!taken[i]) {
            if (table.mapping[RevisionsTable::DESCRIPTION] == -1) {
                table.mapping[RevisionsTable::DESCRIPTION] = i;
                infered++;
            } else if (row[table.mapping[RevisionsTable::DESCRIPTION]].size() < row[i].size()) {
                table.mapping[RevisionsTable::DESCRIPTION] = i;
            }
        }
    }

    return infered >= 2;
}


inline nlohmann::json read_revision_table(RevisionsTable& table) {
    if (table.header.empty() && !infer_mapping(table)) {
        throw std::runtime_error("No mapping!");
    }

    nlohmann::json j = nlohmann::json::array({});
    for (size_t i = 0; i < table.data.size(); ++i) {
        const auto& row = table.data[i];
        j.push_back( nlohmann::json{} );

        if (table.mapping[RevisionsTable::VERSION] != -1) {
            j.back()["version"] = row[table.mapping[RevisionsTable::VERSION]];
        }
        if (table.mapping[RevisionsTable::DATE] != -1) {
            j.back()["date"] = row[table.mapping[RevisionsTable::DATE]];
        }
        if (table.mapping[RevisionsTable::DESCRIPTION] != -1) {
            j.back()["decription"] = row[table.mapping[RevisionsTable::DESCRIPTION]];
        }
    }

    return j;
}
/*
inline std::optional<std::string> parse_date(const std::string& date) {
    static std::regex date_reg(R"((\d{1,2})\W([a-zA-Z]+|\d{1,2})\W(\d{4}))");
    std::smatch match;

    if (!std::regex_match(date, match, date_reg)) {
        return std::nullopt;
    }

    static std::map<std::string, std::string> months {
        { "janunary", "01" }, { "jan", "01" },
        { "february", "02" }, { "feb", "02" },
        { "march", "03" }, { "mar", "03" },
        { "april", "04" }, { "apr", "04" },
        { "may", "05" }, { "may", "05" },
        { "june", "06" }, { "jun", "06" },
        { "july", "07" }, { "jul", "07" },
        { "august", "08" }, { "aug", "08" },
        { "september", "09" }, { "sept", "09" },
        { "october", "10" }, { "oct", "10" },
        { "november", "11" }, { "nov", "11" },
        { "december", "12" }, { "dec", "12" },
    };

    std::string day = match[1], month = match[2], year = match[3];
    if (is_alpha(month[0]) && months.count(month) > 0) {
        month = months[month];
    } else if (std::stoul(month) > 12) {
        std::swap(day, month); 
    }

    return day + "-" + month + "-" + year;
}*/

/**
 * Determines if the provided line contains a header of the revisions table
 * 
 * Rules:
 *  - the header has at least 2 columns (since the table must have at least two columns)
 *  - the table typically contains columns 'version', 'date' and 'description' so
 *    we have to succesfully find at least two of those
 *  - ??? the header probably contains no numbers ???
 */
inline bool parse_revisions_header(const std::vector<Column>& cols, RevisionsTable& table) {
    if (cols.size() < 2) {
        return false;
    }

    table.header.clear();
    table.mapping = { -1, -1, -1 };

    const static std::regex ver_reg(R"(ver|rev)", std::regex_constants::icase);
    const static std::regex date_reg(R"(date)", std::regex_constants::icase);
    const static std::regex desc_reg(R"(description|changes|subject)", std::regex_constants::icase);
    std::smatch match;

    int matched = 0;

    for (size_t i = 0; i < cols.size(); ++i) {
        const Column& caption = cols[i];
        if (count_words(caption.data) > 4) {
            continue;
        }

        if (table.mapping[RevisionsTable::VERSION] == -1 && 
            std::regex_search(caption.data, match, ver_reg))
        {
            matched++;
            table.mapping[RevisionsTable::VERSION] = i;
        } else if (table.mapping[RevisionsTable::DATE] == -1 &&
                    std::regex_search(caption.data, match, date_reg))
        {
            matched++;
            table.mapping[RevisionsTable::DATE] = i;
        } else if (table.mapping[RevisionsTable::DESCRIPTION] == -1 &&
                    std::regex_search(caption.data, match, desc_reg)) {
            matched++;
            table.mapping[RevisionsTable::DESCRIPTION] = i;
        }
    }

    for (const auto& caption : cols) {
        table.header.push_back(caption.data);
    }

    return matched >= 2;
}

inline bool parse_revisions_table(const std::vector<std::string>& data,
                                  size_t start_pos,
                                  RevisionsTable& table) 
{
    size_t i = start_pos;
    size_t n = data.size();

    std::vector<Column> last_row;
    size_t max_dist = 3;
    size_t dist = 0;

    table.data.clear();

    while(true) {
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
                    std::cout << "skipping: " << data[i] << "\n";
                }
            } else {
                // we dont recognize this as a row
                std::cout << "skipping: " << data[i] << "\n";
            }
        } else {
            if (row.size() >= 2) {
                if (!table.header.empty() && row.size() != table.header.size()) {
                    return false;
                }
                last_row = row;
                dist = 0;
            } else {
                std::cout << "skipping: " << data[i] << "\n";
            }
        }
        
        dist++;
        i++;
    }
}

/**
 * Returns true iff the provided line is a title of the section containing the revisions table
 * 
 * Rules:
 *  - title should be reasonable short - at most 3 words
 *  - title should contain one of the keywords: 'revision', 'evolution', 'version'
 */
inline bool is_revisions_title(const std::string& line) {
    static const std::regex title_reg(R"(revision|evolution|version)", std::regex_constants::icase);
    std::smatch match;
    return count_words(line) <= 3 && std::regex_search(line, match, title_reg);
}

/**
 * Checks if above the line at index <line_index> there is a title of the revisions section.
 */
inline bool has_revisions_title(const std::vector<std::string>& data, size_t line_index) {
    size_t max_search_len = 6;
    size_t search_len = std::min(max_search_len, line_index);
    for (size_t i = 1; i <= search_len; ++i) {
        if (is_revisions_title(data[line_index - i])) {
            return true;
        }
    }
    return false;
}

inline bool try_parse_revisions(const std::vector<std::string>& data,
                                size_t start_pos,
                                RevisionsTable& table)
{
    std::vector<Column> cols = split_line_into_columns(data[start_pos]);

    // The first line of the table must have at least one column
    if (cols.size() < 2) {
        return false;
    }

    bool header_ok = parse_revisions_header(cols, table);
    if (header_ok || has_revisions_title(data, start_pos)) {
        std::cout << "found on [" << start_pos + 1 << "] with " << (header_ok ? "header" : "title") << "\n";
        std::cout << data[start_pos] << "\n";
        if (header_ok) {
            start_pos++;
        }
        return parse_revisions_table(data, start_pos, table);
    }

    return false;
}   

inline nlohmann::json parse_revisions(const std::vector<std::string>& data) {
    size_t search_depth = data.size();
    RevisionsTable table;

    for (size_t i = 0; i < search_depth; ++i) {
        if (try_parse_revisions(data, i, table)) {
            try {
                return read_revision_table(table);
            } catch (std::runtime_error& e) {}
        }
    }

    return {};
}
