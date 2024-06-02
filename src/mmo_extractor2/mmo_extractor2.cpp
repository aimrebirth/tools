/*
 * AIM mmo_extractor2 (only for aim1)
 * Copyright (C) 2024 lzwdgc
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

#include "mmo2.h"
#include "objects.h"

#include <primitives/sw/main.h>
#include <primitives/sw/cl.h>

void read_mmo(const path &fn)
{
    mmo_storage2 s{fn};
    s.load();
}

int main(int argc, char *argv[])
{
    cl::opt<path> p(cl::Positional, cl::desc("<.mmo file or directory with .mmo files>"), cl::value_desc("file or directory"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    if (fs::is_regular_file(p))
        read_mmo(p);
    else if (fs::is_directory(p))
    {
        auto files = enumerate_files_like(p, ".*\\.[Mm][Mm][Oo]", false);
        for (auto &file : files)
        {
            std::cerr << "processing: " << file << "\n";
            read_mmo(file);
        }
    }
    else
        throw std::runtime_error("Bad fs object");

    return 0;
}
