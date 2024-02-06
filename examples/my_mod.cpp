/*
name: aim_mod_maker
c++: 23
deps: pub.lzwdgc.Polygon4.Tools.aim1_mod_maker-master
*/

/*
 * Put this file near aim.exe.
 * Invoke mod creation using the following command:
 *    sw run my_mod.cpp
 * You can run the modded game now.
 * You can also distribute mod archive (requires 7z program).
 **/

#ifndef INJECTED_DLL
#include "aim1_mod_maker.h"

// patch note: Authors and Credits
// patch note: lz, Solant, Streef
// patch note:
// patch note: Description
// patch note: This mod fixes some issues with the original AIM v1.04 game.
// patch note:
// patch note: Installation
// patch note: Unpack and drop all files near original aim.exe. Replace files if necessary.
// patch note:

int main(int argc, char *argv[]) {
    mod_maker mod;

    // patch note: CHANGES
    // patch note:
    // patch note: General Notes
    // patch note: enabled free camera (use F3 key) (Solant)
    mod.enable_free_camera();
    // patch note: enabled WIN key during the game (Solant)
    mod.enable_win_key();
    // patch note:

    // patch note: Volcano Sector
    // patch note: rename second FINSWIL-1 to FINSWIL-3 to make this group appear on the map (Streef)
    mod.patch<uint8_t>("location4.mmo", 0x7F599, 0x31, 0x33);
    // patch note: make SWIRE appear (Streef)
    mod.patch<uint8_t>("location4.mmo", 0x7FA34, 1, 0);
    // patch note: make SWILHUN appear (Streef)
    mod.patch<uint8_t>("location4.mmo", 0x7F913, 1, 0);
    // patch note: reposition SWILHUN-3 group (Streef)
    mod.patch<uint8_t>("location4.mmo", 0x7F528, 0x40, 0xB1);
    // patch note: reposition SWILHUN-1 group (Streef)
    mod.patch<uint8_t>("location4.mmo", 0x7EE62, 0xA1, 0xA0);
    // patch note:

    // patch note: Desert Sector
    // patch note: reposition SACREFI-2 (Streef)
    mod.patch<uint8_t>("location5.mmo", 0xBAFF7, 0x18, 0x17);
    // patch note: fix 'TOV_POLYMER_PLATE' spawn (Streef)
    mod.replace("location5.scr", "TOV_POLYMER_PLATES", "TOV_POLYMER_PLATE");
    // patch note:

    // patch note: Hills Sector
    // patch note: set correct model for a plant (Streef)
    mod.patch<uint8_t>("location6.mmo", 0x575DD, 'R', 'F');
    // patch note: fix 'TOV_POLYMER_PLATE' spawn (Streef)
    mod.replace("location6.scr", "TOV_POLYMER_PLATES", "TOV_POLYMER_PLATE");
    // patch note:

    // patch note: Script Changes
    // patch note: _ISGLIDER() function can check exact glider name now, for example _ISGLIDER(GL_M3_A_FIRST1) (lz)
    // patch note: fix joining First Ones having one of four required gliders (lz, Streef)
    mod.replace("ORG_FIRST.scr",
        "IF(_PLAYERHAS(GL_M3_A_FIRST1)||_PLAYERHAS(GL_M3_A_FIRST1))",
        "IF(_ISGLIDER(GL_M3_A_FIRST1)|_ISGLIDER(GL_M3_A_FIRST2))");
    mod.replace("ORG_FIRST.scr",
        "IF(_PLAYERHAS(GL_M4_A_FIRST1)||_PLAYERHAS(GL_M4_A_FIRST1))",
        "IF(_ISGLIDER(GL_M4_S_FIRST1)|_ISGLIDER(GL_M4_S_FIRST2))");
    // patch note: fix joining Sinigr having one of two required gliders (lz, Streef)
    mod.replace("ORG_SINIGR.scr",
        "IF(_PLAYERHAS(GL_S2_PA_SINYGR)|_PLAYERHAS(GL_S4_S_SINYGR))",
        "IF(_ISGLIDER(GL_S2_PA_SINYGR)|_ISGLIDER(GL_S4_S_SINYGR))");
    // patch note:

    /*mod.replace("Script/bin/B_L1_BASE1.scr", "_ADDBALANCE(300)", R"(
    _ADDBALANCE(300 )

    _ADDOBJECT(GL_M3_PA_EYEDSTONE)
    _ADDOBJECT(EQP_VACUUM_DRIVE_S3)
    _ADDOBJECT(EQP_ZERO_ARMOR_S3)
    _ADDOBJECT(EQP_SHIELD_GENERATOR4_S3)

    _ADDOBJECT(GL_M4_S_FIRST2)
    _ADDOBJECT(EQP_VACUUM_DRIVE_S4)
    //_ADDOBJECT(EQP_MEZON_REACTOR_S4)
    //_ADDOBJECT(EQP_GLUON_REACTOR_S1)
    _ADDOBJECT(EQP_ZERO_ARMOR_S4)
    _ADDOBJECT(EQP_SHIELD_GENERATOR4_S4)
    _ADDOBJECT(GUN_MICROWAVE_OSCILLATOR)
    _ADDOBJECT(GUN_RAILGUN)

    _ADDRATING(300000000)
    _ADDBALANCE(30000000)

    _SETEVENT(SECTOR1.VISIT)
    _SETEVENT(SECTOR1.ACCESS)
    _SETEVENT(SECTOR2.VISIT)
    _SETEVENT(SECTOR2.ACCESS)
    _SETEVENT(SECTOR3.VISIT)
    _SETEVENT(SECTOR3.ACCESS)
    _SETEVENT(SECTOR4.VISIT)
    _SETEVENT(SECTOR4.ACCESS)
    _SETEVENT(SECTOR5.VISIT)
    _SETEVENT(SECTOR5.ACCESS)
    _SETEVENT(SECTOR6.VISIT)
    _SETEVENT(SECTOR6.ACCESS)
    _SETEVENT(SECTOR7.VISIT)
    _SETEVENT(SECTOR7.ACCESS)
    //_SETEVENT(SECTOR8.VISIT)
    _SETEVENT(SECTOR8.ACCESS)
)");*/

    // patch note: Release Manager
    // patch note: lz
    // patch note:
    // patch note: Have fun!
    mod.apply();

    // patch note:
    // patch note: References and Links
    // patch note: * Polygon-4 Project page https://github.com/aimrebirth
    // patch note: * Tools and Utilities https://github.com/aimrebirth/tools
    // patch note: * Custom mod example https://github.com/aimrebirth/tools/blob/master/examples/my_mod.cpp
    // discord (can expire)? telegram group?
    // patch note:
    // patch note: Hacking Section
    // patch note: If you want to build your own mod based on this, follow the next steps:
    // patch note: * Install the latest Visual Studio or Build Tools https://visualstudio.microsoft.com/downloads/
    // patch note: * Download and unpack sw tool into the PATH https://software-network.org/client/
    // patch note: * (Optional) You may also need archives '7z' tool installed and visible in PATH.
    // patch note: * Run the following command from console (fix the according cpp filename):
    // patch note: *   sw run modname.cpp
    // patch note: After this step the archive with your mod (modname.zip) will be created.
    // patch note:

    return 0;
}

