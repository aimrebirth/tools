#include <string>
#include <string_view>

#include <stdint.h>
#include <windows.h>

/*struct char20 {
    char string[20];
};*/

#define _BYTE uint8_t
#define this
#define __unaligned
#include "aim.exe.h"
#undef this
#undef __unaligned

using namespace std::literals;

constexpr auto call_command_length = 5;

enum aim1_fix : uint32_t {
    script_function__ISGLIDER = 0x0043A1F6,
    trade_actions_weapon_checks = 0x004072FA,
    setup_proper_weapon_slots_for_a_glider = 0x004D62E4,
    put_weapon_into_the_right_slot_after_purchase = 0x00417A6D,
    sell_correct_weapon = 0x004176BC,
    empty_light_weapon_message = 0x004067C4,
    empty_heavy_weapon_message = 0x0040688B,
};
uint32_t known_caller;
auto player_ptr = (Player **)0x005B7B38;
auto get_player_ptr() {
    auto player_ptr2 = (Player *)0x00678FEC;
    return player_ptr2;
}

enum class glider_id : uint32_t {
    nargoon = 0x7,
    eyedstone = 0xe,
    finder1 = 0x19,
    finder2 = 0x1A,
};

bool __stdcall strequ(const char *s1, const char *s2) {
    do {
        if (*s1 != *s2) {
            return false;
        }
    } while (*s1++ && *s2++);
    return true;
}

__declspec(naked) void fix_script_function__ISGLIDER() {
    __asm {
        ; restore values
        popad
        pushad

        mov edi, eax                ; my glider
        mov esi, [esp+20h+4+8*4]    ; arg

    label_cmp:
        mov al, [edi]
        mov cl, [esi]
        cmp al, cl
        jne bad
        cmp al, 0
        je end
        inc edi
        inc esi
        jmp label_cmp
    end:
        popad
        ; push some regs as required by success branch
        push    ebx
        push    ebp
        push    esi
        push    edi
        push 0x0043A2DA
        ret

    bad:
        popad
        ; original thunk
    copy:
        mov     cl, [eax]; injection point
        inc     eax
        mov[edx], cl
        inc     edx
        test    cl, cl
        jnz     copy

        ; epilogue
        push known_caller
        ret
    }
}

bool is_double_light_weapons_glider() {
    return false
        || get_player_ptr()->glider_name.idx == (int)glider_id::nargoon
        || get_player_ptr()->glider_name.idx == (int)glider_id::finder1
        ;
}
bool is_double_heavy_weapons_glider() {
    return false
        || get_player_ptr()->glider_name.idx == (int)glider_id::eyedstone
        || get_player_ptr()->glider_name.idx == (int)glider_id::finder2
        ;
}
bool is_special_glider() {
    return is_double_light_weapons_glider() || is_double_heavy_weapons_glider();
}

bool is_double_light_weapons_glider(glider_id id) {
    return false
        || id == glider_id::nargoon
        || id == glider_id::finder1
        ;
}
bool is_double_heavy_weapons_glider(glider_id id) {
    return false
        || id == glider_id::eyedstone
        || id == glider_id::finder2
        ;
}
bool is_special_glider(glider_id id) {
    return is_double_light_weapons_glider(id) || is_double_heavy_weapons_glider(id);
}

bool __stdcall can_buy_weapon(int selected_weapon_type, int *canbuy) {
    if (is_special_glider()) {
        if (selected_weapon_type == light && is_double_heavy_weapons_glider()) {
            *(uint8_t*)canbuy = 0;
        }
        if (selected_weapon_type == heavy && is_double_light_weapons_glider()) {
            *(uint8_t *)canbuy = 0;
        }
        return true;
    }
    return false;
}
__declspec(naked) void fix_trade_actions_weapon_checks() {
    __asm {
        ; restore values
        popad
        pushad

        ; ebx = selected weapon type in the store
        ; esi = gun_desc
        ; [esp + 102h] bool is_buy_change
        ; [esp + 103h] bool can_buy

        mov edx, esp
        add edx, 123h
        lea edx, [edx]
        push edx

        push ebx
        call can_buy_weapon
        test al, 1
        jz epi
        popad
        push 0x00407308
        ret

    epi:
        ; epilogue
        popad
        mov     eax, ebx
        sub     eax, 0
        push known_caller
        ret
    }
}

