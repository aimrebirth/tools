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

void copy_dbs() {
    auto from_to = [](auto &&from, const std::string &to) {
        path base = from;
        base /= "data";
        const path out = "d:/dev/ue/AIM/translations/aim1/";
        db2 db{base / "quest"};
        auto f = db.create();
        f.open();
        auto m = f.to_map(code_pages.at(to.substr(0,2)));
        write_file(path{out} += "/quest_"s + to + ".json", m.to_json().dump(1));
    };
    from_to("h:/Games/AIM", "ru_RU");
    for (auto &&lang : {
        "cs_CZ",
        "de_DE",
        "en_US",
        "et_EE",
        "fr_FR",
        }) {
        from_to("h:/Games/AIM/1/dbs/"s + lang, lang);
    }
}

int main(int argc, char *argv[]) {
    //copy_dbs();

    cl::opt<path> db_fn(cl::Positional, cl::desc("<db file or json file to backwards conversion>"), cl::Required);
    cl::opt<int> codepage(cl::Positional, cl::desc("<codepage>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    path fn = db_fn;
    fn = fs::absolute(fn);
    if (fn.extension() != ".json") {
        db2 db{fn};
        auto f = db.create();
        f.open();
        auto m = f.to_map(codepage);
        write_file(path{fn} += ".json", m.to_json().dump(1));
    } else {
        db2::files_type::db2_internal db;
        db.load_from_json(fn);
        db.save(fn.parent_path() / fn.stem(), codepage);
    }

    return 0;
}
