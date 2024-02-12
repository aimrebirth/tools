/*
 * AIM db_extractor
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

#include "db.h"

#include <buffer.h>
#include <common.h>
#include <mmap.h>
#include <mmap.h>

#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>

#include <fstream>
#include <span>
#include <print>

struct db2 {
    using char20 = char[0x20];

    // table structure
    struct tab {
        struct table {
            uint32_t id;
            char20 name;
            uint32_t unk;
        };
        struct field {
            uint32_t table_id;
            uint32_t id;
            char20 name;
            FieldType type;
        };

        uint32_t n_tables;
        uint32_t n_fields;

        auto tables() {
            auto base = (table *)(&n_fields+1);
            return std::span{base, base+n_tables};
        }
        auto fields() {
            auto table_base = (table *)(&n_fields + 1);
            auto base = (field *)(table_base + n_tables);
            return std::span{base, base + n_fields};
        }
    };
    // table values (index)
    struct ind {
        struct value {
            uint32_t table_id;
            char20 name;
            uint32_t offset;
            uint32_t size;
        };

        uint32_t n_values;

        auto values() {
            auto base = (value *)(&n_values + 1);
            return std::span{base, base + n_values};
        }
    };
    // field values
    struct dat {
        // NOTE: for some reason int fields can be != 4
        // so follow this size field
        struct field_value_base {
            uint32_t field_id;
            uint32_t size;
        };
    };

    primitives::templates2::mmap_file<uint8_t> fdat,find,ftab;
    tab *tab_;
    ind *ind_;
    dat *dat_;

    db2(const path &fn) {
        fdat.open(path{fn} += ".dat");
        find.open(path{fn} += ".ind");
        ftab.open(path{fn} += ".tab");

        dat_ = (dat *)find.p;
        ind_ = (ind *)find.p;
        tab_ = (tab *)ftab.p;
    }
};

int main(int argc, char *argv[])
{
    cl::opt<path> db_fn(cl::Positional, cl::desc("<db file>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    db2 db{db_fn};
    auto tbl = db.tab_->tables();
    auto fields = db.tab_->fields();
    auto values = db.ind_->values();

    std::string spaceval(4, ' ');
    std::string spacefield(8, ' ');
    for (auto &&t : tbl) {
        std::println("{}:", t.name);
        for (auto &&v : values | std::views::filter([&](auto &v){return v.table_id == t.id;})) {
            std::println("{}{}:", spaceval, v.name);
            auto max = db.fdat.p + v.offset + v.size;
            auto p = db.fdat.p + v.offset;
            while (p < max) {
                auto vb = (db2::dat::field_value_base*)p;
                p += sizeof(db2::dat::field_value_base);
                auto f = std::ranges::find_if(fields, [&](auto &f){return f.table_id == t.id && f.id == vb->field_id;});
                if (f == fields.end()) {
                    continue;
                }
                switch (f->type) {
                case FieldType::Integer: {
                    auto fv = (int*)p;
                    p += vb->size;
                    std::println("{}{}: {}", spacefield, f->name, *fv);
                    break;
                }
                case FieldType::Float: {
                    auto fv = (float*)p;
                    p += vb->size;
                    std::println("{}{}: {}", spacefield, f->name, *fv);
                    break;
                }
                case FieldType::String: {
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
