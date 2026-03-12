#include "silent_threads.h"
#include "../../../util/console/console.h"
#include <iostream>

using namespace math;

namespace silent_threads {
    uint64_t c_silent_help::cached_input_object = 0;

    void c_silent_help::set_frame_pos_x(uint64_t position) {
        write<uint64_t>(address + offsets::FramePositionOffsetX, position);
    }

    void c_silent_help::set_frame_pos_y(uint64_t position) {
        write<uint64_t>(address + offsets::FramePositionOffsetY, position);
    }

    uint64_t get_current_input_object(uint64_t base_address) {
        uint64_t object_address = read<uint64_t>(base_address + offsets::InputObject + sizeof(std::shared_ptr<void*>));
        return object_address;
    }

    void c_silent_help::initialize_mouse_service(uint64_t address) {
        cached_input_object = get_current_input_object(address);

        if (cached_input_object && cached_input_object != 0xFFFFFFFFFFFFFFFF) {
            const char* base_pointer = reinterpret_cast<const char*>(cached_input_object);
            _mm_prefetch(base_pointer + offsets::MousePosition, _MM_HINT_T0);
            _mm_prefetch(base_pointer + offsets::MousePosition + sizeof(Vector2), _MM_HINT_T0);
        }
    }

    void c_silent_help::write_mouse_position(uint64_t address, float x, float y) {
        cached_input_object = get_current_input_object(address);
        if (cached_input_object != 0 && cached_input_object != 0xFFFFFFFFFFFFFFFF) {
            Vector2 new_position = { x, y };
            write<Vector2>(cached_input_object + offsets::MousePosition, new_position);
        }
    }

    Vector2 c_silent_help::read_real_mouse_position() {
        POINT cursor_point;
        HWND rblxWnd = FindWindowA(nullptr, "Roblox");
        if (!rblxWnd || !GetCursorPos(&cursor_point) || !ScreenToClient(rblxWnd, &cursor_point)) {
            return Vector2{ 0, 0 };
        }
        return Vector2{ static_cast<float>(cursor_point.x), static_cast<float>(cursor_point.y) };
    }

    void c_silent_help::restore_real_mouse_position(uint64_t address) {
        Vector2 real_pos = read_real_mouse_position();
        if (real_pos.x != 0 || real_pos.y != 0) {
            write_mouse_position(address, real_pos.x, real_pos.y);
        }
    }

    bool should_silent_aim_be_active() {
        if (!globals::combat::silentaim)
            return false;
        
        globals::combat::silentaimkeybind.update();
        
        return globals::combat::silentaimkeybind.enabled;
    }

    roblox::player getClosestPlayerFromCursor() {
        POINT cursor_point;
        HWND rblxWnd = FindWindowA(nullptr, "Roblox");
        if (!rblxWnd) {
            return {};
        }

        if (!GetCursorPos(&cursor_point)) {
            return {};
        }

        if (!ScreenToClient(rblxWnd, &cursor_point)) {
            return {};
        }

        Vector2 cursor = { static_cast<float>(cursor_point.x), static_cast<float>(cursor_point.y) };

        std::vector<roblox::player> players_snapshot = globals::instances::cachedplayers;
        if (players_snapshot.empty()) {
            return {};
        }

        roblox::player closestPlayer{};
        float shortestDistance = FLT_MAX;

        Vector2 dimensions = globals::instances::visualengine.get_dimensions();
        Matrix4x4 viewmatrix = read<Matrix4x4>(globals::instances::visualengine.address + offsets::viewmatrix);

        bool useFov = globals::combat::usesfov;
        float fovSize = globals::combat::sfovsize;
        float fovSizeSquared = fovSize * fovSize;

        for (roblox::player& player : players_snapshot) {
            if (player.main.address == 0)
                continue;

            if (globals::combat::teamcheck && globals::is_teammate(player)) {
                continue;
            }

            if (globals::combat::grabbedcheck && globals::is_grabbed(player)) {
                continue;
            }

            if (globals::combat::knockcheck) {
                try {
                    if (player.bodyeffects.findfirstchild("K.O").read_bool_value()) {
                        continue;
                    }
                } catch (...) {}
            }

            roblox::instance part = player.uppertorso;
            if (!is_valid_address(part.address))
                continue;

            Vector3 partPosition = part.get_pos();
            
            if (globals::combat::silentpredictions) {
                Vector3 velocity = part.get_velocity();
                Vector3 veloVector = velocity / Vector3(globals::combat::silentpredictionsx, globals::combat::silentpredictionsy, globals::combat::silentpredictionsx);
                partPosition = partPosition + veloVector;
            }
            
            Vector2 partScreen = math::WorldToScreen(partPosition, dimensions, viewmatrix);

            if (partScreen.x == 0 && partScreen.y == 0) {
                continue;
            }

            float distance_from_cursor = (partScreen - cursor).magnitude();
            
            if (useFov && distance_from_cursor > fovSize) {
                continue;
            }

            if (distance_from_cursor < shortestDistance) {
                shortestDistance = distance_from_cursor;
                closestPlayer = player;
            }
        }

        return closestPlayer;
    }

