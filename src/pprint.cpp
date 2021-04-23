#include <iostream>
#include <iomanip>

#include "pprint.hpp"
#include "utils.hpp"

void pprint(const nlohmann::json& js, const std::set<SectionType>& sections, size_t max_width) {
    for (auto sect : sections) {
        switch (sect) {
            case SectionType::bibliography:
                pprint_bibliography(js["bibliography"], max_width);
                break;
            case SectionType::contents:
                pprint_contents(js["contents"], max_width);
                break;
            case SectionType::revisions:
                pprint_revisions(js["revisions"], max_width);
                break;
            case SectionType::title:
                pprint_title(js["title"], max_width);
                break;
            case SectionType::versions:
                pprint_versions(js["versions"], max_width);
                break;
        }
    }
}

size_t print_wrapped(const std::string& str, size_t start_pos, size_t max_width) {
    for (size_t i = 0; i < max_width; ++i) {
        if (start_pos + i >= str.size()) {
            return std::string::npos;
        }
        std::cout << str[start_pos + i];
    }
    return start_pos + max_width;
}

void pprint_title(const nlohmann::json& js, size_t max_width) {

}

void pprint_bibliography(const nlohmann::json& js, size_t max_width) {
    if (js.empty()) {
        return;
    } 

    std::vector<std::pair<std::string, std::string>> entries(js.size());
    size_t i = 0;
    bool all_numbers = true;
    for (const auto& item : js.items()) {
        assert((item.key().size() >= 3 &&  item.key().front() == '[' && item.key().back() == ']'));
        std::string k = item.key().substr(1, item.key().size() - 2);
        if (!std::all_of(k.begin(), k.end(), is_digit)) {
            all_numbers = false;
        }
        entries[i++] = { k, item.value() };
    }

    auto cmp_function = [all_numbers] (const auto& a, const auto& b) {
        if (all_numbers) {
            return std::stoi(a.first) < std::stoi(b.first);
        }
        return a.first < b.first;
    };

    std::sort(entries.begin(), entries.end(), cmp_function);

    std::cout << "BIBLIOGRAPHY\n\n";

    for (const auto& [ key, value ] : entries) {
        std::cout << "  [" << key << "]  ";
        size_t indent_size = 6   + key.size();
        size_t i = 0;
        while (i != std::string::npos) {
            if (i != 0) {
                std::cout << std::setw(indent_size) << "";
            }
            i = print_wrapped(value, i, max_width - indent_size);
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}


void print_hline(const std::array<size_t, 3>& widths) {
    std::cout << "+";
    for (auto w : widths) {
        for (size_t i = 0; i < w; ++i) {
            std::cout << "-";
        }
        std::cout << "+";
    }
    std::cout << "\n";
}

void print_cell_centered(const std::string& str, size_t w) {
    size_t left = (w - str.size())/2;
    size_t right = (w - str.size() + 1)/2;
    std::cout << std::setw(left) << "";
    std::cout << str;
    std::cout << std::setw(right) << "";
}

void print_cell_wrapped(const std::string& str, size_t w, const std::array<size_t, 3>& widths) {
    size_t str_w = w - 2;
    size_t k = str.size()/str_w;
    for (size_t i = 0; i < str.size()/str_w; ++i) {
        if (i > 0) {
            std::cout << "|" << std::setw(widths[0] + 1) << "|"
                      << std::setw(widths[1] + 1) << "|";
        }
        std::cout << " ";
        for (size_t j = i*str_w; j < (i + 1)*str_w; ++j) {
            std::cout << str[j];
        }
        std::cout << " |\n";
    }
    if (str.size() > str_w) {
        std::cout << "|" << std::setw(widths[0] + 1) << "|"
                    << std::setw(widths[1] + 1) << "|";
    }
    std::cout << " ";
    for (size_t i = k*str_w; i < str.size(); ++i) {
        std::cout << str[i];
    }
    for (size_t i = 0; i < str_w - str.size() % str_w; ++i) {
        std::cout << " ";
    }
    std::cout << " |\n";
}

void pprint_revisions(const nlohmann::json& js, size_t max_width) {
    if (js.empty()|| !js.is_array()) {
        return;
    }

    std::array<size_t, 3> widths = { 9, 12, 0 };
    widths[2] = max_width - widths[0] - widths[1];

    print_hline(widths);
    std::cout << "|";
    int i = 0;
    for (std::string s : {"version", "date", "description"}) {
        size_t left = (widths[i] - s.size())/2;
        size_t right = (widths[i] - s.size() + 1)/2;
        std::cout << std::setw(left) << "";
        std::cout << s;
        std::cout << std::setw(right) << "";
        std::cout << "|";
        ++i;
    }
    std::cout << "\n";
    print_hline(widths);

    for (const auto& rev : js) {
        std::cout << "|";
        print_cell_centered(rev["version"], widths[0]);
        std::cout << "|";
        print_cell_centered(rev["date"], widths[1]);
        std::cout << "|";
        print_cell_wrapped(rev["description"], widths[2], widths);
        print_hline(widths);
    }

}

void pprint_versions(const nlohmann::json& js, size_t max_width) {

}

void pprint_contents(const nlohmann::json& js, size_t max_width) {

}
