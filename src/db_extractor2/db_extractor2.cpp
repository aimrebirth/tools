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
#include <primitives/templates2/overload.h>
#include <nlohmann/json.hpp>

#include <fstream>
#include <span>
#include <print>

int main(int argc, char *argv[])
{
    cl::opt<path> db_fn(cl::Positional, cl::desc("<db file>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    db2 db{db_fn};
    auto f = db.open();
    auto tbl = f.to_map();

    nlohmann::json ja;
    for (auto &&[tn,t] : tbl) {
        auto &jt = ja[tn];
        for (auto &&[vn,v] : t) {
            auto &jv = jt[vn];
            for (auto &&[fn, fv] : v) {
                std::visit(overload{
                    [&](const int &v){jv[fn] = v;},
                    [&](const float &v){jv[fn] = v;},
                    [&](const std::string &v){jv[fn] = v;},
                    }, fv);
            }
        }
    }
    write_file(path{db_fn} += ".json", ja.dump(1));

    db2 x{path{db_fn} += "new"};
    auto newdb = x.open();
    for (auto &&[t,vals] : ja.items()) {
        for (auto &&[v,fields] : vals.items()) {
            for (auto &&[f,val] : fields.items()) {
                auto s = newdb(t, v, f);
                if (0) {
                } else if (val.is_number_float()) {
                    s = val.get<float>();
                } else if (val.is_number_integer()) {
                    s = val.get<int>();
                } else if (val.is_string()) {
                    s = val.get<std::string>();
                } else {
                    throw std::logic_error{"bad type"};
                }
            }
        }
    }

    return 0;
}
