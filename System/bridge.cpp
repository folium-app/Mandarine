//
//  bridge.cpp
//  Mandarine
//
//  Created by Jarrod Norwell on 14/6/2026.
//

#include "avocado/bridge.h"
#include "avocado/config.h"
#include "avocado/system.h"
#include "avocado/system_tools.h"
#include "avocado/input/input_manager.h"
#include "avocado/memory_card/card_formats.h"
#include "avocado/sound/sound.h"

#import "Mandarine-Swift.h"
using namespace Mandarine;

#include <_printf.h>
#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <sstream>
#include <thread>

#include <fmt/core.h>
#include <fmt/format.h>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

bool fileExists(const std::string &path) {
    return std::filesystem::exists(path);
}

std::vector<uint8_t> getFileContents(const std::string &path) {
    std::vector<uint8_t> contents;

    FILE *f = fopen(path.c_str(), "rb");
    if (!f) return contents;

    fseek(f, 0, SEEK_END);
    int filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    contents.resize(filesize);
    fread(&contents[0], 1, filesize, f);

    fclose(f);
    return contents;
}

bool putFileContents(const std::string &name, const std::vector<unsigned char> &contents) {
    FILE *f = fopen(name.c_str(), "wb");
    if (!f) return false;

    fwrite(&contents[0], 1, contents.size(), f);

    fclose(f);

    return true;
}
bool writeToDisc(const std::string& name, const std::vector<uint8_t>& contents) { return putFileContents(name, contents); };

bool putFileContents(const std::string &path, const std::string contents) {
    FILE *f = fopen(path.c_str(), "wb");
    if (!f) return false;

    fwrite(&contents[0], 1, contents.size(), f);

    fclose(f);

    return true;
}

std::string getFileContentsAsString(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Could not open file");
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

size_t getFileSize(const std::string &path) {
    return std::filesystem::file_size(path);
}

class GCInputManager : public InputManager {
public:
    GCInputManager() {}
    
    void press(std::string button, int index) {
        state[fmt::format("controller/{}/{}", index, button).c_str()] = AnalogValue(true);
    }
    
    void release(std::string button, int index) {
        state[fmt::format("controller/{}/{}", index, button).c_str()] = AnalogValue(false);
    }
    
    void drag(std::string button, int index, uint8_t value) {
        state[fmt::format("controller/{}/{}", index, button).c_str()] = AnalogValue(value);
    }
};

namespace Sound {
std::deque<uint16_t> buffer;
std::mutex audioMutex;
};  // namespace Sound

namespace {
SDL_AudioDeviceID deviceID = 0;
SDL_AudioStream* stream = nullptr;

void audioCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
    (void)userdata;
    
    // additional_amount is byte count
    if (additional_amount <= 0)
        return;
    
    std::vector<uint8_t> data(additional_amount);
    
    std::unique_lock<std::mutex> lock(Sound::audioMutex);
    
    size_t samples_available = Sound::buffer.size();
    size_t samples_needed = additional_amount / sizeof(int16_t);
    
    size_t samples_to_copy = std::min(samples_available, samples_needed);
    
    for (size_t i = 0; i < samples_to_copy; i++) {
        int16_t sample = Sound::buffer.front();
        Sound::buffer.pop_front();
        
        data.at(i * 2) = (uint8_t)sample & 0xFF;
        data.at(i * 2 + 1) = (uint8_t)(sample >> 8) & 0xFF;
    }
    
    SDL_PutAudioStreamData(stream, data.data(), additional_amount);
}
}  // namespace

void Sound::init() {
    SDL_SetMainReady();
    SDL_Init(SDL_INIT_AUDIO);
    
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.channels = 2;
    spec.freq = 44100;
    spec.format = SDL_AUDIO_S16;
    
    deviceID = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    stream = SDL_OpenAudioDeviceStream(deviceID, &spec, &audioCallback, nullptr);
    
    SDL_ResumeAudioStreamDevice(stream);
}

void Sound::play() {
    SDL_ResumeAudioStreamDevice(stream);
}

void Sound::stop() {
    SDL_PauseAudioStreamDevice(stream);
}

void Sound::close() {
    SDL_DestroyAudioStream(stream);
}

void Sound::clearBuffer() {
    buffer.clear();
}