    void thread_1() {
        roblox::player target{};

        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);
        std::cout << "[SILENT_THREADS] Thread 1 started" << std::endl;

        HWND roblox_window = FindWindowA(0, "Roblox");

        for (;;) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

            if (!is_valid_address(globals::instances::datamodel.address)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            auto mouseservice_inst = globals::instances::datamodel.read_service("MouseService");
            if (is_valid_address(mouseservice_inst.address)) {
                if (!g_mouseservice) {
                    g_mouseservice = std::make_unique<roblox::instance>(mouseservice_inst);
                    std::cout << "[SILENT_THREADS] MouseService initialized" << std::endl;
                }
            }

            if (!g_mouseservice || !is_valid_address(globals::instances::datamodel.address) || !is_valid_address(globals::instances::visualengine.address)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            if (!should_silent_aim_be_active()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                g_silent_data_ready = false;
                g_silent_cached_target.main.address = 0;
                target.main.address = 0;
                g_silent_found_target = false;
                continue;
            }

            if (!is_valid_address(globals::instances::datamodel.address)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            auto players = globals::instances::datamodel.read_service("Players");
            if (!is_valid_address(players.address)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            auto local_player = roblox::instance(read<uint64_t>(players.address + offsets::LocalPlayer));

            if (!is_valid_address(local_player.address)) {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                continue;
            }

            static int aim_instance_check_counter = 0;
            if (aim_instance_check_counter++ % 10 == 0) {
                auto player_gui = local_player.findfirstchild("PlayerGui");
                if (is_valid_address(player_gui.address)) {
                    auto main_screen_gui = player_gui.findfirstchild("MainScreenGui");
                    if (!is_valid_address(main_screen_gui.address))
                        main_screen_gui = player_gui.findfirstchild("Main Screen");

                    if (is_valid_address(main_screen_gui.address)) {
                        g_silent_aim_instance = roblox::instance(main_screen_gui.findfirstchild("Aim"));
                    }
                }
            }

            if (globals::combat::connect_to_aimbot) {
                if (globals::combat::aimbot_locked && is_valid_address(globals::combat::aimbot_current_target.main.address)) {
                    target = globals::combat::aimbot_current_target;
                    g_silent_found_target = is_valid_address(target.head.address);
                    g_silent_cached_target = target;
                } else {
                    g_silent_found_target = false;
                    g_silent_data_ready = false;
                    g_silent_cached_target.main.address = 0;
                    target.main.address = 0;
                    continue;
                }
            } else {
                target = getClosestPlayerFromCursor();
                g_silent_found_target = is_valid_address(target.head.address);
                g_silent_cached_target = target;
            }
            
            if (!g_silent_found_target) {
                g_silent_data_ready = false;
                continue;
            }

            if (g_silent_knock_check && g_silent_auto_switch) {
                auto model_instance = roblox::instance(target.main.address).model_instance();
                if (is_valid_address(model_instance.address)) {
                    auto body_effects = model_instance.findfirstchild("BodyEffects");
                    if (is_valid_address(body_effects.address)) {
                        auto ko = body_effects.findfirstchild("K.O");
                        if (is_valid_address(ko.address)) {
                            bool ko_value = read<bool>(ko.address + offsets::Value);
                            if (ko_value) {
                                g_silent_found_target = false;
                                g_silent_data_ready = false;
                                continue;
                            }
                        }
                    }
                }
            }

            if (is_valid_address(globals::instances::visualengine.address)) {
                auto head_part = g_silent_cached_target.head;
                if (is_valid_address(head_part.address)) {
                    Vector3 part_3d = head_part.get_pos();
                    
                    if (globals::combat::silentpredictions) {
                        Vector3 velocity = head_part.get_velocity();
                        Vector3 veloVector = velocity / Vector3(globals::combat::silentpredictionsx, globals::combat::silentpredictionsy, globals::combat::silentpredictionsx);
                        part_3d = part_3d + veloVector;
                    }
                    
                    Vector2 dims = globals::instances::visualengine.get_dimensions();
                    Matrix4x4 view = read<Matrix4x4>(globals::instances::visualengine.address + offsets::viewmatrix);
                    
                    g_silent_partpos = math::WorldToScreen(part_3d, dims, view);
                    
                    if (g_silent_partpos.x == -1.0f || g_silent_partpos.y == -1.0f) {
                        g_silent_data_ready = false;
                        continue;
                    }
                    
                    POINT cursor_point;
                    GetCursorPos(&cursor_point);
                    if (roblox_window)
                        ScreenToClient(roblox_window, &cursor_point);

                    g_silent_cached_position_x = static_cast<uint64_t>(cursor_point.x);
                    g_silent_cached_position_y = static_cast<uint64_t>(dims.y - std::abs(dims.y - static_cast<float>(cursor_point.y)) - 58);
                    g_silent_data_ready = true;
                }
                else {
                    g_silent_data_ready = false;
                }
            }
            else {
                g_silent_data_ready = false;
            }
        }
    }

    void thread_2() {
        c_silent_help mouse_service_instance{};
        bool mouse_service_initialized = false;
        bool is_spoofing = false;
        
        std::cout << "[SILENT_THREADS] Thread 2 started" << std::endl;

        for (;;) {
            if (!g_mouseservice) {
                mouse_service_initialized = false;
                is_spoofing = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            if (!should_silent_aim_be_active()) {
                if (is_spoofing && mouse_service_initialized) {
                    try {
                        mouse_service_instance.restore_real_mouse_position(g_mouseservice->address);
                    } catch (...) {}
                }
                mouse_service_initialized = false;
                is_spoofing = false;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            if (g_silent_cached_target.main.address != 0 && g_silent_data_ready && g_mouseservice && is_valid_address(g_mouseservice->address)) {
                if (g_silent_partpos.x < 0.0f || g_silent_partpos.y < 0.0f ||
                    g_silent_partpos.x > 10000.0f || g_silent_partpos.y > 10000.0f) {
                    if (is_spoofing && mouse_service_initialized) {
                        try {
                            mouse_service_instance.restore_real_mouse_position(g_mouseservice->address);
                        } catch (...) {}
                    }
                    is_spoofing = false;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                    continue;
                }

                try {
                    if (!mouse_service_initialized) {
                        mouse_service_instance.initialize_mouse_service(g_mouseservice->address);
                        mouse_service_initialized = true;
                    }

                    if (g_silent_spoof_mouse && is_valid_address(g_silent_aim_instance.address)) {
                        c_silent_help aim_helper(g_silent_aim_instance.address);
                        aim_helper.set_frame_pos_x(g_silent_cached_position_x);
                        aim_helper.set_frame_pos_y(g_silent_cached_position_y);
                    }

                    mouse_service_instance.write_mouse_position(g_mouseservice->address, g_silent_partpos.x, g_silent_partpos.y);
                    is_spoofing = true;
                }
                catch (...) {
                    mouse_service_initialized = false;
                    is_spoofing = false;
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            else {
                if (is_spoofing && mouse_service_initialized) {
                    try {
                        mouse_service_instance.restore_real_mouse_position(g_mouseservice->address);
                    } catch (...) {}
                    is_spoofing = false;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }

    void initialize() {
        std::cout << "[SILENT_THREADS] Initializing silent threads..." << std::endl;
        g_silent_aim_enabled = true;
        std::thread(thread_1).detach();
        std::thread(thread_2).detach();
        std::cout << "[SILENT_THREADS] Silent threads initialized successfully" << std::endl;
    }
}
