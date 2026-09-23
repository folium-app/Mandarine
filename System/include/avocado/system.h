#pragma once
#include "constants.h"
#include "cpu/cpu.h"
#include "device/cache_control.h"
#include "device/cdrom/cdrom.h"
#include "device/controller/controller.h"
#include "device/dma/dma.h"
#include "device/expansion2.h"
#include "device/gpu/gpu.h"
#include "device/interrupt.h"
#include "device/mdec/mdec.h"
#include "device/memory_control.h"
#include "device/ram_control.h"
#include "device/serial.h"
#include "device/spu/spu.h"
#include "device/timer.h"
#include "utils/macros.h"
#include "utils/timing.h"

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <ranges>
#include <variant>
#include <vector>

/**
 * NOTE:
 * Build flags are configured with Premake5 build system
 */

/**
 * #define ENABLE_IO_LOG
 * Switch --enable-io-log
 * Default: false
 *
 * Enables IO access buffer log
 */

/**
 * #define ENABLE_BIOS_HOOKS
 * Switch --enable-bios-hooks
 * Default: false
 *
 * Enables BIOS syscall hooking/logging
 */

namespace bios {
struct Function;
}

struct System {
    enum class State {
        halted,  // reset to run again
        stop,    // after reset
        pause,   // if debugger is attached
        run      // normal state
    } state{State::stop};
    
    std::vector<uint8_t> bios, ram, expansion_region_1, scratchpad;

    bool debugOutput = true;  // Print BIOS logs
    bool biosLoaded = false;

    uint64_t cycles;

    // Devices
    std::unique_ptr<mips::CPU> cpu;

    std::unique_ptr<CacheControl> cacheControl;
    std::unique_ptr<device::cdrom::CDROM> cdrom;
    std::unique_ptr<device::controller::Controller> controller;
    std::unique_ptr<device::dma::DMA> dma;
    std::unique_ptr<Expansion2> expansion2;
    std::unique_ptr<gpu::GPU> gpu;
    std::unique_ptr<Interrupt> interrupt;
    std::unique_ptr<mdec::MDEC> mdec;
    std::unique_ptr<MemoryControl> memoryControl;
    std::unique_ptr<RamControl> ramControl;
    std::unique_ptr<Serial> serial;
    std::unique_ptr<spu::SPU> spu;
    
    std::vector<std::unique_ptr<device::timer::Timer>> timers;
    
    template <typename T>
    constexpr T fast_read(const uint8_t*, uint32_t);
    
    template <typename T>
    constexpr void fast_write(uint8_t*, uint32_t, T);
    
    template <typename T, typename Peripheral>
    constexpr std::optional<T> read_peripheral(const Peripheral&, uint32_t);
    
    template <typename T, typename Peripheral>
    constexpr void write_peripheral(Peripheral&, uint32_t, T);
    
    template<typename T, typename Peripheral>
    constexpr std::optional<T> read_io(uint32_t, uint32_t, uint32_t, const Peripheral&);
    
    template<typename T, typename Peripheral>
    constexpr bool write_io(uint32_t, T, uint32_t, uint32_t, Peripheral&);
    
    template <typename T = uint32_t>
    T read(uint32_t);
    
    template <typename T = uint32_t>
    void write(uint32_t, T);
    
    constexpr void step(int /* count */ = 1);
    
    using PeripheralTypes = std::variant<device::dma::DMA*, Expansion2*, gpu::GPU*, Interrupt*, mdec::MDEC*, MemoryControl*, RamControl*, CacheControl*, Serial*>;
    
    template <typename Peripheral>
    constexpr void reset_peripheral(const Peripheral&);
    
    constexpr bool reset(bool /* soft */ = true);
    
    bool load(const std::string& /* path */);
    bool load(const std::vector<uint8_t>& /* data */, bool /* is_exe */ = false);
    
    enum class HandleType { BIOS, SYSTEM_CALL };
    void handle(HandleType);

    // TODO: (jarrodnorwell) continue with rewriting the rest of this file
    System();
    void printFunctionInfo(const char*, bios::Function);
    void emulateFrame();
    bool isSystemReady();

    // Helpers
    std::string biosPath;
    int biosLog = 0;
    bool printStackTrace = false;
    void dumpRam();

#ifdef ENABLE_IO_LOG
    struct IO_LOG_ENTRY {
        enum class MODE { READ, WRITE } mode;

        uint32_t size;
        uint32_t addr;
        uint32_t data;
        uint32_t pc;
    };

    std::vector<IO_LOG_ENTRY> ioLogList;
#endif

    template <class Archive>
    void serialize(Archive& ar) {
        ar(*cpu);
        ar(*gpu);
        ar(*spu);
        ar(*interrupt);
        ar(*dma);
        ar(*cdrom);
        ar(*memoryControl);
        ar(*cacheControl);
        ar(*serial);
        ar(*mdec);
        ar(*controller);
        for (auto i : std::views::iota(0, 3))
            ar(*timers.at(i));

        ar(ram);
        ar(scratchpad);
    }
};
