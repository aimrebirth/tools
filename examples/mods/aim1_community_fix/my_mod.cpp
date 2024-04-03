/*
name: aim_mod_maker
c++: 23
package_definitions: true
deps: pub.lzwdgc.Polygon4.Tools.aim1.mod_maker-master
*/

/*
 * Put this file near aim.exe.
 * Invoke mod creation using the following command:
 *    sw run my_mod.cpp
 * You can run the modded game now.
 * You can also distribute mod archive (requires 7z program).
 **/

// TODO:
/*
make patchnotes from code? not from comments
for example
comment(...)
or
patch_note(...)
*/

#define AIM_TYPES_FILE_NAME "aim.exe.h"
#define INJECTIONS_FILE_NAME "aim.exe.fixes.h"

#include INJECTIONS_FILE_NAME
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
// patch note: Or you can use mod_activator.exe, put it near aim.exe and drop mod archive
// patch note:  onto mod_activator.exe in explorer.
// patch note: You may need to run language_switcher.exe to select proper localization language.
// patch note:
// patch note: Game loads .pak files from the latest to oldest, so active mod .pak file must have
// patch note:  the latest timestamp on it.
// patch note: You should start new game after applying this mod. This is necessary for map changes
// patch note:  to became visible.
// patch note:
// patch note: Changes from 0.0.4
// patch note: fix Finder-2 textures
// patch note:
// patch note: Changes from 0.0.3
// patch note: add locale suffix to all quest databases (localized strings). Example: quest_ru_RU.*
// patch note: add en_US localization (quest database, quest_en_US.*)
// patch note: add language_switcher.exe. You can use it to switch game language
// patch note:  (only texts, but not menus). If you want your localization to be included into
// patch note:  this mod, contact mod author (lz) and send your quest.* files.
// patch note:  Run language_switcher.exe to select actual localization.
// patch note:
// patch note: Changes from 0.0.2
// patch note: correctly use .mmo files from the last patch instead of v1.0.0 (from res3.pak)
// patch note: add Finder-2 glider model from aim2
// patch note: db changes for double weapon gliders
// patch note:
// patch note: Changes from 0.0.1
// patch note: enable double weapon gliders
// patch note:

