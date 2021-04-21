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

std::string_view read_next_word(const std::string& line, size_t pos) {
    size_t n = line.size();
    while (pos < n && is_space(line[pos])) {
        ++pos;
    }
    if (pos == n)
        return {};

    size_t start = pos;
    while (pos < n && !is_space(line[pos]) && line[pos] != '{') {
        ++pos;
    }

    if (pos == n || is_space(line[pos])) {
        return {&line[start], pos - start};
    }

    char closing_brace = '}';
    while (pos < n && line[pos] != closing_brace) {
        ++pos;
    }

    if (pos == n) {
        return {};
    }
    return {&line[start], pos - start + 1};
}

void remove_junk_suffix(std::string_view& word) {
    static std::string banned_chars = ".?!>),;'\"";
    while (!word.empty() && is_in(word.back(), banned_chars)) {
        word.remove_suffix(1);
    }
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

        j = i + 3;

        if (i != 0 && !is_space(str[i - 1])) {
            continue;
        }

        if (j == str.size()) {
            break;
        }

        std::string_view word = read_next_word(str, j);
        if (word.empty()) {
            break;
        }
        remove_junk_suffix(word);
        j = i + word.end() - &str[i];

        size_t num_count = 0;
        for (auto c : word) {
            if (is_digit(c))
                num_count++;
        }
        if (num_count != 0 && float(num_count) / word.size() >= 0.4) {
            results.insert(line.substr(i, j - i));
        }
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

        find_versions(line, str, "eal", eal);
        find_versions(line, str, "sha", sha);
        find_versions(line, str, "rsa", rsa);
    }

    js::json js;
    js["eal"] = js::json(eal);

    for (const auto& s : eal) {
        std::cout << s << "\n";
    }
    for (const auto& s : rsa) {
        std::cout << s << "\n";
    }
    for (const auto& s : sha) {
        std::cout << s << "\n";
    }

    return js;
}
