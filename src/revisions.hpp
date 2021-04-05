#pragma once

#include <string>
#include <cctype>
#include <vector>
#include <regex>
#include <sstream>
#include <optional>

#include "utils.hpp"
#include "json.hpp"

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
