//
//  bridge.h
//  Mandarine
//
//  Created by Jarrod Norwell on 14/6/2026.
//

#include <cstdint>
#include <functional>
#include <string>

namespace mandarine {
void print_about(void);

std::string disc_identifier(std::string);

void initialize_paths(void);
void initialize_memory_cards(void);
void initialize_system(void);

void destroy_system(void);

void insert_disc(std::string);

bool is_paused(bool = false, bool = false);
bool is_running(bool = false, bool = false);

void start(void);
void stop(void);

int16_t framebuffer_start_x(void), framebuffer_start_y(void);
int framebuffer_height(void), framebuffer_width(void);

using VideoBufferCallback15Bit = void(*)(void*, void*);
VideoBufferCallback15Bit callback_15bit;
void video_buffer_callback_15bit(VideoBufferCallback15Bit);

using VideoBufferCallback24Bit = void(*)(void*, uint16_t*);
VideoBufferCallback24Bit callback_24bit;
void video_buffer_callback_24bit(VideoBufferCallback24Bit);

void press_button(std::string, int), release_button(std::string, int);
void drag_thumbstick(std::string, uint8_t);

void* context;
void set_context(void* context);

enum class SETTING {
    WIDESCREEN = 0,
    FORCE_WIDESCREEN = 1,
    VSYNC = 2,
    FORCE_NTSC = 3,
    NATIVE_TEXTURE_FORMAT = 4,
    SOUND_ENABLED = 5,
    PRESERVE_STATE = 6,
    TIME_TRAVEL = 7,
    EXTENDED_MEMORY = 8,
    
    LOG_BIOS = 9,
    LOG_CDROM = 10,
    LOG_CONTROLLER = 11,
    LOG_DMA = 12,
    LOG_GPU = 13,
    LOG_GTE = 14,
    LOG_MDEC = 15,
    LOG_MEMORY_CARD = 16,
    LOG_MEMORY_CONTROL = 17,
    LOG_SPU = 18,
    LOG_SYSTEM = 19
};

void set_setting(SETTING, bool);
}

#ifdef __cplusplus
extern "C" {
#endif

#include <chd.h>

namespace chd_reader {
struct CHDReader {
    chd_file *chd;
    
    uint8_t *hunkBuffer;
    uint32_t hunkBytes;
    
    uint32_t frameBytes;
    
    uint32_t cachedHunk;
    int hasCachedHunk;
    
    char error[256];
};

/// Opens a CHD file for reading.
CHDReader *chd_reader_open(const char *path);

/// Closes a CHD reader.
void chd_reader_close(CHDReader *reader);

/// Returns the logical size of the CHD in bytes.
uint64_t chd_reader_logical_bytes(CHDReader *reader);

/// Returns the bytes per CD frame.
uint32_t chd_reader_frame_bytes(CHDReader *reader);

/// Reads one complete CD frame.
int chd_reader_read_frame(
                          CHDReader *reader,
                          uint32_t frame,
                          uint8_t *output
                          );

/// Returns a human-readable error.
const char *chd_reader_error(CHDReader *reader);
}
#ifdef __cplusplus
}
#endif