double limitFramerate(bool framelimiter, bool ntsc) {
    static double timeToSkip = 0;
    static double counterFrequency = (double)SDL_GetPerformanceFrequency();
    static double startTime = SDL_GetPerformanceCounter() / counterFrequency;
    static double fpsTime = 0.0;
    static double fps = 0;
    static int deltaFrames = 0;

    double currentTime = SDL_GetPerformanceCounter() / counterFrequency;
    double deltaTime = currentTime - startTime;

    double frameTime = ntsc ? (1.0 / timing::NTSC_FRAMERATE) : (1.0 / timing::PAL_FRAMERATE);

    if (framelimiter && deltaTime < frameTime) {
        // If deltaTime was shorter than frameTime - spin
        if (deltaTime < frameTime - timeToSkip) {
            while (deltaTime < frameTime - timeToSkip) {  // calculate real difference
                SDL_Delay(1);

                currentTime = SDL_GetPerformanceCounter() / counterFrequency;
                deltaTime = currentTime - startTime;
            }
            timeToSkip -= (frameTime - deltaTime);
            if (timeToSkip < 0.0) timeToSkip = 0.0;
        } else {  // Else - accumulate
            timeToSkip += deltaTime - frameTime;
        }
    }

    startTime = currentTime;
    fpsTime += deltaTime;
    deltaFrames++;

    if (fpsTime > 0.25f) {
        fps = (double)deltaFrames / fpsTime;
        deltaFrames = 0;
        fpsTime = 0.0;
    }

    return fps;
}


struct MandarineCPP {
    MandarineCommon mandarineCommon{MandarineCommon::init()};
    MandarineSystem mandarineSystem{MandarineSystem::init()};
    
    std::unique_ptr<GCInputManager> controller;
    std::unique_ptr<System> system;
    
    std::filesystem::path mandarine_path, memory_cards_path, system_data_path;
    
    std::jthread thread;
    std::mutex mutex;
    std::condition_variable_any cv;
} m_cntnr;

void mandarine::print_about(void) {
    printf("Welcome to Mandarine\n");
    printf("PlayStation 1 emulator based on Avocado\n");
}


std::string mandarine::disc_identifier(std::string path) {
    const int blockSize = 1024 * 1024; // Read in 1MB blocks
    std::ifstream binFile(path, std::ios::binary);
    if (!binFile.is_open()) {
        throw std::runtime_error("Failed to open the .bin file");
    }

    binFile.seekg(0, std::ios::end);
    size_t fileSize = binFile.tellg();
    binFile.seekg(0, std::ios::beg);

    std::vector<char> buffer(blockSize);
    size_t bytesRead = 0;

    while (bytesRead < fileSize) {
        size_t readSize = std::min(blockSize, (int)(fileSize - bytesRead));
        binFile.read(buffer.data(), readSize);

        std::string content(buffer.begin(), buffer.begin() + readSize);
        size_t bootPos = content.find("BOOT");

        if (bootPos != std::string::npos) {
            size_t start = content.find(':', bootPos);
            if (start == std::string::npos) {
                throw std::runtime_error("Invalid BOOT line format");
            }

            ++start; // move past ':'

            // Skip optional slash/backslash and whitespace
            while (start < content.size() &&
                   (content[start] == '\\' ||
                    content[start] == '/' ||
                    std::isspace(static_cast<unsigned char>(content[start]))))
            {
                ++start;
            }

            size_t end = content.find(';', start);
            if (end == std::string::npos) {
                throw std::runtime_error("Invalid BOOT line format");
            }

            return content.substr(start, end - start);
        }

        bytesRead += readSize;
    }

    throw std::runtime_error("BOOT line not found in the file");
}


void mandarine::initialize_paths(void) {
    auto mandarineDirectoryURL{m_cntnr.mandarineCommon.getMandarineDirectoryURL()};
    if (mandarineDirectoryURL.isSome()) {
        auto mandarine_path{std::filesystem::path{mandarineDirectoryURL.get()}};
        
        m_cntnr.mandarine_path = mandarine_path;
        m_cntnr.memory_cards_path = mandarine_path / "memory_cards";
        m_cntnr.system_data_path = mandarine_path / "system_data";
    }
    
    auto memory_card = [](std::string index) -> std::filesystem::path {
        auto system_data_path = m_cntnr.memory_cards_path / "";
        return system_data_path.string() + "memory_card_" + index + ".mcr";
    };
    
    auto bios = m_cntnr.system_data_path / "bios.bin";
    
    auto memory_card_0 = memory_card("0");
    auto memory_card_1 = memory_card("1");
    
    config.bios = bios.string();
    config.memoryCard[0].path = memory_card_0.string();
    config.memoryCard[1].path = memory_card_1.string();
}

