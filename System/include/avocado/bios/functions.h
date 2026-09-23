#pragma once

#include <cstdint>
#include <functional>
#include <string_view>
#include <unordered_map>
#include <vector>

struct System;
namespace bios {

struct Function {
    struct Argument {
        enum class Type {
            CHARACTER,
            CHARACTER_POINTER,
            POINTER,
            INTEGER
        };

        std::string_view name;
        Type type;
    };

    std::vector<Argument> arguments;
    std::function<bool(System*)> callback;
    std::string_view name;

    Function(std::function<bool(System*)>, std::string_view);
    Function(std::string_view, std::function<bool(System*)> = nullptr);
};

using AFunction = Function;
using BFunction = Function;
using CFunction = Function;
using SCFunction = Function;

extern const std::unordered_map<uint8_t, AFunction> A;
extern const std::unordered_map<uint8_t, BFunction> B;
extern const std::unordered_map<uint8_t, CFunction> C;
extern const std::unordered_map<uint8_t, SCFunction> SYSTEM_CALL;

extern const std::array<std::unordered_map<uint8_t, Function>, 3> tables;

};