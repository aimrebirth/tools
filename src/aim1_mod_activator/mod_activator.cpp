/*
 * AIM mod_activator
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

#include <primitives/pack.h>
#include <primitives/sw/main.h>
#include <primitives/sw/cl.h>

void activate(auto &&fn) {
    // files will get new timestamp automatically
    // our unpack_file() does not preserve timestamps
    unpack_file(fn, fs::current_path());
}

int main(int argc, char *argv[])
{
    cl::opt<path> p(cl::Positional, cl::desc("<mod .zip archive>"), cl::value_desc("file"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    if (fs::is_regular_file(p)) {
        activate(p);
    } else {
        throw std::runtime_error("Bad fs object");
    }

    return 0;
}
