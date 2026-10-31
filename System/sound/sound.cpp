//
//  sound.cpp
//  Mandarine
//
//  Created by Jarrod Norwell on 17/10/2026.
//

#if __APPLE__
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE

#include "avocado/sound/sound.h"

#else

#include "sound/sound.h"

#endif // TARGET_OS_IPHONE

#include <SDL3/SDL_audio.h>

static void audioCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
    (void)userdata;

    if (additional_amount <= 0)
        return;

    std::vector<uint8_t> data(additional_amount);

    std::unique_lock<std::mutex> lock(sound::mutex);

    size_t samples_available = sound::buffer.size();
    size_t samples_needed = additional_amount / sizeof(int16_t);

    size_t samples_to_copy = std::min(samples_available, samples_needed);

    for (size_t i = 0; i < samples_to_copy; i++) {
        int16_t sample = sound::buffer.front();
        sound::buffer.pop_front();

        data.at(i * 2) = (uint8_t)sample & 0xFF;
        data.at(i * 2 + 1) = (uint8_t)(sample >> 8) & 0xFF;
    }

    SDL_PutAudioStreamData(stream, data.data(), additional_amount);
};

namespace sound {
    std::deque<uint16_t> buffer;
    std::mutex mutex;

    SDL_AudioDeviceID deviceID;
    SDL_AudioStream* stream;
};


void sound::clear() {
    sound::buffer.clear();
}

void sound::close() {
    SDL_DestroyAudioStream(sound::stream);
}

void sound::init() {
    SDL_AudioSpec spec{};
    spec.channels = 2;
    spec.freq = 44100;
    spec.format = SDL_AUDIO_S16;

    sound::deviceID = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    sound::stream = SDL_OpenAudioDeviceStream(sound::deviceID, &spec, &audioCallback, nullptr);

    SDL_ResumeAudioStreamDevice(stream);
}

void sound::play() {
    SDL_ResumeAudioStreamDevice(sound::stream);
}

void sound::stop() {
    SDL_PauseAudioStreamDevice(sound::stream);
}
