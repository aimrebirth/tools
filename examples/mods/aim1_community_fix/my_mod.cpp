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
make doc/patchnotes from code? not from comments
for example
comment(...)
or
patch_note(...)
*/

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
// patch note: add more translations. Use language_switcher.exe to change your language.
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
    mod.add_map_good("location6.mmo", "B_L6_IK_FINDER", "GL_S3_PS_FINDER1",
                     mmo_storage2::map_good("GL_S3_PS_FINDER2", "T_L6_IK_F2.COMPLETE"));
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
    mod.make_script_engine_injections();
    // end of scripts section
    // patch note:

    // patch note: Database Changes
    // patch note: DB
    auto &db = mod.db();
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

    // patch note: INFORMATION
    auto &quest = mod.quest();
    // patch note: add name for SINIGR armor, it was unnamed before (lz)
    quest["ru_RU"]["INFORMATION"]["EQP_ZERO_ARMOR_S_SIN"]["NAME"] = "Особая нуль-броня";
    quest["en_US"]["INFORMATION"]["EQP_ZERO_ARMOR_S_SIN"]["NAME"] = "Special zero armor";
    // patch note:

    // patch note: Game Changes
    // patch note: enable double weapon gliders (lz)
    // patch note:    double light weapons: GL_M2_PA_NARGOON and GL_S3_PS_FINDER1
    // patch note:    double heavy weapons: GL_M3_PA_EYEDSTONE and GL_S3_PS_FINDER2
    // patch note:    (still have many bugs related)
    mod.make_double_weapon_injections();
    // patch note:

    // test scripts
