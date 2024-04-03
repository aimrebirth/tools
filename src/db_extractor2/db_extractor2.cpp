/*
 * AIM db_extractor
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

#include "db2.h"

#include <buffer.h>
#include <common.h>

#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>

#include <fstream>
#include <span>
#include <print>

int main(int argc, char *argv[])
{
    cl::opt<path> db_fn(cl::Positional, cl::desc("<db file or json file to backwards conversion>"), cl::Required);
    cl::opt<int> codepage(cl::Positional, cl::desc("<codepage>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    path fn = db_fn;
    fn = fs::absolute(fn);
    if (fn.extension() != ".json") {
        db2 db{fn};
        auto f = db.open();
        auto m = f.to_map(codepage);
        write_file(path{fn} += ".json", m.to_json().dump(1));
    } else {
        db2::files::db2_internal db;
        db.load_from_json(fn);
        db.save(fn.parent_path() / fn.stem(), codepage);
    }

    return 0;
}
