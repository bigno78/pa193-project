#pragma once

#include <string>
#include "utils.hpp"


std::string parse_NSCIB(std::vector<std::string>& data) {
	if (data[0].find("Rheinland Nederland B.V.") != std::string::npos) {
		std::vector<std::string> lines = {};
		for (size_t i = 11; i <= 16; i++) {
			if (data[i].empty()) {
				break;
			}
			lines.push_back(data[i]);
			trim(lines[lines.size()-1]);
		}
		std::string title = lines[0];
		for (size_t i = 1; i < lines.size(); i++) {
			append_line(title, lines[i]);
		}
		return title;
	}
	return "";
}

std::string parse_security_target(std::vector<std::string>& data) {
	for (int i = 0; i < 10; i++) {
		std::string line = data[i];
		std::transform(line.begin(), line.end(), line.begin(), [](unsigned char c) { return std::tolower(c); });
		std::vector<std::string> lines = {};
		if (line.find("security target") != std::string::npos) {
			if (i == 0) {
				size_t from = i + 1;
				if (data[from].empty()) {
					from++;
				}
				for (size_t j = from; j < i + 6; j++) {
					if (data[j].empty()) {
						break;
					}
					lines.push_back(data[j]);
					trim(lines[lines.size() - 1]);
				}
				std::string title = lines[0];
				for (size_t j = 1; j < lines.size(); j++) {
					append_line(title, lines[j]);
				}
				return title;
			}
			else {
				for (int j = i - 1; j > i - 5; j--) {
					if (j < 0 || data[j].empty()) {
						break;
					}
					lines.push_back(data[j]);
					trim(lines[lines.size() - 1]);
				}
				std::string title = lines[lines.size()-1];
				for (int j = lines.size()-2; j >= 0; j--) {
					append_line(title, lines[j]);
				}
				return title;
			}
		}
	}
	return "";
}

std::string parse_title(std::vector<std::string> &data) {
	if (std::string title = parse_NSCIB(data); !title.empty()) {
		return title;
	}
	if (std::string title = parse_security_target(data); !title.empty()) {
		return title;
	}
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
				//std::cout << candidet << std::endl;
				candidets.push_back(candidet);
			}
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

	for (size_t i = 0; i < candidets.size(); i++) {
		std::cout << candidets[i] << ": " << candidets[i].length() << std::endl;
	}
	//breaks stuff too
	bool (*lambda)(std::string & a, std::string & b) = [](std::string &a, std::string &b) {return a.length() < b.length(); };

	/**
	if (data[0].find("Rheinland Nederland B.V.") != std::string::npos) {
		lambda = [](std::string &a, std::string &b) {return a.length() > b.length(); };
	}
	**/
	auto it = (std::max_element(candidets.begin(), candidets.end(), lambda));
	if (it == candidets.end() || (*it).empty()) {
		std::string line = data[0];
		trim(line);
		return line;
	}
	
	return *it;
}