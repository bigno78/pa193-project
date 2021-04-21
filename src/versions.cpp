#include <algorithm>
#include <iostream>
#include <regex>
#include <set>
#include <string_view>

#include "versions.hpp"

#include "utils.hpp"

namespace js = nlohmann;

/*
- JSON object with keys: eal, global_platform, java_card, sha, rsa, ecc, des
- the keys do not need to be present if corresponding version was not found
within the document
- the value under the key is a list of strings with versions as they were
matched in text, e.g., ["Java Card 3.0.1", "JavaCard 3.1.0"]
*/

bool is_in(char c, const std::string& str) {
    for (char a : str) {
        if (c == a)
            return true;
    }
    return false;
}

std::size_t find_version_end(const std::string& line, size_t pos) {
    size_t n = line.size();
    while (pos < n && is_space(line[pos])) {
        ++pos;
    }
    if (pos == n)
        return std::string::npos;

    while (pos < n && !is_space(line[pos]) && line[pos] != '{') {
        ++pos;
    }

    if (pos == n || is_space(line[pos])) {
        return pos;
    }

    char closing_brace = '}';
    while (pos < n && line[pos] != closing_brace) {
        ++pos;
    }
    return pos + 1;

    /*if (pos == n) {
        return std::string::npos;
    }
    return pos + 1;*/
}

size_t remove_junk_suffix(const std::string& str, size_t start, size_t end) {
    static std::string banned_chars = ".?!>),;'\"-]";
    while (end > start && is_in(str[end - 1], banned_chars)) {
        --end;
    }
    return end;
}

void find_versions(const std::string& line,
                   const std::string& str,
                   const std::string& searchee,
                   std::set<std::string>& results) 
{
    size_t i = 0;  // begining of the matched string
    size_t j = 0;  // one ofter the end
    do {
        i = str.find(searchee, j);
        if (i == std::string::npos) {  // wasnt found
            break;
        }
        j = i + searchee.size();

        // must be the begining of a word
        if (i != 0 && !is_space(str[i - 1])) {
            continue;
        }

        // there must be something more
        if (j == str.size()) {
            break;
        }

        size_t pos = find_version_end(str, j);
        if (pos == std::string::npos) {
            break;
        }
        pos = remove_junk_suffix(str, j, pos);

        if (pos == j || pos - j >= 15) {
            continue;
        }

        if (is_in(str[pos-1], "=:")) {
            continue;
        }

        size_t num_count = 0;
        size_t char_count = 0;
        for (size_t idx = j; idx < pos; ++idx) {
            if (is_digit(str[idx]))
                num_count++;
            if (!is_space(str[idx]))
                char_count++;
        }

        if (num_count != 0 && float(num_count) / char_count >= 0.4) {
            results.insert(line.substr(i, pos - i));
        }
        j = pos;
    } while (i != std::string::npos);
}

nlohmann::json parse_versions(const std::vector<std::string>& data) {
    std::string str;

    std::set<std::string> eal;
    std::set<std::string> rsa;
    std::set<std::string> ecc;
    std::set<std::string> sha;
    std::set<std::string> global_platfom;
    std::set<std::string> java_card;
    std::set<std::string> des;

    for (const auto& line : data) {
        str.clear();
        for (size_t i = 0; i < line.size(); ++i) {
            str.push_back(to_lower(line[i]));
        }

        //find_versions(line, str, "eal", eal);
        //find_versions(line, str, "sha", sha);
       // find_versions(line, str, "rsa", rsa);
       find_versions(line, str, "java card", java_card);
    }

    js::json js;
    js["eal"] = js::json(eal);

    for (const auto& s : java_card) {
        std::cout << s << "\n";
    }
    /*for (const auto& s : rsa) {
        std::cout << s << "\n";
    }
    for (const auto& s : sha) {
        std::cout << s << "\n";
    }*/

    return js;
}
