#include <string>

nlohmann::json parse_contents(std::vector<std::string>& data) {
    nlohmann::json toret = nlohmann::json::array({});
    std::vector<std::tuple<std::string, std::string, std::string>> contents = {};
    size_t i = 0;
    
    for (; i < 150; i++) {
        if (data[i].find("Contents") != std::string::npos) {
            std::cout << "here" << std::endl;
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
    std::string line = "";
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
                // std::cout << "prvni" << std::endl;
                break;
            }
            std::cout << pos1 << std::endl;
            chapter_num = line.substr(0, pos1);
            trim(chapter_num);
        }
        
        auto pos2 = line.find('.', pos1);
        if (pos2 == std::string::npos) {
            //std::cout << "druhy" << std::endl;
            break;
        }
        std::cout << pos2 << std::endl;
        chapter_name = line.substr(pos1, pos2 - pos1);
        trim(chapter_name);
        auto pos3 = line.find_first_of("0123456789", pos2);
        if (pos3 == std::string::npos) {
            break;
        }
        page = line.substr(pos3, line.length()-1);
        trim(page);
        contents.push_back(std::tuple<std::string, std::string, std::string>(chapter_num, chapter_name, page));
        i++;
    }
    for (size_t i = 0; i < contents.size(); i++) {
        //std::cout << std::get<1>(contents[i]) << std::endl;
        toret.push_back(nlohmann::json::array({contents[i]}));
    }
    return toret;
}