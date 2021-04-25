#include <iostream>
#include <iomanip>

#include "pprint.hpp"
#include "utils.hpp"

void pprint(const nlohmann::json& js, const std::set<SectionType>& sections, size_t max_width) {
    if (sections.count(SectionType::title) > 0 && js.contains("title")) {
        pprint_title(js["title"], max_width);
    }
    if (sections.count(SectionType::revisions) > 0 && js.contains("revisions")) {
        pprint_revisions(js["revisions"], max_width);
    }
    if (sections.count(SectionType::versions) > 0 && js.contains("versions")) {
        pprint_versions(js["versions"], max_width);
    }
    if (sections.count(SectionType::contents) > 0 && js.contains("table_of_contents")) {
        pprint_contents(js["table_of_contents"], max_width);
    }
    if (sections.count(SectionType::bibliography) > 0 && js.contains("bibliography")) {
        pprint_bibliography(js["bibliography"], max_width);
    }
}

size_t print_wrapped(const std::string& str, size_t start_pos, size_t max_width) {
    size_t i = 0;
    for (; i < max_width; ++i) {
        if (start_pos + i >= str.size()) {
            return str.size();
        }
        std::cout << str[start_pos + i];
    }
    return start_pos + max_width;
}

void pprint_title(const nlohmann::json& js, size_t max_width) {
    if (!js.is_string() || js.empty()) {
        return;
    }

    std::cout << "\n";

    std::string title = js;

    size_t margin = 0.1*max_width;
    size_t text_width = max_width - 2*margin;

    size_t i = 0;
    while (i + text_width < title.size()) {
        std::cout << std::setw(margin) << "";
        for (size_t j = i; j < i + text_width; ++j) {
            std::cout << title[j];
        }
        std::cout << std::setw(margin) << "" << "\n";
        i += text_width;
    }

    size_t remaining = title.size() - i;
    size_t left = (text_width - remaining + 1)/2;
    size_t right = (text_width - remaining)/2;

    std::cout << std::setw(margin + left) << "";
    for (size_t j = i; j < title.size(); ++j) {
        std::cout << title[j];
    }
    std::cout << std::setw(margin + right) << "" << "\n\n\n";
}

void pprint_bibliography(const nlohmann::json& js, size_t max_width) {
    if (!js.is_object() || js.empty()) {
        return;
    } 

    std::vector<std::pair<std::string, std::string>> entries(js.size());
    size_t i = 0;
    bool all_numbers = true;
    size_t key_width = 0;
    for (const auto& item : js.items()) {
        if (!item.value().is_string() || item.key().size() < 3) {
            return;
        }
        std::string k = item.key().substr(1, item.key().size() - 2);
        if (!std::all_of(k.begin(), k.end(), is_digit)) {
            all_numbers = false;
        }
        key_width = std::max(key_width, item.key().size());
        entries[i++] = { item.key(), item.value() };
    }

    std::cout << "BIBLIOGRAPHY\n\n";

    if (key_width + 4 + 5 > max_width) {
        std::cerr << "The width is too small to print this bibliography!\n\n";
        return;
    }

    auto cmp_function = [all_numbers] (const auto& a, const auto& b) {
        if (all_numbers) {
            return std::stoi(a.first) < std::stoi(b.first);
        }
        return a.first < b.first;
    };

    std::sort(entries.begin(), entries.end(), cmp_function); 

    for (const auto& [ key, value ] : entries) {
        std::cout << "  " << key << "  ";
        size_t indent_size = 4 + key.size();
        size_t i = print_wrapped(value, 0, max_width - indent_size);
        while (i != value.size()) {
            std::cout << "\n" << std::setw(indent_size) << "";
            i = print_wrapped(value, i, max_width - indent_size);
        }
        std::cout << "\n\n";
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
    if (!js.is_array()) {
        return;
    }

    std::vector<std::array<std::string, 3>> data;
    std::array<size_t, 3> widths = { 9, 4, 0 };
    size_t margin = 1;

    for (const auto& rev : js) {
        if (!rev.is_object() || !rev.contains("version") ||
            !rev.contains("date") || !rev.contains("description"))
        {
            return;
        }
        if (!rev["date"].is_string() || !rev["version"].is_string() || 
            !rev["description"].is_string())
        {
            return;
        }
        data.push_back({ rev["version"], rev["date"], rev["description"] });
        widths[0] = std::max(widths[0], data.back()[0].size());
        widths[1] = std::max(widths[1], data.back()[1].size());
    }
    widths[0] += 2;
    widths[1] += 2;

    std::cout << "REVISIONS\n\n";

    if (widths[0] + widths[1] + 2*margin + 4 + 11 > max_width) {
        std::cerr << "Max width too small to print the revisions!\n\n";
        return;
    }

    widths[2] = max_width - widths[0] - widths[1] - 4;

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
    if (!js.is_object()) {
        return;
    }

    std::cout << "VERSIONS\n\n";

    for (const auto& [ key, vals ] : js.items()) {
        if (!vals.is_array()) {
            return;
        }
        std::cout << "  - " << key << ": ";
        size_t indent = 6 + key.size(); 
        size_t w = indent;
        bool first = true;
        for (const auto& v : vals) {
            std::string val = v;
            size_t item_width = val.size() + 1;
            if (!first) item_width += 2;
            if (w + item_width > max_width) {
                std::cout << (first ? "" : ",");
                std::cout << "\n" << std::setw(indent) << "";
                w = indent;
            } else if (!first){
                std::cout << ", ";
                w += 2;
            }
            std::cout << val;
            w += val.size();
            first = false;
        }
        std::cout << "\n\n";
    }

    std::cout << "\n\n";
}

void pprint_contents(const nlohmann::json& js, size_t max_width) {
    if (!js.is_array()) {
        return;
    }

    std::vector<std::array<std::string, 3>> data;
    size_t section_width = 0;
    size_t page_width = 0;
    for (const auto& item : js) {
        if (!item.is_array()) return;
        if (!item[0].is_string() || !item[1].is_string() || !item[2].is_number()) {
            return;
        }

        std::string section = item[0];
        std::string name = item[1];
        std::string page = std::to_string(size_t(item[2]));

        section_width = std::max(section.size(), section_width);
        page_width = std::max(page.size(), page_width);

        data.push_back({ section, name, page });
    }

    std::cout << "TABLE OF CONTENTS\n\n";

    if (section_width + page_width + 2 + 10 + 1 > max_width) {
        std::cerr << "Sorry, the width is too small for printing contents.\n\n";
        return;
    }

    size_t middle_width = max_width - section_width - page_width - 2 - 1;
    size_t min_dots = 0.3*middle_width;
    size_t max_name = middle_width - min_dots;

    for (const auto& [ sec, name, page ] : data) {
        std::cout << std::setw(section_width + 2) << std::left << sec;

        size_t i = 0;
        size_t j = print_wrapped(name, i, max_name);
        while (j != name.size()) {
            std::cout << "\n" << std::setw(section_width + 2) << "";
            i = j;
            j = print_wrapped(name, i, max_name);
        }

        size_t needed = middle_width - (j - i);
        std::cout << std::setw(needed) << std::setfill('.') << "";
        std::cout << " " << page;
        std::cout << std::setfill(' ') << "\n";
    }

    std::cout << "\n\n";
}
