/*
name: aim_mod_maker
c++: 23
package_definitions: true
deps: pub.lzwdgc.Polygon4.Tools.aim1_mod_maker-master
*/

/*
 * Put this file near aim.exe.
 * Invoke mod creation using the following command:
 *    sw run my_mod.cpp
 * You can run the modded game now.
 * You can also distribute mod archive (requires 7z program).
 **/

#define AIM_TYPES_FILE_NAME "aim.exe.h"
#define INJECTIONS_FILE_NAME "aim.exe.fixes.h"

#ifndef INJECTED_DLL
#include "aim1_mod_maker.h"

// patch note: Authors and Credits
// patch note: lz, Solant, Streef
// patch note:
// patch note: Description
// patch note: This mod fixes some issues with the original AIM v1.06 (DRM-free) (1.0.6.3) game.
// patch note:
// patch note: Installation
// patch note: Unpack and drop all files near original aim.exe. Replace files if necessary.
// patch note:

int main(int argc, char *argv[]) {
    mod_maker mod(
#ifdef NDEBUG
        "community_fix"s
#else
        "test_mod"s
#endif
        + "-0.0.2"s
    );
    mod.add_code_file_for_archive(INJECTIONS_FILE_NAME);
    mod.add_code_file_for_archive(AIM_TYPES_FILE_NAME);

    // patch note: CHANGES
    // patch note:
    // patch note: General Notes
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
    // patch note: allow to buy double heavy weapon Finder-2 glider on Finders base after the second quest. You must start the new game to make it appear (lz)
    mod.add_map_good("location6.mmo", "B_L6_IK_FINDER", "GL_S3_PS_FINDER1", R"(
47 4c 5f 53 33 5f 50 53 5f 46 49 4e 44 45 52 32
00 d2 e2 77 42 04 06 00 35 01 00 00 76 0c 01 30
54 5f 4c 36 5f 49 4b 5f 46 32 2e 43 4f 4d 50 4c
45 54 45 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00 00 00 00 00 00 00 00 00 00 00 00
)"_bin);
    // patch note: set correct model for a plant (Streef)
    mod.patch<uint8_t>("location6.mmo", 0x575DD, 'R', 'F');
    // patch note: fix 'TOV_POLYMER_PLATE' spawn (Streef)
    mod.replace("location6.scr", "TOV_POLYMER_PLATES", "TOV_POLYMER_PLATE");
    // patch note:

    // patch note: Script Changes
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

    // patch note: * _ISGLIDER() function can check exact glider name now, for example _ISGLIDER(GL_M3_A_FIRST1) (lz)
    mod.make_injection(0x0043A1F6, 10);
    // end of scripts section
    // patch note:

    // patch note: Database Changes
    // patch note: add name for SINIGR armor, it was unnamed before (lz)
    mod.db.quest().add_value("INFORMATION"sv, "EQP_ZERO_ARMOR_S_SIN"sv, "NAME", (const char *)u8"Особая нуль-броня");
    // patch note:

    // patch note: Game Changes
    // patch note: enable double weapon gliders (lz)
    // patch note:    double light weapons: GL_M2_PA_NARGOON and GL_S3_PS_FINDER1
    // patch note:    double heavy weapons: GL_M3_PA_EYEDSTONE and GL_S3_PS_FINDER2
    // patch note:    (still have many bugs related)
    mod.make_injection(0x004072FA); // can trade for buy purposes
    mod.make_injection(0x004D62E4); // setup proper weapon slots for a glider
    mod.make_injection(0x00417A6D); // put weapon into the right slot after purchase
    mod.make_injection(0x004176BC); // sell correct weapon
    mod.make_injection(0x004067C4); // empty light weap
    mod.make_injection(0x0040688B); // empty heavy weap
    // patch note:

    // test scripts
#ifndef NDEBUG
    // patch note dev: Developer Mode!!!
    // patch note dev: enabled developer mode (free camera - F3 key, time shift - N key) (lz, Solant)
    mod.enable_free_camera();
    // patch note dev: start money, rating, glider and sector access
    mod.replace("Script/bin/B_L1_BASE1.scr", "_ADDBALANCE(300)", R"(
    _ADDBALANCE(300 )

    //_ADDOBJECT(GL_S3_PS_FINDER1)
    //_ADDOBJECT(EQP_VACUUM_DRIVE_S3)
    //_ADDOBJECT(EQP_ZERO_ARMOR_S3)
    //_ADDOBJECT(EQP_SHIELD_GENERATOR4_S3)

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
)");
    // patch note dev:
#endif

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
#include INJECTIONS_FILE_NAME
#endif
