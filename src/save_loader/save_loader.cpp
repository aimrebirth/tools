/*
 * AIM save_loader
 * Copyright (C) 2016 lzwdgc
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>

#include "save.h"

int main(int argc, char *argv[])
try
{
    if (argc != 2)
    {
        printf("Usage: %s file.sav\n", argv[0]);
        return 1;
    }

    //save_changes.mech_org = "ORG_PLAYER";
    save_changes.money = 999999999.0f;
    save_changes.upgrade_equ_for_player = true;

    buffer f(readFile(argv[1]));
    save_changes.out = buffer(f.buf());

    save s;
    s.load(f);

    writeFile(argv[1], save_changes.out.buf());

    return 0;
}
catch (std::exception &e)
{
    printf("%s\n", argv[1]);
    printf("error: %s\n", e.what());
    return 1;
}
catch (...)
{
    printf("%s\n", argv[1]);
    printf("error: unknown exception\n");
    return 1;
}
