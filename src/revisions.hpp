#pragma once

#include <string>
#include <cctype>
#include <vector>
#include <regex>
#include <sstream>

#include "utils.hpp"
#include "json.hpp"

class RevisionsParser {
    const std::vector<std::wstring>& data;
    
public:
    RevisionsParser(const std::vector<std::wstring>& data) : data(data) {}

    nlohmann::json parse() {
        static std::wregex title_reg(LR"(revision|evolution)", std::regex_constants::icase);
        size_t limit = 200;
        std::wsmatch match;
        for (size_t i = 0; i < limit; ++i) {
            if (std::regex_search(data[i], match, title_reg) && count_words(data[i]) <= 3) {
                for (int j = 0; j < 5; ++j) {
                    std::wcout << L"Line " << i + 1 << L": " << data[i] << L"\n";
                    ++i;
                }
                std::wcout << L"-----------------------------------------------\n";
            }
        }
        return {};
    }

private:

    size_t count_words(const std::wstring& line) {
        std::wstringstream s(line);
        std::wstring tok;
        size_t count = 0;
        while (s) {
            s >> tok;
            count++;
        }
        return count;
    }
};
