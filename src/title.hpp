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
			candidets.push_back(candidet);
			current.clear();
			continue;
		}
		current.push_back(data[i]);
	}
	for (size_t i = 0; i < candidets.size(); i++) {
		std::vector<Column> temp = split_line_into_columns(candidets[i]);
		if (temp.size() > 1) {
			candidets.erase(candidets.begin() + i);
			i--;
		}
	}
	auto it = (std::max_element(candidets.begin(), candidets.end(), [](std::string a, std::string b) {return a.length() < b.length(); }));
	if (it == candidets.end()) {
		return "";
	}
	return *it;
}