#if !defined(NDEBUG)
    // TODO: copy whole sector?

    // patch note dev: Developer Mode!!!
    // patch note dev: enabled developer mode (free camera - F3 key, time shift - N key) (lz, Solant)
    mod.enable_free_camera();
    auto add_test_eqp = [&](std::string type, std::string copy_from
        , std::optional<float> value1 = {}
        , std::optional<float> value2 = {}
        , std::optional<float> value3 = {}
        ) {
        auto test_name = "EQP_"s + type + "_TEST";
        if (type == "ENGINE") {
            test_name = "EQP_DRIVE_TEST";
        }
        db["Оборудование"][test_name] = db["Оборудование"][copy_from];
        db["Оборудование"][test_name]["STANDARD"] = 1;
        if (value1) {
            db["Оборудование"][test_name]["VALUE1"] = *value1;
        }
        if (value2) {
            db["Оборудование"][test_name]["VALUE2"] = *value2;
        }
        if (value3) {
            db["Оборудование"][test_name]["VALUE3"] = *value3;
        }
        db["Конфигурации"]["CFG_STARTUP"][type] = test_name;
    };
    // patch note dev: give glider FLASH
    db["Конфигурации"]["CFG_STARTUP"]["GLIDER"] = "GL_M4_S_FLASH";
    // patch note dev: give powerful reactor
    add_test_eqp("REACTOR", "EQP_GLUON_REACTOR_S1", 9'000'000.f);
    // patch note dev: give powerful engine
    add_test_eqp("ENGINE", "EQP_ION_DRIVE_S1", 4158000.f);
    // patch note dev: give powerful armor
    add_test_eqp("ARMOR", "EQP_ZERO_ARMOR_S1", 100000.f, {}, 1000.f); // with regen
    // patch note dev: give powerful shield
    add_test_eqp("SHIELD", "EQP_SHIELD_GENERATOR1_S1", 100000.f, {}, 1000.f); // with regen
    // patch note dev: give powerful weapons
    db["Оружие"]["GUN_MICROWAVE_OSCILLATOR_TEST"] = db["Оружие"]["GUN_MICROWAVE_OSCILLATOR"];
    db["Оружие"]["GUN_MICROWAVE_OSCILLATOR_TEST"]["DAMAGE"] = 2500.f;
    db["Оружие"]["GUN_MICROWAVE_OSCILLATOR_TEST"]["STANDARD"] = 1;
    db["Конфигурации"]["CFG_STARTUP"]["LIGHTGUN1"] = "GUN_MICROWAVE_OSCILLATOR_TEST";
    // patch note dev: give IMPULSE MEGALAZER
    db["Оружие"]["GUN_IMPULSE_MEGALAZER_TEST"] = db["Оружие"]["GUN_IMPULSE_MEGALAZER"];
    db["Оружие"]["GUN_IMPULSE_MEGALAZER_TEST"]["DAMAGE"] = 40000.f;
    db["Оружие"]["GUN_IMPULSE_MEGALAZER_TEST"]["STANDARD"] = 1;
    db["Конфигурации"]["CFG_STARTUP"]["HEAVYGUN1"] = "GUN_IMPULSE_MEGALAZER_TEST";
    // end of db changes in dev mode
    // patch note dev: allow to buy GL_S3_PS_FINDER1 without pre-quest
    mod.add_map_good("location6.mmo", "B_L6_IK_FINDER", "GL_S3_PS_FINDER1", mmo_storage2::map_good("GL_S3_PS_FINDER1"));
    // patch note dev: allow to buy GL_S3_PS_FINDER2 without pre-quest
    mod.add_map_good("location6.mmo", "B_L6_IK_FINDER", "GL_S3_PS_FINDER2", mmo_storage2::map_good("GL_S3_PS_FINDER2"));
    // patch note dev: allow to buy GL_M3_PA_EYEDSTONE without joining the clan
    mod.add_map_good("location4.mmo", "B_L4_TRAIN_STONES", "GL_M3_PA_EYEDSTONE", mmo_storage2::map_good("GL_M3_PA_EYEDSTONE"));
    // patch note dev: copy all new gliders from AIM2
    auto m2_gliders = mod.open_aim2_db().at("Глайдеры");
    for (auto &&[n,_] : db["Глайдеры"]) {
        m2_gliders.erase(n);
    }
    for (auto &&[n, _] : m2_gliders) {
        mod.copy_glider_from_aim2(n);
    }
    std::string after;
    for (after = "GL_M1_A_ATTACKER"s; auto &&[n, _] : db["Глайдеры"]) {
        after = mod.add_map_good("location1.mmo", "B_L1_BASE1", after, mmo_storage2::map_good(n));
    }
    for (after = "GUN_RAY_LAZER"s; auto &&[n, _] : db["Оружие"]) {
        after = mod.add_map_good("location1.mmo", "B_L1_BASE1", after, mmo_storage2::map_good(n));
    }
    // patch note dev: copy GUN_DRAINER and GUN_GRAVITON from AIM2. They crash frequently while in F2 mode. Some does not have fx.
    // TODO: check in debug why they crash
    mod.copy_weapon_from_aim2("GUN_DRAINER");
    after = mod.add_map_good("location1.mmo", "B_L1_BASE1", after, mmo_storage2::map_good("GUN_DRAINER"));
    mod.copy_weapon_from_aim2("GUN_GRAVITON");
    after = mod.add_map_good("location1.mmo", "B_L1_BASE1", after, mmo_storage2::map_good("GUN_GRAVITON"));

    // does not work, crashes. Maybe different item size
    // or maybe too many goods
    /*for (auto after = "EQP_POLYMER_ARMOR_S1"s; auto &&[n, _] : db["Оборудование"]) {
        if (!(false
            || n.contains("_DRIVE_")
            || n.contains("_REACTOR_")
            || n.contains("_ARMOR_")
            || n.contains("_SHIELD_G")
            )) {
            continue;
        }
        after = mod.add_map_good("location1.mmo", "B_L1_BASE1", after, mmo_storage2::map_good(n));
    }*/
    // patch note dev: start money, rating, glider and sector access
    mod.replace("Script/bin/B_L1_BASE1.scr", "_ADDOBJECT(EQP_TITANIUM_ARMOR_S1)", "//_ADDOBJECT(EQP_TITANIUM_ARMOR_S1)");
    mod.replace("Script/bin/B_L1_BASE1.scr", "_ADDOBJECT(EQP_SHIELD_GENERATOR1_S1)", "//_ADDOBJECT(EQP_SHIELD_GENERATOR1_S1)");
    mod.replace("Script/bin/B_L1_BASE1.scr", "_ADDOBJECT(GUN_PULSE_LAZER)", "//_ADDOBJECT(GUN_PULSE_LAZER)");
    mod.replace("Script/bin/B_L1_BASE1.scr", "_ADDOBJECT(AMM_FAST_BOMB,5)", "//_ADDOBJECT(AMM_FAST_BOMB,5)");
    //
    mod.replace("Script/bin/B_L1_BASE1.scr", "_ADDBALANCE(300)", R"(
    _ADDBALANCE(300)

    //_ADDOBJECT(GL_S3_PS_FINDER1)
    //_ADDOBJECT(EQP_VACUUM_DRIVE_S3)
    //_ADDOBJECT(EQP_ZERO_ARMOR_S3)
    //_ADDOBJECT(EQP_SHIELD_GENERATOR4_S3)

    //_ADDOBJECT(GL_M4_S_FIRST2)
    //_ADDOBJECT(EQP_VACUUM_DRIVE_S4)
    //_ADDOBJECT(EQP_MEZON_REACTOR_S4)
    //_ADDOBJECT(EQP_GLUON_REACTOR_S1)
    //_ADDOBJECT(EQP_ZERO_ARMOR_S4)
    //_ADDOBJECT(EQP_SHIELD_GENERATOR4_S4)
    //_ADDOBJECT(GUN_MICROWAVE_OSCILLATOR)
    //_ADDOBJECT(GUN_RAILGUN)
    //_ADDOBJECT(GUN_IMPULSE_MEGALAZER)

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

