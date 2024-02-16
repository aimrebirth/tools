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
    cl::opt<path> db_fn(cl::Positional, cl::desc("<db file>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    db2 db{db_fn};
    auto f = db.open();

    auto tbl2 = f.find_table(u8"Глайдеры");
    auto valuexxx = tbl2.find_value("GL_S3_PS_FINDER2");
    auto valuexxx2 = tbl2.find_value("GL_TEST_XXX");
    valuexxx2.set_field("NAME", "X");
    valuexxx2.set_field("NAME", "X");
    valuexxx2.set_field("NAME", "X");
    valuexxx2.set_field("NAME", "X");
    valuexxx2.set_field("NAME", "X");
    valuexxx2.set_field("NA", "X");
    valuexxx2.set_field("VW", 5);
    valuexxx2.set_field("VW2", 5.3f);
    valuexxx2.set_field("VW2", 6.3f);
    valuexxx2.set_field("VW2", 6.3f);
    valuexxx2.set_field("VW2", 6.3f);
    valuexxx2.set_field("VW2", 6.3f);

    f(u8"Глайдеры", "GL_S3_PS_FINDER2", "NAME") = "xx";
    f(u8"Глайдеры", "GL_S3_PS_FINDER2", "VW2") = 4.3f;

    auto tbl = f.tab_.data->tables();
    auto fields = f.tab_.data->fields();
    auto values = f.ind_.data->values();

    std::string spaceval(4, ' ');
    std::string spacefield(8, ' ');
    for (auto &&t : tbl) {
        std::println("{}:", t.name);
        for (auto &&v : values | std::views::filter([&](auto &v){return v.table_id == t.id;})) {
            std::println("{}{}:", spaceval, v.name);
            auto max = f.dat_.f.p + v.offset + v.size;
            auto p = f.dat_.f.p + v.offset;
            while (p < max) {
                auto vb = (db2::dat::field_value_base*)p;
                p += sizeof(db2::dat::field_value_base);
                auto f = std::ranges::find_if(fields, [&](auto &f){return f.table_id == t.id && f.id == vb->field_id;});
                if (f == fields.end()) {
                    continue;
                }
                switch (f->type) {
                case db2::field_type::integer: {
                    auto fv = (int*)p;
                    p += vb->size;
                    std::println("{}{}: {}", spacefield, f->name, *fv);
                    break;
                }
                case db2::field_type::float_: {
                    auto fv = (float*)p;
                    p += vb->size;
                    std::println("{}{}: {}", spacefield, f->name, *fv);
                    break;
                }
                case db2::field_type::string: {
                    auto fv = (const char*)p;
                    p += vb->size;
                    std::println("{}{}: {}", spacefield, f->name, fv);
                    break;
                }
                default: {
                    break;
                }
                }
            }
        }
    }

    return 0;
}
