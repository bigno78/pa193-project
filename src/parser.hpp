#pragma once

#include <istream>
#include <string>
#include <vector>
#include <set>

#include "json.hpp"

#include "bibliography.hpp"
#include "revisions.hpp"
#include "title.hpp"
#include "toc.hpp"
#include "versions.hpp"
#include "contents.hpp"
#include "types.hpp"

inline nlohmann::json parse_document(std::istream& in, const std::set<SectionType>& sections) {
    std::vector< std::string > data;
    std::string line;
    while( std::getline(in, line) ) {
        data.push_back( std::move(line) );
    }

    nlohmann::json out;

    for (auto sect : sections) {
        switch (sect) {
            case SectionType::bibliography:
                out["bibliography"] = parse_bibliography(data);
                break;
            case SectionType::contents:
                out["table_of_contents"] = parse_contents(data);
                break;
            case SectionType::revisions:
                out["revisions"] = parse_revisions(data);
                break;
            case SectionType::title:
                out["title"] = parse_title(data);
                break;
            case SectionType::versions:
                out["versions"] = parse_versions(data);
                break;
        }
    }

    return out;
}

