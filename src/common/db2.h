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

#include "common.h"
#include "mmap.h"

struct db2 {
    using char20 = char[0x20];

    enum class field_type : uint32_t {
        string,
        integer,
        float_,
    };

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
            field_type type;
        };

        uint32_t n_tables;
        uint32_t n_fields;

        auto tables() {
            auto base = (table *)(&n_fields + 1);
            return std::span{base, base + n_tables};
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

    path fn;
    primitives::templates2::mmap_file<uint8_t> fdat, find, ftab;
    tab *tab_{};
    ind *ind_{};
    dat *dat_{};

    db2() = default;
    db2(const path &fn) : fn{fn} {
        open(fn, primitives::templates2::mmap_file<uint8_t>::ro{});
    }

    void open(const path &fn, auto mode) {
        close();
        this->fn = fn;
        fdat.open(path{fn} += ".dat", mode);
        find.open(path{fn} += ".ind", mode);
        ftab.open(path{fn} += ".tab", mode);

        dat_ = (dat *)find.p;
        ind_ = (ind *)find.p;
        tab_ = (tab *)ftab.p;
    }
    void open(auto mode) {
        open(fn, mode);
    }
    void close() {
        fdat.close();
        find.close();
        ftab.close();
    }

    void add_value(std::string_view table, std::string_view value, auto && ... fields1) {
        auto tbl = tab_->tables();
        auto fields = tab_->fields();
        auto values = ind_->values();

        auto calc_fields_size = [](this auto &&f, std::string_view field_name, auto &&n, auto &&v, auto &&...fields) {
            if (field_name == n) {
                if constexpr (std::same_as<std::decay_t<decltype(v)>, int>) {
                    return sizeof(db2::dat::field_value_base) + sizeof(int);
                } else if constexpr (std::same_as<std::decay_t<decltype(v)>, float>) {
                    return sizeof(db2::dat::field_value_base) + sizeof(float);
                } else {
                    auto s = str2str(v, CP_UTF8, 1251);
                    return sizeof(db2::dat::field_value_base) + s.size() + 1;
                }
            }
            if constexpr (sizeof...(fields)) {
                return f(field_name, fields...);
            }
            if constexpr (std::same_as<std::decay_t<decltype(v)>, int>) {
                return sizeof(db2::dat::field_value_base) + sizeof(int);
            } else if constexpr (std::same_as<std::decay_t<decltype(v)>, float>) {
                return sizeof(db2::dat::field_value_base) + sizeof(float);
            } else {
                return sizeof(db2::dat::field_value_base) + 1;
            }
        };
        auto write_fields = [](this auto &&f, auto &&p, auto &&field, std::string_view field_name, auto &&n, auto &&v, auto &&...fields) {
            if (field_name == n) {
                if constexpr (std::same_as<std::decay_t<decltype(v)>, int>) {
                    if (field.type != db2::field_type::integer) {
                        throw std::runtime_error{"field type mismatch"};
                    }
                    (*(db2::dat::field_value_base*)p).field_id = field.id;
                    (*(db2::dat::field_value_base*)p).size = sizeof(int);
                    p += sizeof(db2::dat::field_value_base);
                    *(int*)p = v;
                    p += sizeof(int);
                    return;
                } else if constexpr (std::same_as<std::decay_t<decltype(v)>, float>) {
                    if (field.type != db2::field_type::float_) {
                        throw std::runtime_error{"field type mismatch"};
                    }
                    (*(db2::dat::field_value_base *)p).field_id = field.id;
                    (*(db2::dat::field_value_base *)p).size = sizeof(float);
                    p += sizeof(db2::dat::field_value_base);
                    *(float *)p = v;
                    p += sizeof(float);
                    return;
                } else {
                    if (field.type != db2::field_type::string) {
                        throw std::runtime_error{"field type mismatch"};
                    }
                    auto s = str2str(v, CP_UTF8, 1251);
                    (*(db2::dat::field_value_base *)p).field_id = field.id;
                    (*(db2::dat::field_value_base *)p).size = s.size() + 1;
                    p += sizeof(db2::dat::field_value_base);
                    memcpy(p, s.data(), s.size());
                    p[s.size()] = 0;
                    p += s.size() + 1;
                    return;
                }
            }
            if constexpr (sizeof...(fields)) {
                return f(p, field, field_name, fields...);
            }
            if constexpr (std::same_as<std::decay_t<decltype(v)>, int>) {
                (*(db2::dat::field_value_base *)p).field_id = field.id;
                (*(db2::dat::field_value_base *)p).size = 0;
                p += sizeof(db2::dat::field_value_base);
                return;
            } else if constexpr (std::same_as<std::decay_t<decltype(v)>, float>) {
                (*(db2::dat::field_value_base *)p).field_id = field.id;
                (*(db2::dat::field_value_base *)p).size = 0;
                p += sizeof(db2::dat::field_value_base);
                return;
            } else {
                (*(db2::dat::field_value_base *)p).field_id = field.id;
                (*(db2::dat::field_value_base *)p).size = 1;
                p += sizeof(db2::dat::field_value_base);
                p[1] = 0;
                p += 1;
                return;
            }
        };

        auto it = std::ranges::find_if(tbl, [&](auto &v){return v.name == table;});
        if (it == tbl.end()) {
            throw std::runtime_error{"no such table"};
        }
        auto &t = *it;
        auto itv = std::ranges::find_if(values, [&](auto &v){return v.table_id == t.id && value == v.name;});
        if (itv == values.end()) {
            db2::ind::value i{};
            i.table_id = t.id;
            memcpy(i.name, value.data(), value.size());
            i.offset = fdat.sz;
            for (auto &&f : fields) {
                if (f.table_id != t.id) {
                    continue;
                }
                std::string_view fn = f.name;
                i.size += calc_fields_size(fn, fields1...);
            }

            ++ind_->n_values;
            auto p = find.alloc_raw(find.sz + sizeof(i));
            memcpy(p, &i, sizeof(i));

            p = fdat.alloc_raw(fdat.sz + i.size);
            for (auto &&f : fields) {
                if (f.table_id != t.id) {
                    continue;
                }
                std::string_view fn = f.name;
                write_fields(p, f, fn, fields1...);
            }
        }
    }
};
