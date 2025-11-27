//
//  MandarineEmulator.m
//  Mandarine
//
//  Created by Jarrod Norwell on 17/11/2025.
//

#import "MandarineEmulator.h"
#import <GameController/GameController.h>

#include <atomic>
#include <condition_variable>
#include <fstream>
#include <iostream>
#include <mach/mach_time.h>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "avocado/config.h"
#include "avocado/memory_card/card_formats.h"
#include "avocado/input/input_manager.h"
#include "avocado/state/state.h"
#include "avocado/system.h"
#include "avocado/system_tools.h"
#include "avocado/utils/file.h"

bool fileExists(const std::string &name) {
    FILE *f = fopen(name.c_str(), "r");
    bool exists = false;
    if (f) {
        exists = true;
        fclose(f);
    }
    return exists;
}

std::vector<uint8_t> getFileContents(const std::string &name) {
    std::vector<uint8_t> contents;

    FILE *f = fopen(name.c_str(), "rb");
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

bool putFileContents(const std::string &name, const std::string contents) {
    FILE *f = fopen(name.c_str(), "wb");
    if (!f) return false;

    fwrite(&contents[0], 1, contents.size(), f);

    fclose(f);

    return true;
}

std::string getFileContentsAsString(const std::string &name) {
    std::string contents;

    FILE *f = fopen(name.c_str(), "rb");
    if (!f) return contents;

    fseek(f, 0, SEEK_END);
    int filesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    contents.resize(filesize);
    fread(&contents[0], 1, filesize, f);

    fclose(f);
    return contents;
}

size_t getFileSize(const std::string &name) {
    FILE *f = fopen(name.c_str(), "rb");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fclose(f);

    return size;
}

size_t findSystemCnf(std::ifstream& file, size_t startOffset, size_t maxReadSize) {
    std::vector<char> buffer(maxReadSize);
    file.seekg(startOffset);
    file.read(buffer.data(), maxReadSize);
    std::string content(buffer.begin(), buffer.end());
    return content.find("SYSTEM.CNF");
}

std::string gameID(const std::string& binFilePath) {
    const int blockSize = 1024 * 1024; // Read in 1MB blocks
    std::ifstream binFile(binFilePath, std::ios::binary);
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
            // Extract the game ID
            size_t start = content.find("cdrom:", bootPos) + 7;
            size_t end = content.find(';', start);

            if (start == std::string::npos || end == std::string::npos) {
                throw std::runtime_error("Invalid BOOT line format");
            }

            return content.substr(start, end - start);
        }

        bytesRead += readSize;
    }

    throw std::runtime_error("BOOT line not found in the file");
}

#include "avocado/sound/sound.h"
#include <fmt/core.h>

namespace Sound {
std::deque<uint16_t> buffer;
std::mutex audioMutex;
};  // namespace Sound

namespace {
SDL_AudioDeviceID deviceID = 0;
SDL_AudioStream* stream = nullptr;

void audioCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount)
{
    (void)userdata;
    
    // additional_amount is byte count
    if (additional_amount <= 0)
        return;
    
    uint8_t* buf = (uint8_t*)malloc(additional_amount);
    memset(buf, 0, additional_amount);
    
    std::unique_lock<std::mutex> lock(Sound::audioMutex);
    
    size_t samples_available = Sound::buffer.size();
    size_t samples_needed = additional_amount / sizeof(int16_t);
    
    size_t samples_to_copy = std::min(samples_available, samples_needed);
    
    for (size_t i = 0; i < samples_to_copy; i++) {
        int16_t sample = Sound::buffer.front();
        Sound::buffer.pop_front();
        
        // Write LE 16-bit PCM correctly
        buf[i * 2 + 0] = (uint8_t)(sample & 0xFF);       // LSB
        buf[i * 2 + 1] = (uint8_t)((sample >> 8) & 0xFF); // MSB
    }
    
    lock.unlock();
    
    SDL_PutAudioStreamData(stream, buf, additional_amount);
    
    free(buf);
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

void Sound::play() { SDL_ResumeAudioStreamDevice(stream); }

void Sound::stop() { SDL_PauseAudioStreamDevice(stream); }

void Sound::close() { SDL_DestroyAudioStream(stream); }

void Sound::clearBuffer() { buffer.clear(); }

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

class GCInputManager : public InputManager {
public:
    GCInputManager() {}
    
    void press(std::string button, int index) {
        state[fmt::format("controller/{}/{}", index, button).c_str()] = AnalogValue(true);
    }
    
    void release(std::string button, int index) {
        state[fmt::format("controller/{}/{}", index, button).c_str()] = AnalogValue(false);
    }
    
    void drag(std::string button, int index, int16_t value) {
        auto av = AnalogValue(static_cast<uint8_t>(value >> 8));
        state[fmt::format("controller/{}/{}", index, button).c_str()] = av;
        state[fmt::format("controller/{}/{}", index, button).c_str()] = {};
    }
};

struct Object {
    std::unique_ptr<System> system;
    
    std::jthread thread;
    std::atomic<bool> paused;
    std::mutex mutex;
    std::condition_variable_any cv;
} object;

auto manager = std::make_unique<GCInputManager>();
@implementation MandarineEmulator
+(MandarineEmulator *) sharedInstance {
    static MandarineEmulator *sharedInstance = NULL;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
    });
    return sharedInstance;
}

