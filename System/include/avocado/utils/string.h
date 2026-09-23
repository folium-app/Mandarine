#pragma once

#include <string>
#include <string_view>
#include <vector>

const std::string_view trim(std::string_view);

// TODO: (jarrodnorwell) continue with rewriting the rest of this file
bool endsWith(const std::string &a, const std::string &b);
std::string replaceAll(const std::string &str, const std::string &find, const std::string &replace);