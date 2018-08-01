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

#include "save.h"

#include <primitives/filesystem.h>
#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>

#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
    cl::opt<path> p(cl::Positional, cl::desc("<file.sav or saves dir>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    //save_changes.mech_org = "ORG_PLAYER";
    save_changes.money = 999999999.0f;
    save_changes.upgrade_equ_for_player = true;

    auto func = [](auto &p)
    {
        buffer f(read_file(p));
        save_changes.out = buffer(f.buf());

        save s;
        s.load(f);

        writeFile(p.string(), save_changes.out.buf());
    };

    if (fs::is_regular_file(p))
        func(p);
    else if (fs::is_directory(p))
    {
        auto files = enumerate_files_like(p, ".*\\.sav", false);
        for (auto &f : files)
        {
            std::cout << "processing: " << f << "\n";
            func(f);
        }
    }
    else
        throw std::runtime_error("Bad fs object");

    return 0;
}
