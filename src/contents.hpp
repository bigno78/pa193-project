#include <string>

struct TocItem {
    std::string chapter_num;
    std::string chapter_name;
    int page;
};

size_t ignore_whitespace(const std::string& line, size_t i) {
    while (i < line.size() && is_space(line[i])) {
        ++i;
    }
    return i;
}

nlohmann::json parse_contents(std::vector<std::string>& data) {
    nlohmann::json toret = nlohmann::json::array({});
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

    /*if (i >= data.size()) {
        return {};
    }*/

    constexpr int max_tolerance = 10;
    int tolerance = max_tolerance;

    std::string chapter_num;
    std::string chapter_name;
    std::string page;
    
    std::string line;
    while (1) {
        chapter_num.clear();
        chapter_name.clear();
        page.clear();

        if (i >= data.size() || tolerance <= 0) {
            break;
        }
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
        std::cout << "tolerance: " << tolerance << std::endl;
        std::cout << line << std::endl;
        size_t pos1 = 0;
        if (is_digit(line[0])) {
            pos1 = line.find(' ');
            if (pos1 == std::string::npos) {
                i++;
                tolerance--;
                continue;
            }
            chapter_num = line.substr(0, pos1);
            trim(chapter_num);
        } else if ( line.size() >= 3 && is_upper(line[0]) && line[1] == '.' && is_space(line[2])) {
            pos1 = 2;
            chapter_num = line.substr(0, pos1);
        }

        pos1 = ignore_whitespace(line, pos1);
        if (pos1 >= line.size() || line[pos1] == '.') {
            std::cout << "1" << std::endl;
            i++;
            tolerance--;
            continue;
        }
        
        size_t pos2 = pos1;
        size_t mezera_count = 0;
        size_t dot_count = 0;
        while (pos2 < line.size() && dot_count < 2 && mezera_count < 2) {
            if (is_space(line[pos2])) {
                ++mezera_count;
            } else if (line[pos2] == '.') {
                dot_count++;
            } else {
                mezera_count = 0;
                dot_count = 0;
            }
            ++pos2;
        }
        if (dot_count == 2) {
            pos2 -= 2;
        }

        if (pos2 >= line.size()) {
            std::cout << "2" << std::endl;
            i++;
            tolerance--;
            continue;
        }

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

        chapter_name = line.substr(pos1, pos2 - pos1);
        trim(chapter_name);

        while (pos2 < line.size() &&
               (line[pos2] == '.' || is_space(line[pos2]))) {
            ++pos2;    
        }

        size_t pos3 = pos2;
        while (pos3 < line.size() && is_digit(line[pos3])) {
            ++pos3;
        }
        if (pos3 > line.size()) {
            std::cout << "3" << std::endl;
            i++;
            tolerance--;
            continue;
        }
        if (pos3 <= line.size()-1 && !is_space(line[pos3])) {
            std::cout << "4" << std::endl;
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
            continue;
        }

        try {
            contents.push_back( {chapter_num, chapter_name, std::stoi(page)});
        } catch (...) {
            assert(false);
        }
        tolerance = max_tolerance;
        i++;
    }
    
    if (i < data.size() && contents.size() < 5) {
        contents.clear();
        goto tryagain;
    }
    
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