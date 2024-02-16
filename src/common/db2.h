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
        template <typename T, typename V>
        struct setter {
            T field;
            V value;
            void operator=(auto &&v) {
                value.set_field(field, v);
            }
        };
        struct table {
            files &f;
            db2::tab::table t;

            auto find_field(auto &&name, auto ftype) {
                auto fields = f.tab_.data->fields();
                auto field_db_cp = f.db.utf8_to_dbstr(name);
                auto itf = std::ranges::find_if(fields, [&](auto &v) {
                    return v.table_id == t.id && field_db_cp == v.name;
                });
                if (itf == fields.end()) {
                    if (field_db_cp.size() + 1 > sizeof(tab::field::name)) {
                        throw std::runtime_error{"too long field name: "s + field_db_cp};
                    }
                    auto p = f.tab_.f.alloc_raw(f.tab_.f.sz + sizeof(tab::field));
                    auto &newfield = *(tab::field *)p;
                    memset(&newfield, 0, sizeof(newfield));
                    newfield.table_id = t.id;
                    newfield.id = ++f.tab_.data->n_fields;
                    strcpy(newfield.name, field_db_cp.data());
                    newfield.type = (decltype(newfield.type))ftype;
                    return newfield;
                }
                if (itf->type != ftype) {
                    throw std::runtime_error{"field type mismatch: "s + field_db_cp};
                }
                return *itf;
            }
            auto find_value(auto &&name) {
                auto values = f.ind_.data->values();
                auto value_db_cp = f.db.utf8_to_dbstr(name);
                auto itv = std::ranges::find_if(values, [&](auto &v) {
                    return v.table_id == t.id && value_db_cp == v.name;
                });
                if (itv == values.end()) {
                    db2::ind::value i{};
                    i.table_id = t.id;
                    if (value_db_cp.size() + 1 > sizeof(i.name)) {
                        throw std::runtime_error{"too long value name: "s + value_db_cp};
                    }
                    memcpy(i.name, value_db_cp.data(), value_db_cp.size());
                    i.offset = f.dat_.f.sz;
                    ++f.ind_.data->n_values;
                    auto p = f.ind_.f.alloc_raw(f.ind_.f.sz + sizeof(i));
                    memcpy(p, &i, sizeof(i));
                    return value{*this,*(db2::ind::value *)p};
                }
                return value{*this,*itv};
            }
            auto operator()(auto &&name) {
                return find_value(name);
            }
            auto operator()(auto &&vname, auto &&fname) {
                auto value = find_value(vname);
                if constexpr (std::is_convertible_v<decltype(fname), const char *>) {
                    return setter<const char *, decltype(value)>{fname, value};
                } else {
                    return setter<std::string, decltype(value)>{fname, value};
                }
            }
        };
        struct value {
            table t;
            db2::ind::value &v;

            static auto field_type(auto &&v) {
                if constexpr (std::same_as<std::decay_t<decltype(v)>, int>) {
                    return field_type::integer;
                } else if constexpr (std::same_as<std::decay_t<decltype(v)>, float>) {
                    return field_type::float_;
                } else {
                    return field_type::string;
                }
            }
            void set_field(auto &&name, auto &&v) {
                using T = std::decay_t<decltype(v)>;
                auto f = t.find_field(name, field_type(v));
                if (f.type != field_type(v)) {
                    throw std::runtime_error{"field type mismatch: "s + t.f.db.utf8_to_dbstr(name)};
                }
                dat::field_value_base newfield{f.id};
                if constexpr (std::same_as<std::decay_t<decltype(v)>, int> ||
                              std::same_as<std::decay_t<decltype(v)>, float>) {
                    newfield.size = sizeof(T);
                } else {
                    newfield.size = t.f.db.utf8_to_dbstr(v).size() + 1;
                }
                uint32_t newfieldsize = sizeof(newfield) + newfield.size;
                std::vector<uint8_t> data(this->v.size + newfieldsize);
                auto dp = data.data();
                auto base = t.f.dat_.f.p + this->v.offset;
                auto p = base;
                while (p < base + this->v.size) {
                    auto &header = *(dat::field_value_base *)p;
                    auto len = sizeof(header) + header.size;
                    if (header.field_id != f.id) {
                        memcpy(dp, p, len);
                        dp += len;
                    } else {
                        if constexpr (std::same_as<std::decay_t<decltype(v)>, int> ||
                                      std::same_as<std::decay_t<decltype(v)>, float>) {
                            if (header.size == newfield.size && memcmp(p + sizeof(header), &v, sizeof(v)) == 0) {
                                return;
                            }
                        } else {
                            if (header.size == newfield.size &&
                                strcmp((const char *)p + sizeof(header), t.f.db.utf8_to_dbstr(v).data()) == 0) {
                                return;
                            }
                        }
                    }
                    p += len;
                }
                *(dat::field_value_base *)dp = newfield;
                dp += sizeof(newfield);
                if constexpr (std::same_as<std::decay_t<decltype(v)>, int> ||
                              std::same_as<std::decay_t<decltype(v)>, float>) {
                    *(T *)dp = v;
                    dp += sizeof(v);
                } else {
                    auto s = t.f.db.utf8_to_dbstr(v);
                    memcpy(dp, s.data(), s.size());
                    dp += s.size() + 1;
                }
                auto reallen = dp - data.data();
                this->v.size = reallen;
                this->v.offset = t.f.dat_.f.sz;
                memcpy(t.f.dat_.f.alloc_raw(t.f.dat_.f.sz + reallen), data.data(), reallen);
            }
            auto operator()(auto &&name) {
                if constexpr (std::is_convertible_v<decltype(name), const char *>) {
                    return setter<const char *, value>{name, *this};
                } else {
                    return setter<std::string, value>{name, *this};
                }
            }
        };

        db2 &db;
        file<tab> tab_;
        file<ind> ind_;
        file<dat> dat_;

        files(auto &&db, auto &&base) : db{db}, tab_{base}, ind_{base}, dat_{base} {}

        auto get_files() const {
            return std::set<path>{tab_.fn,ind_.fn,dat_.fn};
        }
        auto find_table(auto &&name) {
            auto tbl = tab_.data->tables();
            auto table_db_cp = db.utf8_to_dbstr(name);
            auto it = std::ranges::find_if(tbl, [&](auto &v) {
                return v.name == table_db_cp;
            });
            if (it == tbl.end()) {
                if (table_db_cp.size() + 1 > sizeof(tab::table::name)) {
                    throw std::runtime_error{"too long table name: "s + table_db_cp};
                }
                tab_.f.alloc_raw(tab_.f.sz + sizeof(tab::table));
                auto base = tab_.f.p + sizeof(tab) + tab_.data->n_tables * sizeof(tab::table);
                memmove(base + sizeof(tab::table), base, tab_.f.sz - (base - tab_.f.p + sizeof(tab::table)));
                auto &newtab = *(tab::table *)base;
                memset(&newtab, 0, sizeof(newtab));
                newtab.id = ++tab_.data->n_tables;
                strcpy(newtab.name, table_db_cp.data());
                return table{*this, newtab};
            }
            return table{*this,*it};
        }

        // [] not in msvc yet
        auto operator()(auto &&tname) {
            return find_table(tname);
        }
        auto operator()(auto &&tname, auto &&vname, auto &&fname) {
            auto tbl = find_table(tname);
            auto value = tbl.find_value(vname);
            if constexpr (std::is_convertible_v<decltype(fname), const char *>) {
                return setter<const char *, decltype(value)>{fname, value};
            } else {
                return setter<std::string, decltype(value)>{fname, value};
            }
        }
    };

    auto open() {
        return files{*this,fn};
    }

private:
    std::string utf8_to_dbstr(const char *s) const {
        return utf8_to_dbstr((const char8_t *)s);
    }
    std::string utf8_to_dbstr(const char8_t *s) const {
        return str2str((const char *)s, CP_UTF8, codepage);
    }
};
