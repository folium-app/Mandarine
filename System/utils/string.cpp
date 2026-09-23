#include "utils/string.h"

#include <cstdarg>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <string_view>
#include <string>

const std::string_view trim(std::string_view input) {
    while (not input.empty() and std::isspace(input.front()))
        input.remove_prefix(1);

    while (not input.empty() and std::isspace(input.back()))
        input.remove_suffix(1);

    return input;
}

// TODO: (jarrodnorwell) continue with rewriting the rest of this file
bool endsWith(const std::string& a, const std::string& b) {
    if (a.length() >= b.length()) {
        return a.compare(a.length() - b.length(), b.length(), b) == 0;
    } else {
        return false;
    }
}

std::string replaceAll(const std::string& str, const std::string& find, const std::string& replace) {
    std::string s = str;
    size_t pos = 0;
    while ((pos = s.find(find, pos)) != std::string::npos) {
        s.replace(pos, find.length(), replace);
        pos += replace.length();
    }
    return s;
}