void mandarine::initialize_memory_cards(void) {
    auto memory_card_0{std::filesystem::path{config.memoryCard[0].path}};
    auto memory_card_1{std::filesystem::path{config.memoryCard[1].path}};
    
    if (!std::filesystem::exists(memory_card_0)) {
        std::array<uint8_t, memory_card::MEMCARD_SIZE> data;
        memory_card::format(data);
        memory_card::save(data, memory_card_0.string());
    }
    
    if (!std::filesystem::exists(memory_card_1)) {
        std::array<uint8_t, memory_card::MEMCARD_SIZE> data;
        memory_card::format(data);
        memory_card::save(data, memory_card_1.string());
    }
}

void mandarine::initialize_system(void) {
    m_cntnr.controller.reset();
    Sound::close();
    
    if (m_cntnr.system == nullptr) {
        m_cntnr.system = system_tools::hardReset();
        m_cntnr.system->state = System::State::stop;
    } else {
        m_cntnr.system->reset();
        m_cntnr.system->state = System::State::stop;
    }
    
    Sound::init();
    m_cntnr.controller = std::make_unique<GCInputManager>();
    InputManager::setInstance(m_cntnr.controller.get());
}


void mandarine::destroy_system(void) {
    mandarine::initialize_system();
}


void mandarine::insert_disc(std::string path) {
    system_tools::loadFile(m_cntnr.system, path);
}


bool mandarine::is_paused(bool change, bool set_paused) {
    if (change)
        m_cntnr.system->state = set_paused ? System::State::pause : System::State::run;
    return m_cntnr.system->state == System::State::pause;
}

bool mandarine::is_running(bool change, bool set_running) {
    if (change)
        m_cntnr.system->state = set_running ? System::State::run : System::State::stop;
    return m_cntnr.system->state == System::State::run;
}


void mandarine::start(void) {
    m_cntnr.system->state = System::State::run;
    m_cntnr.thread = std::jthread([&](std::stop_token token) {
        using namespace std::chrono;
        
        while (!token.stop_requested()) {
            switch (m_cntnr.system->state) {
                case System::State::halted:
                case System::State::stop:
                case System::State::pause:
                    break;
                case System::State::run:
                    m_cntnr.system->gpu->clear();
                    m_cntnr.system->controller->update();
                    
                    m_cntnr.system->emulateFrame();
                    
                    if (m_cntnr.system->gpu->gp1_08.colorDepth == gpu::GP1_08::ColorDepth::bit24)
                        mandarine::callback_24bit(mandarine::context, m_cntnr.system->gpu->vram.data());
                    else
                        mandarine::callback_15bit(mandarine::context, m_cntnr.system->gpu->vram.data());
                    
                    limitFramerate(true, m_cntnr.system->gpu->isNtsc());
                    break;
            }
        }
    });
}

void mandarine::stop(void) {
    m_cntnr.thread.request_stop();
    if (m_cntnr.thread.joinable())
        m_cntnr.thread.join();
    
    system_tools::saveMemoryCard(m_cntnr.system, 0, true);
    system_tools::saveMemoryCard(m_cntnr.system, 1, true);
    
    mandarine::destroy_system();
}


int16_t mandarine::framebuffer_start_x(void) {
    return m_cntnr.system->gpu->displayAreaStartX;
}

int16_t mandarine::framebuffer_start_y(void) {
    return m_cntnr.system->gpu->displayAreaStartY;
}

int mandarine::framebuffer_height(void) {
    return m_cntnr.system->gpu->gp1_08.getVerticalResoulution();
}

int mandarine::framebuffer_width(void) {
    return m_cntnr.system->gpu->gp1_08.getHorizontalResoulution();
}


void mandarine::video_buffer_callback_15bit(mandarine::VideoBufferCallback15Bit callback) {
    mandarine::callback_15bit = callback;
}

