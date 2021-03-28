#pragma once

#include <istream>
#include <string>
#include <vector>

#include "json.hpp"

#include "bibliography.hpp"
#include "revisions.hpp"

inline nlohmann::json parse_document(std::istream& in) {
    // load all the data
    std::vector< std::string > data;
    std::string line;
    while( std::getline(in, line) ) {
        data.push_back( std::move(line) );
    }

    nlohmann::json out;
    //out["bibliography"] = parse_bibliography(data);
    
    RevisionsParser par(data);
    out["revisions"] = par.parse();

    return out;
}

