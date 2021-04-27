#pragma once

#include <string>
#include <iostream>

#include "utils.hpp"


//concatenates all strings in vector into a single string
std::string multiple_lines_into_one(std::vector<std::string>& lines) {
    std::string title = lines[0];
    for (size_t i = 1; i < lines.size(); i++) {
        append_line(title, lines[i]);
    }
    return title;
}

//concatenates all strings in vector into a single string starting from the last one
std::string multiple_lines_into_one_reverse(std::vector<std::string>& lines) {
    std::string title = lines[lines.size() - 1];
    for (int j = static_cast<int>(lines.size() - 2); j >= 0; j--) {
        append_line(title, lines[j]);
    }
    return title;
}

//creates new candidate for title out of current
void add_new_candidate(std::vector<std::string>& candidates, std::vector<std::string>& current) {
    std::string candidet = current[0];
    trim(candidet);
    for (size_t j = 1; j < current.size(); j++) {
        trim(current[j]);
        append_line(candidet, current[j]);
    }
    if (!candidet.empty()) {
        candidates.push_back(candidet);
    }
    current.clear();
}

//function for parsing the type of certificate which has Rheinland Nederland B.V. in the first line
std::string parse_NSCIB(const std::vector<std::string>& data) {
	if (data[0].find("Rheinland Nederland B.V.") != std::string::npos) {
		std::vector<std::string> lines = {};
        if (data.size() <= 17) {
            return "";
		}
		//for types of certificates containing Rheinland Nederland B.V. in the first line
		//it is typical that title is on line 11 (or 11 + some more if title is multi line)
		for (size_t i = 11; i <= 16; i++) {
			if (data[i].empty()) {
				break;
			}
			lines.push_back(data[i]);
			trim(lines[lines.size()-1]);
		}
        return multiple_lines_into_one(lines);
	}
	return "";
}

//function for parsing certificates which have security target somewhere in the first 10 lines
std::string parse_security_target(const std::vector<std::string>& data) {
    if (data.size() < 10) {
        return "";
	}
	for (size_t i = 0; i < 10; i++) {
		std::string line = data[i];
        to_lower(line);
		std::vector<std::string> lines = {};
		if (line.find("security target") != std::string::npos) {
			//case where title is after "security target"
			if (i == 0) {
				size_t from = i + 1;
				if (data[from].empty()) {
					from++;
				}
				//lets assume title does not have more than 10 lines
				for (size_t j = from; j < i + 10; j++) {
					if (data[j].empty() || data[j].find(".....") != std::string::npos) {
						break;
					}
					lines.push_back(data[j]);
					trim(lines[lines.size() - 1]);
				}
				if (lines.empty()) {
					return "";
				}
				return multiple_lines_into_one(lines);
			}
			//case where title is after "security target"
			else {
				int from = 1;
				if (data[static_cast<size_t>(i - 1)].empty()) {
					from++;
				}
				//lets assume title does not have more than 10 lines
				for (int j = static_cast<int>(i - from); j > static_cast<int>(i - 10); j--) {
					if (j < 0 || data[j].empty() || data[j].find(".....") != std::string::npos) {
						break;
					}
					lines.push_back(data[j]);
					trim(lines[lines.size() - 1]);
				}
				if (lines.empty()) {
					return "";
				}
				return multiple_lines_into_one_reverse(lines);
			}
		}
	}
	return "";
}

std::string parse_title(const std::vector<std::string> &data) {
    if (data.size() == 0) {
        return "";
	}
	if (std::string title = parse_NSCIB(data); !title.empty()) {
		return title;
	}
	if (std::string title = parse_security_target(data); !title.empty()) {
		return title;
	}
	std::vector<std::string> candidates = {};
	std::vector<std::string> current = {};
	//number of lines in which we assume we will find title
    size_t length = 50;
    if (length > data.size()) {
        length = data.size();
	}
	for (size_t i = 0; i < length; i++) {
		std::string line = data[i];
		trim(line);
		//title lines are short - skip long lines
		if (line.length() > 60) {
			for (size_t j = i + 1; j < length; j++) {
				if (!is_empty_line(data[j])) {
					continue;
				}
				i = j+1;
				current.clear();
				break;
			}
		}
		if (i >= data.size()) {
			break;
		}
		std::cout << data.size() << " " << i << std::endl;
		if (is_empty_line(data[i]) && !current.empty()) {
            add_new_candidate(candidates, current);
			continue;
		}
		current.push_back(data[i]);
	}
	//lines with multiple collums are not containing title
	for (size_t i = 0; i < candidates.size(); i++) {
		std::vector<Column> temp = split_line_into_columns(candidates[i]);
		if (temp.size() > 1) {
			candidates.erase(candidates.begin() + i);
			i--;
		}
	}

	//the longest candidet is assumed to be title
    auto it = (std::max_element(candidates.begin(), candidates.end(),
                                    [](std::string& a, std::string& b) { return a.length() < b.length(); }));
	if (it == candidates.end() || (*it).empty()) {
        if (data.empty()) {
            return "";
		}
		//if all else fails, assume that first line is title
		std::string line = data[0];
		trim(line);
		return line;
	}
	
	return *it;
}