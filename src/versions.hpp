#pragma once

#include <vector>
#include <string>

#include "json.hpp"

nlohmann::json parse_versions(const std::vector<std::string>& data);