void mandarine::video_buffer_callback_24bit(mandarine::VideoBufferCallback24Bit callback) {
    mandarine::callback_24bit = callback;
}


void mandarine::press_button(std::string button, int index) {
    m_cntnr.controller->press(button, index);
}

void mandarine::release_button(std::string button, int index) {
    m_cntnr.controller->release(button, index);
}

void mandarine::drag_thumbstick(std::string thumbstick, uint8_t value) {
    m_cntnr.controller->drag(thumbstick, 1, value);
}


void mandarine::set_context(void* context) {
    mandarine::context = context;
}

void mandarine::set_setting(mandarine::SETTING setting, bool value) {
    switch (setting) {
        case SETTING::WIDESCREEN:
            config.options.graphics.widescreen = value;
            break;
        case SETTING::FORCE_WIDESCREEN:
            config.options.graphics.forceWidescreen = value;
            break;
        case SETTING::VSYNC:
            config.options.graphics.vsync = value;
            break;
        case SETTING::FORCE_NTSC:
            config.options.graphics.forceNtsc = value;
            break;
        case SETTING::NATIVE_TEXTURE_FORMAT:
            config.options.graphics.nativeTextureFormat = value;
            break;
        case SETTING::SOUND_ENABLED:
            config.options.sound.enabled = value;
            if (config.options.sound.enabled)
                Sound::play();
            else
                Sound::stop();
            break;
        case SETTING::PRESERVE_STATE:
            config.options.emulator.preserveState = value;
            break;
        case SETTING::TIME_TRAVEL:
            config.options.emulator.timeTravel = value;
            break;
        case SETTING::EXTENDED_MEMORY:
            config.options.system.ram8mb = value;
            break;
        case SETTING::LOG_BIOS:
            config.debug.log.bios = value;
            break;
        case SETTING::LOG_CDROM:
            config.debug.log.cdrom = value;
            break;
        case SETTING::LOG_CONTROLLER:
            config.debug.log.controller = value;
            break;
        case SETTING::LOG_DMA:
            config.debug.log.dma = value;
            break;
        case SETTING::LOG_GPU:
            config.debug.log.gpu = value;
            break;
        case SETTING::LOG_GTE:
            config.debug.log.gte = value;
            break;
        case SETTING::LOG_MDEC:
            config.debug.log.mdec = value;
            break;
        case SETTING::LOG_MEMORY_CARD:
            config.debug.log.memoryCard = value;
            break;
        case SETTING::LOG_MEMORY_CONTROL:
            config.debug.log.memoryControl = value;
            break;
        case SETTING::LOG_SPU:
            config.debug.log.spu = value;
            break;
        case SETTING::LOG_SYSTEM:
            config.debug.log.system = value;
            break;
    }
}