-(void) insertCartridge:(NSURL *)url {
    NSURL *mandarineDirectoryURL = [[[[NSFileManager defaultManager] URLsForDirectory:NSDocumentDirectory inDomains:NSUserDomainMask] firstObject] URLByAppendingPathComponent:@"Mandarine"];
    
    NSURL *mcdOne = [[mandarineDirectoryURL URLByAppendingPathComponent:@"memcards"] URLByAppendingPathComponent:@"card1.mcr"];
    NSURL *mcdTwo = [[mandarineDirectoryURL URLByAppendingPathComponent:@"memcards"] URLByAppendingPathComponent:@"card2.mcr"];
    
    config.bios = [[[mandarineDirectoryURL URLByAppendingPathComponent:@"sysdata"] URLByAppendingPathComponent:@"bios.bin"].path UTF8String];
    config.memoryCard[0].path = [mcdOne.path UTF8String];
    config.memoryCard[1].path = [mcdTwo.path UTF8String];
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:mcdOne.path]) {
        std::array<uint8_t, memory_card::MEMCARD_SIZE> data;
        memory_card::format(data);
        memory_card::save(data, config.memoryCard[0].path);
    }
    
    if (![[NSFileManager defaultManager] fileExistsAtPath:mcdTwo.path]) {
        std::array<uint8_t, memory_card::MEMCARD_SIZE> data;
        memory_card::format(data);
        memory_card::save(data, config.memoryCard[1].path);
    }
    
    Sound::init();
    if (config.options.sound.enabled)
        Sound::play();
    
    object.system = system_tools::hardReset();
    
    system_tools::loadFile(object.system, [url.path UTF8String]);
    
    InputManager::setInstance(manager.get());
}

-(void) start {
    object.thread = std::jthread([&](std::stop_token token) {
        using namespace std::chrono;

        while (!token.stop_requested()) {
            if (object.paused)
                continue;
            
            if (object.system->state == System::State::run) {
                object.system->gpu->clear();
                object.system->controller->update();
                
                object.system->emulateFrame();
            }
            
            if (object.system->gpu->gp1_08.colorDepth == gpu::GP1_08::ColorDepth::bit24) {
                if (auto buffer = [[MandarineEmulator sharedInstance] rgb888])
                    dispatch_async(dispatch_get_main_queue(), ^{
                        buffer(object.system->gpu->vram.data(),
                               object.system->gpu->displayAreaStartX,
                               object.system->gpu->displayAreaStartY,
                               object.system->gpu->gp1_08.getHorizontalResoulution(),
                               object.system->gpu->gp1_08.getVerticalResoulution());
                    });
            } else {
                if (auto buffer = [[MandarineEmulator sharedInstance] bgr555])
                    dispatch_async(dispatch_get_main_queue(), ^{
                        buffer(object.system->gpu->vram.data(),
                               object.system->gpu->displayAreaStartX,
                               object.system->gpu->displayAreaStartY,
                               object.system->gpu->gp1_08.getHorizontalResoulution(),
                               object.system->gpu->gp1_08.getVerticalResoulution());
                    });
            }
            
            limitFramerate(true, object.system->gpu->isNtsc());
        }
    });
}

-(void) stop {
    object.thread.request_stop();
    if (object.thread.joinable())
        object.thread.join();
    
    system_tools::saveMemoryCard(object.system, 0, true);
    system_tools::saveMemoryCard(object.system, 1, true);
    
    Sound::close();
    
    object.paused.store(false);
}

-(void) pause:(BOOL)pause {
    if (pause)
        object.paused.store(true);
    else {
        object.paused.store(false);
        object.cv.notify_all();
    }
}

-(BOOL) isPaused {
    return object.paused.load();
}

-(void) input:(NSInteger)slot button:(NSString *)button pressed:(BOOL)pressed {
    auto string = [button cStringUsingEncoding:NSUTF8StringEncoding];
    auto index = [[NSNumber numberWithInteger:slot] intValue] + 1;
    if (pressed) {
        manager->press(string, index);
    } else {
        manager->release(string, index);
    }
}

-(void) drag:(NSInteger)slot stick:(NSString *)stick value:(int16_t)value {
    auto string = [stick cStringUsingEncoding:NSUTF8StringEncoding];
    auto index = [[NSNumber numberWithInteger:slot] intValue] + 1;
    manager->drag(string, index, value);
}

-(void) load:(NSURL *)url {
    state::loadFromFile(object.system.get(), [url.path UTF8String]);
}

-(void) save:(NSURL *)url {
    state::saveToFile(object.system.get(), [url.path UTF8String]);
}

-(NSString *) id:(NSURL *)url {
    NSString *string = @"";
    try {
        string = [NSString stringWithCString:gameID([url.path UTF8String]).c_str() encoding:NSUTF8StringEncoding];
    } catch (std::runtime_error& e) {
        NSLog(@"%@", [NSString stringWithCString:e.what() encoding:NSUTF8StringEncoding]);
        string = @"";
    }
    
    return [[string stringByReplacingOccurrencesOfString:@"_" withString:@"-"] stringByReplacingOccurrencesOfString:@"." withString:@""];
}
@end
