#include <string>

nlohmann::json parse_contents(std::vector<std::string>& data) {
    nlohmann::json toret = nlohmann::json::array({});
    std::vector<std::tuple<std::string, std::string, int>> contents = {};
    size_t i = 0;
    int tolerance = 10;
tryagain:
    std::string line = "";
    
    for (; i < data.size(); i++) {
        //maybe do content - 1022b - DONE
        //also the weird ones with introduction
        //and do line to lower - DONE
        //investigate 1107b (it should work?) - DONE
        //1126b - the second contents kills it - DONE
        //contents with no dots inbetween (NSCIB) - DONE
        //investigate anssi
        line = data[i];
        std::transform(line.begin(), line.end(), line.begin(),
                       [](unsigned char c) { return std::tolower(c); });
        if (line.find("content") != std::string::npos) {
            i++;
            while (is_empty_line(data[i])) {
                i++;
            }
            break;
        }
    }
    std::string chapter_num = "";
    std::string chapter_name = "";
    std::string page = "";
    while (1) {
        if (is_empty_line(data[i])) {
            i++;
            continue;
        }
        line = data[i];
        trim(line);
        size_t pos1 = 0;
        if (is_digit(line[0])) {
            pos1 = line.find(' ');
            if (pos1 == std::string::npos) {
                if (tolerance > 0) {
                    i++;
                    tolerance--;
                    continue;
                }
                break;
            }
            chapter_num = line.substr(0, pos1);
            trim(chapter_num);
        }
        
        auto pos2 = line.find('.', pos1);
        if (pos2 == std::string::npos) {
            //std::cout << "druhy" << std::endl;
            pos2 = line.find_first_of(" \n\t\r\v\f", pos1);
            //std::cout << line << std::endl;
            //std::cout << pos1 << std::endl;
            //std::cout << pos2 << " " << line[pos2] << std::endl;
            auto postemp =
                line.find_first_of("abcdefghijklmnopqrstuvwxyz", pos2);
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
        }
        chapter_name = line.substr(pos1, pos2 - pos1);
        trim(chapter_name);
        auto pos3 = line.find_first_of("0123456789", pos2);
        if (pos3 == std::string::npos) {
            if (tolerance > 0) {
                tolerance--;
                i++;
                continue;
            }
            break;
        }
        page = line.substr(pos3, line.length()-1);
        trim(page);
        contents.push_back(std::tuple<std::string, std::string, int>(chapter_num, chapter_name, std::stoi(page)));
        tolerance = 10;
        i++;
    }
    if (contents.size() < 10) {
        tolerance = 10;
        goto tryagain;
    }
    for (size_t i = 0; i < contents.size(); i++) {
        //std::cout << std::get<1>(contents[i]) << std::endl;
        toret.push_back(nlohmann::json::array({std::get<0>(contents[i]), std::get<1>(contents[i]), std::get<2>(contents[i])}));
    }
    return toret;
}