namespace chd_reader {
static void set_error(
                      CHDReader *reader,
                      const char *message
                      ) {
    if (!reader) {
        return;
    }
    
    snprintf(
             reader->error,
             sizeof(reader->error),
             "%s",
             message ? message : "Unknown CHD error"
             );
}

CHDReader *chd_reader_open(const char *path)
{
    if (!path) {
        return NULL;
    }
    
    CHDReader *reader =
    (CHDReader *)calloc(1, sizeof(CHDReader));
    
    if (!reader) {
        return NULL;
    }
    
    chd_error error = chd_open(
                               path,
                               CHD_OPEN_READ,
                               NULL,
                               &reader->chd
                               );
    
    if (error != CHDERR_NONE) {
        set_error(
                  reader,
                  chd_error_string(error)
                  );
        
        free(reader);
        return NULL;
    }
    
    const chd_header *header =
    chd_get_header(reader->chd);
    
    if (!header) {
        set_error(reader, "Unable to read CHD header");
        chd_close(reader->chd);
        free(reader);
        return NULL;
    }
    
    reader->hunkBytes = header->hunkbytes;
    
    /*
     * PlayStation CD-ROM data normally uses:
     *
     * 2352 bytes/frame
     *
     * Some CHDs contain:
     *
     * 2448 bytes/frame
     *
     * where the additional 96 bytes are subchannel data.
     *
     * libchdr exposes unitbytes specifically for this.
     */
    if (header->unitbytes == 2352 ||
        header->unitbytes == 2448) {
        
        reader->frameBytes = header->unitbytes;
        
    } else if (
               header->hunkbytes % 2448 == 0
               ) {
                   
                   reader->frameBytes = 2448;
                   
               } else if (
                          header->hunkbytes % 2352 == 0
                          ) {
                              
                              reader->frameBytes = 2352;
                              
                          } else {
                              
                              set_error(
                                        reader,
                                        "Unsupported CHD CD frame size"
                                        );
                              
                              chd_close(reader->chd);
                              free(reader);
                              return NULL;
                          }
    
    reader->hunkBuffer =
    (uint8_t *)malloc(reader->hunkBytes);
    
    if (!reader->hunkBuffer) {
        set_error(reader, "Unable to allocate CHD hunk buffer");
        
        chd_close(reader->chd);
        free(reader);
        return NULL;
    }
    
    reader->hasCachedHunk = 0;
    
    return reader;
}

void chd_reader_close(CHDReader *reader)
{
    if (!reader) {
        return;
    }
    
    if (reader->chd) {
        chd_close(reader->chd);
    }
    
    free(reader->hunkBuffer);
    free(reader);
}

uint64_t chd_reader_logical_bytes(CHDReader *reader)
{
    if (!reader || !reader->chd) {
        return 0;
    }
    
    const chd_header *header =
    chd_get_header(reader->chd);
    
    if (!header) {
        return 0;
    }
    
    return header->logicalbytes;
}

uint32_t chd_reader_frame_bytes(CHDReader *reader)
{
    if (!reader) {
        return 0;
    }
    
    return reader->frameBytes;
}

int chd_reader_read_frame(
                          CHDReader *reader,
                          uint32_t frame,
                          uint8_t *output
                          ) {
    if (!reader ||
        !reader->chd ||
        !output) {
        
        return 0;
    }
    
    const uint64_t byteOffset =
    (uint64_t)frame * reader->frameBytes;
    
    const uint32_t hunk =
    (uint32_t)(
               byteOffset / reader->hunkBytes
               );
    
    const uint32_t offset =
    (uint32_t)(
               byteOffset % reader->hunkBytes
               );
    
    /*
     * A CD frame should never cross a CHD hunk boundary
     * for normal CHDs, but handle it anyway.
     */
    if (offset + reader->frameBytes <= reader->hunkBytes) {
        
        if (!reader->hasCachedHunk ||
            reader->cachedHunk != hunk) {
            
            chd_error error =
            chd_read(
                     reader->chd,
                     hunk,
                     reader->hunkBuffer
                     );
            
            if (error != CHDERR_NONE) {
                set_error(
                          reader,
                          chd_error_string(error)
                          );
                
                return 0;
            }
            
            reader->cachedHunk = hunk;
            reader->hasCachedHunk = 1;
        }
        
        memcpy(
               output,
               reader->hunkBuffer + offset,
               reader->frameBytes
               );
        
        return 1;
    }
    
    /*
     * Frame crosses a hunk boundary.
     */
    uint32_t firstBytes =
    reader->hunkBytes - offset;
    
    uint32_t secondBytes =
    reader->frameBytes - firstBytes;
    
    if (!reader->hasCachedHunk ||
        reader->cachedHunk != hunk) {
        
        chd_error error =
        chd_read(
                 reader->chd,
                 hunk,
                 reader->hunkBuffer
                 );
        
        if (error != CHDERR_NONE) {
            set_error(
                      reader,
                      chd_error_string(error)
                      );
            
            return 0;
        }
        
        reader->cachedHunk = hunk;
        reader->hasCachedHunk = 1;
    }
    
    memcpy(
           output,
           reader->hunkBuffer + offset,
           firstBytes
           );
    
    chd_error error =
    chd_read(
             reader->chd,
             hunk + 1,
             reader->hunkBuffer
             );
    
    if (error != CHDERR_NONE) {
        set_error(
                  reader,
                  chd_error_string(error)
                  );
        
        return 0;
    }
    
    memcpy(
           output + firstBytes,
           reader->hunkBuffer,
           secondBytes
           );
    
    reader->cachedHunk = hunk + 1;
    
    return 1;
}

const char *chd_reader_error(CHDReader *reader)
{
    if (!reader) {
        return "Invalid CHD reader";
    }
    
    return reader->error;
}
}
