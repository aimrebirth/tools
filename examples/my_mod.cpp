/*
c++: 23
deps: pub.lzwdgc.Polygon4.Tools.aim1_mod_maker-master
*/

/*
 * Put this file near aim.exe.
 * Invoke mod creation using the following command:
 *    sw run my_mod.cpp
 * You can run the modded game now.
 **/

#include "aim1_mod_maker.h"

int main(int argc, char *argv[]) {
    mod_maker mod{ "my_mod" };

    mod.enable_free_camera();
    mod.enable_win_key();

    mod.patch<uint8_t>("location4.mmo", 0x7F599, 0x31, 0x33); // rename second FINSWIL-1 to FINSWIL-3 to make it appear
    mod.patch<uint8_t>("location4.mmo", 0x7FA34, 1, 0); // make SWIRE appear
    mod.patch<uint8_t>("location4.mmo", 0x7F913, 1, 0); // make SWILHUN appear
    mod.patch<uint8_t>("location4.mmo", 0x7F528, 0x40, 0xB1); // reposition SWILHUN-3
    mod.patch<uint8_t>("location4.mmo", 0x7EE62, 0xA1, 0xA0); // reposition SWILHUN-1
    mod.patch<uint8_t>("location5.mmo", 0xBAFF7, 0x18, 0x17); // reposition SACREFI-2
    mod.patch<uint8_t>("location6.mmo", 0x575DD, 'R', 'F'); // set correct model for a plant

    mod.replace("location5.scr", "TOV_POLYMER_PLATES", "TOV_POLYMER_PLATE");
    mod.replace("location6.scr", "TOV_POLYMER_PLATES", "TOV_POLYMER_PLATE");

    mod.replace("ORG_FIRST.scr", "IF(_PLAYERHAS(GL_M3_A_FIRST1)||_PLAYERHAS(GL_M3_A_FIRST1))", "IF(_PLAYERHAS(GL_M3_A_FIRST1)||_PLAYERHAS(GL_M3_A_FIRST2))");
    mod.replace("ORG_FIRST.scr", "IF(_PLAYERHAS(GL_M4_A_FIRST1)||_PLAYERHAS(GL_M4_A_FIRST1))", "IF(_PLAYERHAS(GL_M4_S_FIRST1)||_PLAYERHAS(GL_M4_S_FIRST2))");

    mod.replace("Script/bin/B_L1_BASE1.scr", "_ADDBALANCE(300)", R"(
    _ADDBALANCE(300 )

    //_ADDOBJECT(GL_M4_S_FIRST2)
    //_ADDOBJECT(EQP_VACUUM_DRIVE_S4)
    //_ADDOBJECT(EQP_MEZON_REACTOR_S4)
    //_ADDOBJECT(EQP_MESON_REACTOR_S4)
    //_ADDOBJECT(EQP_NUCLON_REACTOR_S4)
    //_ADDOBJECT(EQP_ZERO_ARMOR_S4)
    //_ADDOBJECT(EQP_SHIELD_GENERATOR4_S4)
    //_ADDOBJECT(GUN_MICROWAVE_OSCILLATOR)
    //_ADDOBJECT(GUN_RAILGUN)
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

    mod.apply();

    return 0;
}