#else // INJECTED_DLL

#include <stdint.h>
#include <windows.h>

constexpr auto call_command_length = 5;

enum aim1_fix : uint32_t {
    script_function__ISGLIDER = 0x0043A1F6,
};
uint32_t known_caller;

__declspec(naked) void fix_script_function__ISGLIDER() {
    __asm {
        ; restore values
        popad
        pushad

        mov edi, eax; my glider
        mov esi, [esp + 20h + 4 + 8 * 4]; arg

        label_cmp :
        mov al, [edi]
            mov cl, [esi]
            cmp al, cl
            jne bad
            cmp al, 0
            je end
            cmp cl, 0
            je end
            inc edi
            inc esi
            jmp label_cmp
            end :
        cmp al, cl
            jne bad
            popad
            ; push some regs as required by success branch
            push    ebx
            push    ebp
            push    esi
            push    edi
            push 0x0043A2DA
            ret

            bad :
        popad
            ; popf
            ; original thunk
            copy :
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
    default:
        break;
    }
    // just return
    __asm {
        popa
        push known_caller
        ret
    }
}

void setup() {
    constexpr uint32_t free_data_base = 0x006929C0;
    auto mem = (uint8_t *)free_data_base;
    mem[0] = 0xE9;
    *(uint32_t *)&mem[1] = (uint32_t)&dispatcher - free_data_base - call_command_length;
}
BOOL WINAPI DllMain(HINSTANCE, DWORD fdwReason, LPVOID) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        setup();
    }
    return TRUE;
}

#endif
