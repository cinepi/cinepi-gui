#pragma once
#ifndef SHAREDCONTEXT_HPP
#define SHAREDCONTEXT_HPP

#include <time.h>
#include <stdio.h>
#include <thread>
#include <functional>
#include <cstdint>
#include <cstddef>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <optional>

#include <libcamera/color_space.h>
#include <libcamera/pixel_format.h>
#include <libcamera/controls.h>

#define STATE_TIMEOUT 0x08
#define STATE_NULL_REF 0x04
#define STATE_VALID 0x01

#define PROJECT_ID 0x43494E45 // ASCII for "CINE"

struct StreamInfo
{
	StreamInfo() : width(0), height(0), stride(0) {}
	unsigned int width;
	unsigned int height;
	unsigned int stride;
	libcamera::PixelFormat pixel_format;
	std::optional<libcamera::ColorSpace> colour_space;
};


struct SharedMetadata {
	float exposure_time;
	float analogue_gain;
	float digital_gain;
    unsigned int colorTemp;
    int64_t ts;
    float colour_gains[2];
	float focus;
	float fps;
	bool aelock;
	float lens_position;
	int af_state;
};

// SharedMemoryBuffer structure declaration
struct SharedMemoryBuffer {
    SharedMemoryBuffer() : fd_raw(-1), fd_isp(-1), fd_lores(-1), procid(-1), frame(-1)  {}
    int fd_raw;
    int fd_isp;
    int fd_lores;
    StreamInfo raw;
    StreamInfo isp;
    StreamInfo lores;
    size_t raw_length;
    size_t isp_length;
    size_t lores_length;
    int procid;
    uint64_t frame;
    uint64_t ts;
    SharedMetadata metadata;
    unsigned int sequence;
    float framerate;
    uint8_t stats[23200];
};
// SharedContext class declaration
class SharedContext {
    public:
        SharedContext()
            : 
            state_(0),
            timedOut_(false),
            lastProcId_(-1),
            abortThread_(false)
            

        {
            console = spdlog::stdout_color_mt("shared_context");
            bind_shared_memory();
            main_thread_ = std::thread(std::bind(&SharedContext::threadTask, this));
        };

        ~SharedContext(){
            abortThread_ = true;
            main_thread_.join();
        };

        SharedMemoryBuffer* get_context() const{
            return shared_memory;
        }

        void bind_shared_memory();
        bool connected(){
            return state_ == STATE_VALID && !timedOut_;
        }

        uint64_t getTs(){
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            return uint64_t(ts.tv_sec * 1000LL + ts.tv_nsec / 1000000);
        }

    private:
        SharedMemoryBuffer* shared_memory;

        std::shared_ptr<spdlog::logger> console;

        key_t get_shared_memory_key();

        void threadTask();

        uint8_t state_;

        bool timedOut_;
        int lastProcId_;

        bool abortThread_;
        std::thread main_thread_;
        
};

#endif // SHAREDCONTEXT_HPP