bool can_have_light_weapon_slot(glider *obj, glider_id id, int special) {
    if (is_special_glider(id)) {
        return true;
    }
    return ((special >> 1) & 0x1) == 0;
}
bool can_have_heavy_weapon_slot(glider *obj, glider_id id, int special) {
    if (is_special_glider(id)) {
        return true;
    }
    return obj->standard > 2 && (special & 0x1) == 0;
}
void __stdcall can_have_weapon_slots(glider *gliders, glider *obj, int special) {
    auto id = obj - gliders;
    obj->can_have_light_weap = can_have_light_weapon_slot(obj, (glider_id)id, special);
    obj->can_have_heavy_weap = can_have_heavy_weapon_slot(obj, (glider_id)id, special);
}
__declspec(naked) void fix_can_have_weapon_slots_for_a_glider() {
    __asm {
        ; restore values
        popad
        pushad

        push eax; SPECIAL field value
        mov ecx, edx;
        add ecx, ebx;
        push ecx ; glider*
        push edx ; glider**
        call can_have_weapon_slots

        popad
        push 0x004D6351
        ret
    }
}

bool __stdcall put_purchased_weapon_into_light_slot(int purchased_weapon_type) {
    if (is_special_glider()) {
        return get_player_ptr()->glider_object->light_gun_id == -1;
    }
    return purchased_weapon_type == 0;
}
__declspec(naked) void fix_put_weapon_into_the_right_slot_after_purchase() {
    __asm {
        ; restore values
        popad
        pushad

        push eax ; purchased weapon type

        call put_purchased_weapon_into_light_slot
        cmp al, 1
        jne heavy_exit
        popad
        push 0x00417AA1
        ret

    heavy_exit:
        popad
        push 0x00417A75
        ret
    }
}

bool __stdcall sell_light_weapon(int sold_weapon_type) {
    if (is_special_glider()) {
        return get_player_ptr()->glider_object->light_gun_id != -1;
    }
    return sold_weapon_type == 0;
}
__declspec(naked) void fix_sell_correct_weapon() {
    __asm {
        ; restore values
        popad
        pushad

        push eax; purchased weapon type

        call sell_light_weapon
        cmp al, 1
        jne heavy_exit
        popad
        push 0x004176F1
        ret

    heavy_exit:
        popad
        push 0x004176C4
        ret
    }
}

uint32_t empty_weapon_message;
void __stdcall fix_empty_weapon_message(int light_slot) {
    constexpr auto INT_EMPTY_LIGHT_GUN = 0x00521AA8;
    constexpr auto INT_EMPTY_HEAVY_GUN = 0x00521A94;
    if (is_double_light_weapons_glider()) {
        empty_weapon_message = INT_EMPTY_LIGHT_GUN;
    } else if (is_double_heavy_weapons_glider()) {
        empty_weapon_message = INT_EMPTY_HEAVY_GUN;
    } else {
        empty_weapon_message = light_slot ? INT_EMPTY_LIGHT_GUN : INT_EMPTY_HEAVY_GUN;
    }
}
__declspec(naked) void fix_empty_light_weapon_message() {
    __asm {
        ; restore values
        popad
        pushad

        mov eax, 1
        push eax
        call fix_empty_weapon_message

        popad
        push empty_weapon_message
        push known_caller
        ret
    }
}
__declspec(naked) void fix_empty_heavy_weapon_message() {
    __asm {
        ; restore values
        popad
        pushad

        mov eax, 0
        push eax
        call fix_empty_weapon_message

        popad
        push empty_weapon_message
        push known_caller
        ret
    }
}

extern "C" __declspec(dllexport) __declspec(naked) void dispatcher() {
    __asm {
        pop known_caller
        pushad
    }
    switch (known_caller - call_command_length) {
    case aim1_fix::script_function__ISGLIDER:
        known_caller += 0xA - call_command_length; // make normal ret
        __asm jmp fix_script_function__ISGLIDER
        break;
    case aim1_fix::trade_actions_weapon_checks:
        __asm jmp fix_trade_actions_weapon_checks
        break;
    case aim1_fix::setup_proper_weapon_slots_for_a_glider:
        __asm jmp fix_can_have_weapon_slots_for_a_glider
        break;
    case aim1_fix::put_weapon_into_the_right_slot_after_purchase:
        __asm jmp fix_put_weapon_into_the_right_slot_after_purchase
        break;
    case aim1_fix::sell_correct_weapon:
        __asm jmp fix_sell_correct_weapon
        break;
    case aim1_fix::empty_light_weapon_message:
        __asm jmp fix_empty_light_weapon_message
        break;
    case aim1_fix::empty_heavy_weapon_message:
        __asm jmp fix_empty_heavy_weapon_message
        break;
    default:
        break;
    }
    // just return
    __asm {
        popad
        push known_caller
        ret
    }
}

void setup() {
    //constexpr uint32_t free_data_base = 0x006929C0;
    constexpr uint32_t free_data_base = 0x00692FF0;
    auto mem = (uint8_t *)free_data_base;
    mem[0] = 0xE9;
    *(uint32_t*)&mem[1] = (uint32_t)&dispatcher - free_data_base - call_command_length;
}
BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        setup();
    }
    return TRUE;
}
