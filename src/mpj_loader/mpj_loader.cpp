/*
 * AIM mpj_loader
 * Copyright (C) 2015 lzwdgc
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

#include "mpj.h"

#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>

#include <iostream>
#include <set>
#include <stdint.h>
#include <string>
#include <sstream>

using namespace std;

int main(int argc, char *argv[])
{
    cl::opt<path> p(cl::Positional, cl::desc("<file.mpj>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    mpj m;
    m.load(p);
    return 0;
}