int main(int argc, char *argv[]) {
    mod_maker mod(
#ifdef NDEBUG
        "community_fix"s
#else
        "test_mod"s
#endif
        + "-0.0.5"s
    );
    mod.add_code_file_for_archive(INJECTIONS_FILE_NAME);
    mod.add_code_file_for_archive(AIM_TYPES_FILE_NAME);
    // this mod uses aim2 files (copy finder from there), so we must set up its path
    mod.setup_aim2_path();
    mod.files_to_distribute.insert("language_switcher.exe");

    // patch note: === LIST OF CHANGES ===
    // patch note:
    // patch note: General Notes
    // patch note: enabled WIN key during the game (Solant)
    mod.enable_win_key();
    // patch note:

    // patch note: Volcano Sector
    // patch note: rename second FINSWIL-1 to FINSWIL-3 to make this group appear on the map (Streef)
    mod.patch<uint8_t>("location4.mmo", 0x0007f9c1, '1', '3');
    // patch note: make SWIRE appear (Streef)
    mod.patch_after_pattern<uint8_t>("location4.mmo", "SWIRE\0"s, 0x100, 1, 0);
    // patch note: make SWILHUN appear (Streef)
    mod.patch_after_pattern<uint8_t>("location4.mmo", "SWILHUN\0"s, 0x120, 1, 0);
    // patch note: reposition SWILHUN-3 group (Streef)
    mod.patch_after_pattern<uint8_t>("location4.mmo", "SWILHUN-3"s, 0xB8, 0x40, 0xB1);
    // patch note: reposition SWILHUN-1 group (Streef)
    mod.patch_after_pattern<uint8_t>("location4.mmo", "SWILHUN-1"s, 0xB8, 0xA1, 0xA0);
    // patch note:

    // patch note: Desert Sector
    // patch note: reposition SACREFI-2 (Streef)
    mod.patch_after_pattern<uint8_t>("location5.mmo", "SACREFI-2", 0xB8, 0x18, 0x17);
    // patch note: fix 'TOV_POLYMER_PLATE' spawn (Streef)
    mod.replace("location5.scr", "TOV_POLYMER_PLATES", "TOV_POLYMER_PLATE");
    // patch note:

    // patch note: Hills Sector
    // patch note: allow to buy double heavy weapon Finder-2 glider on Finders base after the second quest. You must start the new game to make it appear (lz)
    mod.add_map_good("location6.mmo", "B_L6_IK_FINDER", "GL_S3_PS_FINDER1", mmo_storage2::map_good("GL_S3_PS_FINDER2"
#ifdef NDEBUG
        , "T_L6_IK_F2.COMPLETE"
#endif
    ));
    // patch note: add Finder-2 model and textures from aim2 game (lz)
    mod.copy_from_aim2("MOD_GL_S3_PS_FINDER2");
    // patch note: set correct model for a plant (Streef)
    mod.patch<uint8_t>("location6.mmo", 0x0005775D, 'R', 'F');
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
    mod.make_injection(aim1_fix::script_function__ISGLIDER);
    // end of scripts section
    // patch note:

    // patch note: Database Changes
    // patch note: DB
    auto db = mod.db();
    // patch note: set glider GL_S3_PS_FINDER2 model to MOD_GL_S3_PS_FINDER2 (lz)
    db["Глайдеры"]["GL_S3_PS_FINDER2"]["MODEL"] = "MOD_GL_S3_PS_FINDER2";
    // patch note: copy MOD_GL_S3_PS_FINDER2 model from aim2 (lz)
    // patch note: copy MOD_GL_S3_PS_FINDER2 textures data from aim2 (lz)
    // patch note: double gun for config CFG_NARGOON (double electro discharge) (lz)
    auto &tblcfg = db["Конфигурации"];
    tblcfg["CFG_NARGOON"]["HEAVYGUN1"] = "GUN_ELECTRO_DISCHARGER";
    // patch note: double gun for config CFG_NARGOON1 (double two-barreled atomic gun) (lz)
    tblcfg["CFG_NARGOON1"]["HEAVYGUN1"] = "GUN_DOUBLE_BARRELED_ATOMIC_GUN";
    // patch note: double gun for config CFG_BASE_NARG - Nargoon (double two-barreled atomic gun) (lz)
    tblcfg["CFG_BASE_NARG"]["HEAVYGUN1"] = "GUN_DOUBLE_BARRELED_ATOMIC_GUN";
    // patch note: double gun for config CFG_STNAR-97 - Nargoon (double GUN_INFRAATOMIC_PLASMA_GUN) (lz)
    tblcfg["CFG_STNAR-97"]["HEAVYGUN1"] = "GUN_INFRAATOMIC_PLASMA_GUN";
    // patch note: double gun for config CFG_FINDER_1 (std.3): from GUN_MICROWAVE_OSCILLATOR (std.4) and GUN_CHAOS_GENERATOR (std.4) to double GUN_FOUR_BARRELED_IMP_GAZER (std.3) (lz)
    auto &finder1 = tblcfg["CFG_FINDER_1"];
    finder1["LIGHTGUN1"] = "GUN_FOUR_BARRELED_IMP_GAZER";
    finder1["HEAVYGUN1"] = "GUN_FOUR_BARRELED_IMP_GAZER";
    // patch note: double gun for config CFG_FINDER_2: from GUN_FOUR_BARRELED_IMP_GAZER (std.3) + GUN_POZITRON_EMITTER (std.4) to double GUN_TACHYON_HEATER (std.3) (lz)
    auto &finder2 = tblcfg["CFG_FINDER_2"];
    finder2["LIGHTGUN1"] = "GUN_TACHYON_HEATER";
    finder2["HEAVYGUN1"] = "GUN_TACHYON_HEATER";
    // patch note: double gun for config CFG_EYEDSTONE_1: from GUN_FAST_ELECTROMAGNETIC_BEAM to double GUN_FAST_ELECTROMAGNETIC_BEAM (lz)
    tblcfg["CFG_EYEDSTONE_1"]["LIGHTGUN1"] = "GUN_FAST_ELECTROMAGNETIC_BEAM";
    // patch note: double gun for config CFG_EYEDSTONE_2: from GUN_FAST_ELECTROMAGNETIC_BEAM to double GUN_FAST_ELECTROMAGNETIC_BEAM (lz)
    tblcfg["CFG_EYEDSTONE_2"]["LIGHTGUN1"] = "GUN_FAST_ELECTROMAGNETIC_BEAM";
    // end of db changes
#ifdef NDEBUG
    db.write();
#endif
    // patch note: INFORMATION
    {
        auto quest = mod.quest("ru_RU");
        // patch note: add name for SINIGR armor, it was unnamed before (lz)
        quest["INFORMATION"]["EQP_ZERO_ARMOR_S_SIN"]["NAME"] = "Особая нуль-броня";
    }
    {
        auto quest = mod.quest("en_US");
        quest["INFORMATION"]["EQP_ZERO_ARMOR_S_SIN"]["NAME"] = "Special zero armor";
    }
    // more known langs: cs_CZ, de_DE, et_EE, fr_FR
    // you can find vanilla dbs here (not sure if it is 1.00 or 1.03, probably 1.00):
    // Supercluster discord
    // https://discord.gg/CFFKpTwYZD
    // https://discord.com/channels/463656710666584064/516316063747538945/615934366593581067
    // patch note:

    // patch note: Game Changes
    // patch note: enable double weapon gliders (lz)
    // patch note:    double light weapons: GL_M2_PA_NARGOON and GL_S3_PS_FINDER1
    // patch note:    double heavy weapons: GL_M3_PA_EYEDSTONE and GL_S3_PS_FINDER2
    // patch note:    (still have many bugs related)
    mod.make_injection(aim1_fix::trade_actions_weapon_checks);
    mod.make_injection(aim1_fix::setup_proper_weapon_slots_for_a_glider);
    mod.make_injection(aim1_fix::put_weapon_into_the_right_slot_after_purchase);
    mod.make_injection(aim1_fix::sell_correct_weapon);
    mod.make_injection(aim1_fix::empty_light_weapon_message);
    mod.make_injection(aim1_fix::empty_heavy_weapon_message);
    mod.make_injection(aim1_fix::can_leave_trade_window);
    // patch note:

    // test scripts
#ifndef NDEBUG
    // TODO: copy whole sector?

    // patch note dev: Developer Mode!!!
    // patch note dev: enabled developer mode (free camera - F3 key, time shift - N key) (lz, Solant)
    mod.enable_free_camera();
    // patch note dev: make initial reactor (EQP_GLUON_REACTOR_S1) and drive (EQP_ION_DRIVE_S1) more powerful
    db["Оборудование"]["EQP_GLUON_REACTOR_S1"]["VALUE1"] = 9'000'000.f;
    db["Оборудование"]["EQP_ION_DRIVE_S1"]["VALUE1"] = 4158000.f;
    // patch note dev: make EQP_VACUUM_DRIVE_S4 more powerful
    db["Оборудование"]["EQP_VACUUM_DRIVE_S4"]["VALUE1"] = 4158000.f;
    // end of db changes in dev mode
    auto m2_gliders = mod.open_aim2_db()["Глайдеры"];
    for (auto &&[n,_] : db["Глайдеры"]) {
        m2_gliders.erase(n);
    }
    m2_gliders.erase("GL_BOT");
    m2_gliders.erase("GL_RACE1");
    db.write();
    // patch note dev: copy gliders from m2: GL_M4_C_MASTODON, GL_M4_S_FLASH, GL_M4_A_FORWARD, GL_M4_A_FORWARD_BLACK
    auto add_glider = [&, after = "GL_M1_A_ATTACKER"s](auto &&name) mutable {
        mod.copy_glider_from_aim2(name);
        after = mod.add_map_good("location1.mmo", "B_L1_BASE1", after, mmo_storage2::map_good(name));
    };
    for (auto &&[n, _] : m2_gliders) {
        // TODO: cannot escape from glider menu with one of aim2 gliders for some reason
        //add_glider(n);
    }
    add_glider("GL_M4_C_MASTODON");
    add_glider("GL_M4_A_FORWARD");
    add_glider("GL_M4_A_FORWARD_BLACK");
    add_glider("GL_M4_S_FLASH");
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
    //_ADDOBJECT(GUN_RAILGUN)
    _ADDOBJECT(GUN_IMPULSE_MEGALAZER)

    _ADDOBJECT(EQP_ANALYZER)
    _ADDOBJECT(EQP_QUANTUM_TRANSLATOR)
    _ADDOBJECT(EQP_INVISIBILITY_SHIELD)
    _ADDOBJECT(EQP_UNIVERSAL_SHIELD)

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
#endif
