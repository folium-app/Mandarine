#pragma once

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/cpu/opcode.h"

#else

#include "cpu/opcode.h"

#endif // TARGET_OS_IPHONE

#include <string>

namespace debugger {
struct Instruction {
    mips::Opcode opcode;
    std::string mnemonic;
    std::string parameters;
    bool valid = true;

    bool isBranch() const;
};

extern bool mapRegisterNames;
extern bool followPC;
std::string reg(unsigned int n);
Instruction decodeInstruction(mips::Opcode& i);
};  // namespace debugger
