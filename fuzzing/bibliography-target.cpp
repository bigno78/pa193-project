#include <string>
#include <sstream>

#include "../src/bibliography.hpp"


extern "C" int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    std::string s( (const char*) Data, Size);
    std::stringstream ss(s);
    std::vector<std::string> data;
    std::string line;
    while( std::getline(ss, line) ) {
        data.push_back( std::move(line) );
    }
    parse_bibliography(data);
    return 0;
}