#pragma once
#include <Windows.h>
#include <thread>
#include <memory>
#include <atomic>
#include <immintrin.h>
#include "../../../util/globals.h"
#include "../../../util/classes/classes.h"
#include "../../../util/offsets.h"

namespace silent_threads {
    inline std::unique_ptr<roblox::instance> g_mouseservice;
    inline std::atomic<bool> g_silent_aim_enabled{ false };
    inline roblox::player g_silent_cached_target;
    inline std::atomic<bool> g_silent_found_target{ false };
    inline std::atomic<bool> g_silent_data_ready{ false };
    inline roblox::instance g_silent_aim_instance;
    inline Vector2 g_silent_partpos;
    inline std::atomic<uint64_t> g_silent_cached_position_x{ 0 };
    inline std::atomic<uint64_t> g_silent_cached_position_y{ 0 };
    inline bool g_silent_knock_check = true;
    inline bool g_silent_auto_switch = true;
    inline bool g_silent_spoof_mouse = true;

    class c_silent_help {
    public:
        uint64_t address;
        static uint64_t cached_input_object;

        c_silent_help() : address(0) {}
        c_silent_help(uint64_t addr) : address(addr) {}

        void set_frame_pos_x(uint64_t position);
        void set_frame_pos_y(uint64_t position);
        void initialize_mouse_service(uint64_t address);
        void write_mouse_position(uint64_t address, float x, float y);
        Vector2 read_real_mouse_position();
        void restore_real_mouse_position(uint64_t address);
    };

    uint64_t get_current_input_object(uint64_t base_address);
    bool should_silent_aim_be_active();
    void update_silent_aim_key_state();
    void thread_1();
    void thread_2();
    void initialize();
}
