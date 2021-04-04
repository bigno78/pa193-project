#pragma once

#include <string>
#include "utils.hpp"

std::string parse_title(std::vector<std::string> data) {
	std::vector<std::string> candidets = {};
	std::vector<std::string> current = {};
	for (size_t i = 0; i < 50; i++) {
		if (data[i].length() > 60) {
			for (size_t j = i + 1; j < 50; j++) {
				if (!is_empty_line(data[j])) {
					continue;
				}
				i = j;
				continue;
			}
		}
		if (is_empty_line(data[i])) {
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
	return *(std::max_element(candidets.begin(), candidets.end(), [](std::string a, std::string b) {return a.length() < b.length(); }));
}