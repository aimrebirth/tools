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

#include <primitives/templates2/type_name.h>

#include <variant>

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
        auto fields(int table_id) {
            auto table_base = (table *)(&n_fields + 1);
            auto base = (field *)(table_base + n_tables);
            return std::span{base, base + n_fields} | std::views::filter([=](auto &v){return v.table_id == table_id;});
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
        auto values(int table_id) {
            auto base = (value *)(&n_values + 1);
            return std::span{base, base + n_values} | std::views::filter([=](auto &v){return v.table_id == table_id;});
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
    int codepage{1251};

    template <typename T>
    struct file {
        path fn;
        primitives::templates2::mmap_file<uint8_t> f;
        T *data;

        file(auto &&base) : fn{path{base} += "."s} {
            fn += type_name<T>();
            f.open(fn, primitives::templates2::mmap_file<uint8_t>::rw{});
            data = (T *)f.p;
        }
    };
    // actual db
    struct files {
        db2 &db;
        file<tab> tab_;
        file<ind> ind_;
        file<dat> dat_;

        files(auto &&db, auto &&base) : db{db}, tab_{base}, ind_{base}, dat_{base} {}
        auto get_files() const {
            return std::set<path>{tab_.fn,ind_.fn,dat_.fn};
        }

        struct db2_internal {
            using db2_memory_value = std::variant<std::string, int, float>;
            using db2_memory = std::map<std::string, std::map<std::string, std::map<std::string, db2_memory_value>>>;

            db2_memory m;

            auto &operator[](this auto &&d, const std::string &s) {
                return d.m[s];
            }
            void save() {

            }
        };

        // converts string to utf8, trims them
        auto to_map() const {
            auto prepare_string = [](auto &&in) {
                auto s = str2utf8(in);
                boost::trim(s);
                return s;
            };

            db2_internal m;
            auto tbl = tab_.data->tables();
            for (auto &&t : tbl) {
                auto &jt = m[prepare_string(t.name)];
                auto fields = tab_.data->fields(t.id);
                for (auto &&v : ind_.data->values(t.id)) {
                    auto vn = prepare_string(v.name);
                    if (jt.contains(vn)) {
                        throw std::logic_error{"duplicate"};
                    }
                    auto &jv = jt[vn];
                    auto p = dat_.f.p + v.offset;
                    auto max = p + v.size;
                    while (p < max) {
                        auto vb = (db2::dat::field_value_base *)p;
                        p += sizeof(db2::dat::field_value_base);
                        auto f = std::ranges::find_if(fields, [&](auto &f) { return f.id == vb->field_id; });
                        if (f == fields.end()) {
                            throw std::logic_error{"unknown field"};
                        }
                        auto fn = prepare_string(f->name);
                        switch (f->type) {
                        case db2::field_type::integer:
                            jv[fn] = *(int *)p;
                            break;
                        case db2::field_type::float_:
                            jv[fn] = *(float *)p;
                            break;
                        case db2::field_type::string:
                            jv[fn] = prepare_string((const char *)p);
                            break;
                        default:
                            throw std::logic_error{"bad type"};
                        }
                        p += vb->size;
                    }
                }
            }
            return m;
        }
        /*auto to_json() const {
            auto prepare_string = [](auto &&in) {
                auto s = str2utf8(in);
                boost::trim(s);
                return s;
            };

            auto tbl = tab_.data->tables();
            nlohmann::json ja;
            for (auto &&t : tbl) {
                auto &jt = ja[prepare_string(t.name)];
                auto fields = tab_.data->fields(t.id);
                for (auto &&v : ind_.data->values(t.id)) {
                    auto vn = prepare_string(v.name);
                    if (jt.contains(vn)) {
                        throw std::logic_error{"duplicate"};
                    }
                    auto &jv = jt[vn];
                    auto p = dat_.f.p + v.offset;
                    auto max = p + v.size;
                    while (p < max) {
                        auto vb = (db2::dat::field_value_base *)p;
                        p += sizeof(db2::dat::field_value_base);
                        auto f = std::ranges::find_if(fields, [&](auto &f) {
                            return f.id == vb->field_id;
                        });
                        if (f == fields.end()) {
                            throw std::logic_error{"unknown field"};
                        }
                        auto fn = prepare_string(f->name);
                        switch (f->type) {
                        case db2::field_type::integer:
                            jv[fn] = *(int *)p;
                            break;
                        case db2::field_type::float_:
                            jv[fn] = *(float *)p;
                            break;
                        case db2::field_type::string:
                            jv[fn] = prepare_string((const char *)p);
                            break;
                        default:
                            throw std::logic_error{"bad type"};
                        }
                        p += vb->size;
                    }
                }
            }
        }*/
    };

    auto open() {
        return files{*this, fn};
    }

private:
    std::string utf8_to_dbstr(const std::string &s) const {
        return utf8_to_dbstr((const char8_t *)s.c_str());
    }
    std::string utf8_to_dbstr(const char *s) const {
        return utf8_to_dbstr((const char8_t *)s);
    }
    std::string utf8_to_dbstr(const char8_t *s) const {
        return str2str((const char *)s, CP_UTF8, codepage);
    }
};
