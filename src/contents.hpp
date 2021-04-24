#include <string>

struct TocItem {
    std::string chapter_num;
    std::string chapter_name;
    int page;
};

/**
 * Return the next index after and including i that is not a space.
 * If there is no such index return line.size()
 */
size_t ignore_whitespace(const std::string& line, size_t i) {
    while (i < line.size() && is_space(line[i])) {
        ++i;
    }
    return i;
}

/**
 * Return the next index after and including i that contains a space.
 * If there is no such index return line.size()
 */
size_t next_whitespace(const std::string& line, size_t i) {
    while (i < line.size() && !is_space(line[i])) {
        ++i;
    }
    return i;
}

size_t find_chapter_num_end(const std::string& line, size_t i) {
    assert(line.size() > i);

    if (is_digit(line[i])) {
        size_t pos = next_whitespace(line, i);
        return pos;
    } else if (is_upper(line[i])) {
        if (i+1 < line.size() && is_space(line[i+1])) {
            return i+1;
        }
        if (i+2 < line.size() && line[i+1] == '.' && is_space(line[i+2])) {
            return i+2;
        }
    }    

    return i;
}

size_t find_chapter_name_end(const std::string& line, size_t i) {
    size_t pos = i;

    size_t mezera_count = 0; // number of consecutive spaces
    size_t dot_count = 0; // number of consecutive dots

    while (pos < line.size() && dot_count < 2 && mezera_count < 2) {
        if (is_space(line[pos])) {
            mezera_count++;
        } else if (line[pos] == '.') {
            dot_count++;
        } else {
            mezera_count = 0;
            dot_count = 0;
        }
        pos++;
    }

    if (dot_count == 2) {
        pos -= 2;
    }

    return pos;
}

size_t ignore_padding(const std::string& line, size_t i) {
    while (i < line.size() && (line[i] == '.' || is_space(line[i]))) {
        ++i;    
    }
    return i;
}

size_t find_page_num_end(const std::string& line, size_t i) {
    while (i < line.size() && is_digit(line[i])) {
        ++i;
    }
    return i;
}



nlohmann::json parse_contents(std::vector<std::string>& data) {
    std::vector<TocItem> contents;
    size_t i = 0;
    static const std::regex reg(R"(content|index)", std::regex_constants::icase);
tryagain:
    
    for (; i < data.size(); i++) {
        if (is_title(data[i], reg)) {
            //std::cout << data[i] << std::endl;
            i++;
            while (i < data.size() && (is_empty_line(data[i]) || is_title(data[i], reg))) {
                i++;
            }
            break;
        }
    }

    constexpr int max_tolerance = 10;
    int tolerance = max_tolerance;

    std::string chapter_num;
    std::string chapter_name;
    std::string page;
    
    std::string line;
    
    while (i < data.size() && tolerance > 0) {
        chapter_num.clear();
        chapter_name.clear();
        page.clear();

        if (is_empty_line(data[i])) {
            i++;
            continue;
        }
        if (is_pagebreak(data[i])) {
            ++i;
            tolerance = max_tolerance;
            continue;
        }

        line = data[i];
        trim(line);
        //std::cout << "tolerance: " << tolerance << std::endl;
        //std::cout << line << std::endl;
        size_t pos1 = find_chapter_num_end(line, 0);
        if (pos1 >= line.size()) {
            i++;
            tolerance--;
            continue;
        }
        if (pos1 > 0 && line[pos1-1] == '.') {
            chapter_num = line.substr(0, pos1-1);    
        } else {
            chapter_num = line.substr(0, pos1);
        }

        pos1 = ignore_whitespace(line, pos1);
        if (pos1 >= line.size() || line[pos1] == '.') {
            i++;
            tolerance--;
            continue;
        }
        
        size_t pos2 = find_chapter_name_end(line, pos1);
        if (pos2 >= line.size()) {
            i++;
            tolerance--;
            continue;
        }
        chapter_name = line.substr(pos1, pos2 - pos1);

        /*auto pos2 = line.find('.', pos1);
        if (pos2 == std::string::npos) {
            //std::cout << "druhy" << std::endl;
            pos2 = line.find_first_of(" \n\t\r\v\f", pos1);
            //std::cout << line << std::endl;
            //std::cout << pos1 << std::endl;
            //std::cout << pos2 << " " << line[pos2] << std::endl;
            auto postemp = line.find_first_of("abcdefghijklmnopqrstuvwxyz", pos2);
            while (postemp != std::string::npos) {
                pos2 = postemp+1;
                while (isalnum(line[pos2])) {
                    pos2++;
                }
                postemp =
                    line.find_first_of("abcdefghijklmnopqrstuvwxyz", pos2);
            }
            if (pos2 == std::string::npos) {
                if (tolerance > 0) {
                    i++;
                    tolerance--;
                    continue;
                }
                break;
            }
        }*/

        pos2 = ignore_padding(line, pos2);
        size_t pos3 = find_page_num_end(line, pos2);
        if (pos3 > line.size()) {
            i++;
            tolerance--;
            continue;
        }
        if (pos3 <= line.size()-1 && !is_space(line[pos3])) {
            i++;
            tolerance--;
            continue;
        } /**
        mezera_count = 0;
        dot_count = 0;
        size_t poscheck = pos3-1;
        bool die = false;
        while (poscheck > pos3 - 10 && mezera_count < 2 && dot_count < 2) {
            std::cout << line[poscheck] << std::endl;
            if (isspace(line[poscheck])) {
                mezera_count++;
            } else if (line[poscheck] == '.') {
                dot_count++;
            } else if (is_digit(line[poscheck])){
                mezera_count = 0;
                dot_count = 0;
            } else {
                std::cout << "im breaking everything" << std::endl;
                die = true;
            }
            poscheck--;
        }
        if (die || poscheck <= pos3 - 10) {
            std::cout << die << std::endl;
            std::cout << "heuheuehuheuheuehueh" << std::endl;
            i++;
            tolerance--;
            continue;
        }
        **/
        page = line.substr(pos2, pos3 - pos2);
        if (chapter_name.empty() || page.empty()) {
            i++;
            tolerance--;
            break;
        }

        try {
            contents.push_back( {chapter_num, chapter_name, std::stoi(page)});
        } catch (...) {
            assert(false); // this should not happen
        }
        tolerance = max_tolerance;
        i++;    
    }
    
    if (i < data.size() && contents.size() < 5) {
        contents.clear();
        goto tryagain;
    }
    
    nlohmann::json toret = nlohmann::json::array({});
    for (size_t i = 0; i < contents.size(); i++) {
        //std::cout << std::get<1>(contents[i]) << std::endl;
        toret.push_back(nlohmann::json::array({ 
            contents[i].chapter_num, 
            contents[i].chapter_name, 
            contents[i].page 
        }));
    }

    return toret;
}