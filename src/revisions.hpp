#pragma once

#include <string>
#include <cctype>
#include <vector>
#include <regex>
#include <sstream>

#include "utils.hpp"
#include "json.hpp"

struct Column {
    size_t start;
    std::string data;
};

inline std::vector<Column> split_line_into_columns(const std::string& line) {
    std::vector<Column> cols;
    size_t i = 0;
    size_t n = line.size();
    
    while(true) {
        // search for the beginning of a column - first non-space character
        while(i < n && is_space(line[i])) {
            ++i;
        }

        if (i >= n) {
            return cols;
        }

        Column col = { i, {} };

        // read till the end of the column - two consecutive spaces
        int spaces = 0;
        while(i < n && spaces < 2) {
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

struct Table {
    std::vector< std::string > header;
    std::vector< std::vector< std::string > > data;
};

inline Table parse_revisions_table(const std::vector<std::string>& data, size_t start_pos) {
    size_t i = start_pos;
    size_t n = data.size();

    Table table;

    std::vector<Column> last_row;
    size_t max_search = 7;
    size_t lines_searched = max_search;

    while(true) {
        if (lines_searched == 0) {
            return table;
        }
        if (i >= n) {
            return table;
        }

        auto cols = split_line_into_columns(data[i]);
        
        if (cols.size() >= 2) {
            table.data.emplace_back();
            for (auto col : cols) {
                table.data.back().push_back(col.data);
            }
            lines_searched = 3;
            last_row = cols;
        } else if (cols.size() == 1) {
            if (!last_row.empty() && last_row.back().start == cols[0].start) {
                append_line(table.data.back().back(), cols[0].data);
            } else {
                std::cout << "skipping: " << data[i] << "\n";
            }
        }

        i++;
        lines_searched--;
    }
}

inline nlohmann::json read_revision_table(const Table& table) {
    assert( !table.data.empty() );

    // first check if all rows have the same length, otherwise this is probably wrong
    size_t len = table.data[0].size();
    if (std::any_of(table.data.begin(), table.data.end(), [len](const auto &row) {return row.size() != len; })) {
        throw std::runtime_error("Rows of different size!");
    }

    // now check if the first row is the header
    bool has_header = true;
    
    // the header probably contains no numbers and just a few word
    for (const auto& caption : table.data[0]) {
        if (count_words(caption) > 4) {
            has_header = false;
            break;
        }
        if (std::any_of(caption.begin(), caption.end(), is_digit)) {
            has_header = false;
            break;
        }
    } 

    // if there is a header and its the only row its probably wrong
    if (has_header && table.data.size() == 1) {
         throw std::runtime_error("Header is the only row!");
    } 

     
    
    static std::regex version_reg(R"(ver|rev)", std::regex_constants::icase);
    static std::regex date_reg(R"(date)", std::regex_constants::icase);
    static std::regex description_reg(R"(description|changes|subject)", std::regex_constants::icase);
    std::smatch match;

    // now try to guess what data each column contains
    enum { VERSION, DATE, DESCRIPTION };
    std::array<int, 3> mapping { -1, -1, -1 };
    
    if (has_header) {
        const auto& header = table.data[0]; 
        for (size_t i = 0; i < len; ++i) {
            if (std::regex_search(header[i], match, version_reg)) {
                if (mapping[VERSION] == -1) {
                    mapping[VERSION] = i;
                }
                
            }
            if (std::regex_search(header[i], match, date_reg)) {
                if (mapping[DATE] == -1) {
                    mapping[DATE] = i;
                }
            }
            if (std::regex_search(header[i], match, description_reg)) {
                if (mapping[DESCRIPTION] == -1) {
                    mapping[DESCRIPTION] = i;
                }
            }
        }
    }

    nlohmann::json j = nlohmann::json::array({});
    for (size_t i = has_header ? 1 : 0; i < table.data.size(); ++i) {
        const auto& row = table.data[i];
        j.push_back( nlohmann::json{} );
        if (mapping[VERSION] != -1) {
            j.back()["version"] = row[mapping[VERSION]];
        }
        if (mapping[DATE] != -1) {
            j.back()["date"] = row[mapping[DATE]];
        }
        if (mapping[DESCRIPTION] != -1) {
            j.back()["decription"] = row[mapping[DESCRIPTION]];
        }
    }

    return j;
}

/**
 * Returns true iff the provided line is a title of the section containing the revisions table
 * 
 * Rules:
 *  - title should be reasonable short - at most 3 words
 *  - title should contain one of the keywords: 'revision', 'evolution', 'version'
 */
inline bool is_revisions_title(const std::string& line) {
    static std::regex title_reg(R"(revision|evolution|version)", std::regex_constants::icase);
    std::smatch match;
    return std::regex_search(line, match, title_reg) && count_words(line) <= 3;
}

/**
 * Determines if the provided line contains a header of the revisions table
 * 
 * Rules:
 *  - the header has at least 2 columns (since the table must have at least two columns)
 *  - the table typically contains columns 'version', 'date' and 'description' so
 *    we have to succesfully find at least two of those
 *  - ??? the header probably contains no numbers ???
 */
inline bool is_revisions_header(const std::string& line) {
    std::vector<Column> cols = split_line_into_columns(line);
    if (cols.size() < 2) {
        return false;
    }

    static std::regex version_reg(R"(ver|rev)", std::regex_constants::icase);
    static std::regex date_reg(R"(date)", std::regex_constants::icase);
    static std::regex description_reg(R"(description|changes|subject)", std::regex_constants::icase);
    std::smatch match;

    int matched = 0;
    int ver = 1;
    int date = 1;
    int desc = 1;
    for (const auto& caption : cols) {
        if (count_words(caption.data) > 4) {
            continue;
        }
        if (std::regex_search(caption.data, match, version_reg)) {
            matched += ver;
            ver = 0;
        } else if (std::regex_search(caption.data, match, date_reg)) {
            matched += date;
            date = 0;
        } else if (std::regex_search(caption.data, match, description_reg)) {
            matched += desc;
            desc = 0;
        }
    }

    return matched >= 2;
}

inline nlohmann::json parse_revisions(const std::vector<std::string>& data) {
    size_t search_depth = data.size();

    for (size_t i = 0; i < search_depth; ++i) {
        if (is_revisions_header(data[i])) {
            std::cout << "found: " << data[i] << "\n";
            Table table = parse_revisions_table(data, i);
            if (table.data.empty()) {
                continue;
            }
            /*for (auto& row : table.data) {
                for (auto s : row) {
                    std::cout << s << " | ";
                }
                std::cout << "\n";
            }
            break;*/
            try {
                return read_revision_table(table);
            } catch (std::runtime_error& e) {

            }
        }
    }

    return {};
}
