#pragma once

#include <stdint.h>

#ifdef DONT_OPTIMIZE
#pragma optimize("", off)
#endif

constexpr auto call_command_length = 5;

// public enums
enum aim1_fix : uint32_t {
    script_function__ISGLIDER = 0x0043A1F6,
    trade_actions_weapon_checks = 0x004072FA,
    setup_proper_weapon_slots_for_a_glider = 0x004D62E4,
    put_weapon_into_the_right_slot_after_purchase = 0x00417A6D,
    sell_correct_weapon = 0x004176BC,
    empty_light_weapon_message = 0x004067C4,
    empty_heavy_weapon_message = 0x0040688B,
    can_leave_trade_window = 0x0040C20E,
};
// set different size if your injection takes more than default 5 bytes
uint32_t get_injection_size(uint32_t key) {
    switch (key) {
    case aim1_fix::script_function__ISGLIDER: return 10;
    case aim1_fix::can_leave_trade_window: return 6;
    default: return call_command_length;
    }
}

#ifdef DONT_OPTIMIZE
#pragma optimize("", on)
#endif
