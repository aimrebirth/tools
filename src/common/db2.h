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
        };
        template <typename T>
        struct setter {
            T field;
            value value;
            void operator=(auto &&v) {
                value.set_field(field, v);
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
        auto operator()(auto &&tname, auto &&vname, auto &&fname) {
            auto tbl = find_table(tname);
            auto value = tbl.find_value(vname);
            if constexpr (std::is_convertible_v<decltype(fname), const char *>) {
                return setter<const char *>{fname, value};
            } else {
                return setter<std::string>{fname, value};
            }
        }
    };

    auto open() {
        return files{*this,fn};
    }
    void add_value(auto &&table, auto &&value, auto && ... fields1) {
        auto f = open();
        auto tbl = f.tab_.data->tables();
        auto fields = f.tab_.data->fields();
        auto values = f.ind_.data->values();

        auto table_db_cp = utf8_to_dbstr(table);
        auto value_db_cp = utf8_to_dbstr(value);

        auto calc_fields_size = [&](this auto &&f, auto &&field_name, auto &&n, auto &&v, auto &&...fields) {
            if (field_name == n) {
                if constexpr (std::same_as<std::decay_t<decltype(v)>, int>) {
                    return sizeof(db2::dat::field_value_base) + sizeof(int);
                } else if constexpr (std::same_as<std::decay_t<decltype(v)>, float>) {
                    return sizeof(db2::dat::field_value_base) + sizeof(float);
                } else {
                    auto s = utf8_to_dbstr(v);
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
        auto write_fields = [&](this auto &&f, auto &&p, auto &&field, auto &&field_name, auto &&n, auto &&v, auto &&...fields) {
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
                    auto s = utf8_to_dbstr(v);
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

        auto it = std::ranges::find_if(tbl, [&](auto &v){return v.name == table_db_cp;});
        if (it == tbl.end()) {
            throw std::runtime_error{"no such table: "s + table_db_cp};
        }
        auto &t = *it;
        auto itv = std::ranges::find_if(values, [&](auto &v){return v.table_id == t.id && value_db_cp == v.name;});
        if (itv == values.end()) {
            db2::ind::value i{};
            i.table_id = t.id;
            memcpy(i.name, value_db_cp.data(), value_db_cp.size());
            i.offset = f.dat_.f.sz;
            for (auto &&f : fields) {
                if (f.table_id != t.id) {
                    continue;
                }
                std::string_view fn = f.name;
                i.size += calc_fields_size(fn, fields1...);
            }

            ++f.ind_.data->n_values;
            auto p = f.ind_.f.alloc_raw(f.ind_.f.sz + sizeof(i));
            memcpy(p, &i, sizeof(i));

            p = f.dat_.f.alloc_raw(f.dat_.f.sz + i.size);
            for (auto &&f : fields) {
                if (f.table_id != t.id) {
                    continue;
                }
                std::string_view fn = f.name;
                write_fields(p, f, fn, fields1...);
            }
        }
    }
    template <typename T>
    void edit_value(auto &&table, auto &&value, auto &&field, const T &field_value) {
        auto f = open();
        auto fields = f.tab_.data->fields();
        auto values = f.ind_.data->values();

        auto value_db_cp = utf8_to_dbstr(value);
        auto field_db_cp = utf8_to_dbstr(field);

        /*auto t = f.find_table(table);
        auto itv = std::ranges::find_if(values, [&](auto &v) {
            return v.table_id == t.id && value_db_cp == v.name;
        });
        if (itv == values.end()) {
            throw std::runtime_error{"no such value: "s + value_db_cp};
        }
        auto itf = std::ranges::find_if(fields, [&](auto &v) {
            return v.table_id == t.id && field_db_cp == v.name;
        });
        if (itf == fields.end()) {
            throw std::runtime_error{"no such field: "s + field_db_cp};
        }

        auto p = f.dat_.f.p + itv->offset;
        while (p < f.dat_.f.p + itv->offset + itv->size) {
            auto &header = *(dat::field_value_base*)p;
            p += sizeof(header);
            if (header.field_id == itf->id) {
                if constexpr (std::same_as<std::decay_t<decltype(field_value)>, int>) {
                    *(int*)p = field_value;
                } else if constexpr (std::same_as<std::decay_t<decltype(field_value)>, float>) {
                    *(float *)p = field_value;
                } else {
                    auto s = utf8_to_dbstr(field_value);
                    if (s.size() + 1 != header.size) {
                        throw std::runtime_error{"not implemented yet"}; // maybe just assign new value into the end of db
                    }
                    memcpy(p, s.data(), s.size());
                }
                return;
            }
            p += header.size;
        }*/
        throw std::runtime_error{"no such field"};
    }

private:
    std::string utf8_to_dbstr(const char *s) const {
        return utf8_to_dbstr((const char8_t *)s);
    }
    std::string utf8_to_dbstr(const char8_t *s) const {
        return str2str((const char *)s, CP_UTF8, codepage);
    }
};
