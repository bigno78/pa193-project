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

bool is_suffix_char(char c) {
    //const static std::string allowed_chars = "/-";
    if (is_space(c)) {
        return false;
    }
    return is_digit(c) || c == '/' || c == '.' || c == '+';
}

/**
 * Find where the suffix of the version string ends.
 * 
 * @return the index of the first character after the end of the suffix
 *         or std::string::npos is the suffix would have 0 non-space chars
 */
size_t find_suffix_end(const std::string& line, size_t pos) {
    size_t n = line.size();

    if (pos >= n) {
        return std::string::npos;
    }

    // there could be either execatly one space or a dash
    if (is_space(line[pos]) || line[pos] == '-' || line[pos] == '_') {
        pos++;
    }

    if (pos >= n) {
        return std::string::npos;
    }

    if (line[pos] == '{') {
        char closing_brace = '}';
        while (pos < n && line[pos] != closing_brace) {
            ++pos;
        }
        return pos < n ? pos + 1 : pos;
    }

    if (!is_digit(line[pos])) {
        return std::string::npos;
    }

    while (pos < n && is_suffix_char(line[pos])) {
        ++pos;
    }
    return pos;
}


size_t find_prefix_start(const std::string& line, size_t pos) {
    size_t max = static_cast<size_t>(-1);

    if (pos == max) {
        return std::string::npos;
    }
    
    // we allow one space 
    if (pos > 0 && is_space(line[pos])) {
        --pos;
    }

    if (is_space(line[pos])) {
        return std::string::npos;
    }

    while (pos != max && !is_space(line[pos])) {
        --pos;
    }

    if (pos == max) {
        return 0;
    }
    return pos + 1;
}

size_t get_des_prefix(const std::string& line, size_t pos) {
    size_t start = find_prefix_start(line, pos-1);
    if (start == std::string::npos) {
        return start;
    }

    assert(start < pos);

    if (is_space(line[pos - 1])) {
        if (line.substr(start, pos - start - 1) == "triple") {
            return start;
        }
        return std::string::npos;
    }
    
    if (line[pos - 1] == '-') {
        pos--;
    }

    if (start >= pos) {
        return std::string::npos;
    }

    if (pos - start == 1 && is_alnum(line[start])) {
        return start;
    }

    if (line.substr(start, pos - start) == "triple") {
        return start;
    }

    return std::string::npos;
}

/**
 * Remove non-digit characters from the end.
 */
size_t remove_junk_suffix(const std::string& str, size_t start, size_t end) {
    if (end > start && str[end-1] == '}') {
        return end;
    }
    while (end > start && !is_digit(str[end-1]) && str[end-1] != '+') {
        --end;
    }
    return end;
}

// start is the first char which should be considered
size_t get_suffix(const std::string& str, size_t start) {
    size_t pos = find_suffix_end(str, start);
    if (pos == std::string::npos) {
        return pos;
    }

    pos = remove_junk_suffix(str, start, pos);

    // if there is nothing or its too long its probably invalid suffix
    if (pos <= start || pos - start >= 20) {
        return std::string::npos;
    }

    return pos;
}

void find_des(const std::string& line,
              const std::string& str,
              const std::string& searchee,
              std::set<std::string>& results)
{
    size_t i = 0;
    size_t j = 0;
    while (j < str.size()) {
        i = str.find(searchee, j);
        if (i == std::string::npos) {  // wasnt found
            break;
        }
        j = i + searchee.size();

        size_t end = get_suffix(str, j);
        size_t start = get_des_prefix(str, i);

        // there must be at least suffix or at least prefix
        if (end == std::string::npos && start == std::string::npos) {
            continue;
        }

        if (end == std::string::npos) {
            end = j;
        }

        if (start == std::string::npos) {
            start = i;
        }

        results.insert(line.substr(start, end - start));
        j = end;
    }
}


void find_versions(const std::string& line,
                   const std::string& str,
                   const std::string& searchee,
                   std::set<std::string>& results) 
{
    size_t i = 0;  // begining of the matched string
    size_t j = 0;  // one ofter the end of the last match
    while(j < str.size()) {
        i = str.find(searchee, j);
        if (i == std::string::npos) {  // wasnt found
            break;
        }
        j = i + searchee.size();

        // must be the begining of a word
        if (i != 0 && !is_space(str[i - 1]) && str[i - 1] != '(') {
            continue;
        }

        size_t end = get_suffix(str, j);
        if (end == std::string::npos) {
            continue;
        }
        
        results.insert(line.substr(i, end - i));
        j = end;
    }
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

    size_t line_no = 0;
    for (const auto& line : data) {
        str.clear();
        for (size_t i = 0; i < line.size(); ++i) {
            str.push_back(to_lower(line[i]));
        }

        find_des(line, str, "des", des);
        find_versions(line, str, "eal", eal);
        find_versions(line, str, "sha", sha);
        find_versions(line, str, "rsa", rsa);
        find_versions(line, str, "ecc", rsa);
        find_versions(line, str, "java card", java_card);
        find_versions(line, str, "global platform", global_platfom);

        line_no++;
    }

    js::json js;

    if (!eal.empty()) js["eal"] = js::json(eal);
    if (!rsa.empty()) js["rsa"] = js::json(rsa);
    if (!sha.empty()) js["sha"] = js::json(sha);
    if (!ecc.empty()) js["ecc"] = js::json(ecc);
    if (!des.empty()) js["des"] = js::json(des);
    if (!global_platfom.empty()) js["global_platfom"] = js::json(global_platfom);
    if (!java_card.empty()) js["java_card"] = js::json(java_card);

    /*for (const auto& s : eal) {
        std::cout << s << "\n";
    }*/

    return js;
}
