#pragma once

#include <set>

#include "json.hpp"
#include "types.hpp"

void pprint(const nlohmann::json& js, const std::set<SectionType>& sections, size_t max_width);

void pprint_title(const nlohmann::json& js, size_t max_width);

void pprint_bibliography(const nlohmann::json& js, size_t max_width);

void pprint_revisions(const nlohmann::json& js, size_t max_width);

void pprint_versions(const nlohmann::json& js, size_t max_width);

void pprint_contents(const nlohmann::json& js, size_t max_width);
