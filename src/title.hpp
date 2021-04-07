#pragma once

#include <string>
#include "utils.hpp"

std::string parse_title(std::vector<std::string> data) {
	std::vector<std::string> candidets = {};
	std::vector<std::string> current = {};
	for (size_t i = 0; i < 50; i++) {
		std::string line = data[i];
		trim(line);
		if (line.length() > 60) {
			for (size_t j = i + 1; j < 50; j++) {
				if (!is_empty_line(data[j])) {
					continue;
				}
				i = j+1;
				current.clear();
				break;
			}
		}
		if (is_empty_line(data[i]) && !current.empty()) {
			std::string candidet = current[0];
			trim(candidet);
			for (size_t j = 1; j < current.size(); j++) {
				trim(current[j]);
				append_line(candidet, current[j]);
			}
			if (!candidet.empty()) {
				candidets.push_back(candidet);
			}
			current.clear();
			continue;
		}
		current.push_back(data[i]);
	}
	for (size_t i = 0; i < candidets.size(); i++) {
		std::vector<Column> temp = split_line_into_columns(candidets[i]);
		//the empty part was creating issues before
		//if(candidets[i].empty() || temp.size() > 1) {
		if (temp.size() > 1) {
			candidets.erase(candidets.begin() + i);
			i--;
		}
	}

	for (size_t i = 0; i < candidets.size(); i++) {
		std::cout << candidets[i] << ": " << candidets[i].length() << std::endl;
	}

	bool (*lambda)(std::string & a, std::string & b) = [](std::string &a, std::string &b) {return a.length() < b.length(); };

	if (data[0].find("Rheinland Nederland B.V.") != std::string::npos) {
		lambda = [](std::string &a, std::string &b) {return a.length() > b.length(); };
	}

	auto it = (std::max_element(candidets.begin(), candidets.end(), lambda));
	if (it == candidets.end()) {
		return "";
	}
	
	if ((*it).empty()) {
		std::string line = data[0];
		trim(line);
		return line;
	}
	
	return *it;
}