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

#include <nlohmann/json.hpp>
#include <primitives/templates2/overload.h>
#include <primitives/templates2/type_name.h>

#include <variant>

std::string utf8_to_dbstr(const char8_t *s, int codepage = 1251) {
    return str2str((const char *)s, CP_UTF8, codepage);
}
std::string utf8_to_dbstr(const char *s, int codepage = 1251) {
    return utf8_to_dbstr((const char8_t *)s, codepage);
}
std::string utf8_to_dbstr(const std::string &s, int codepage = 1251) {
    return utf8_to_dbstr((const char8_t *)s.c_str(), codepage);
}

struct mem_stream {
    std::vector<uint8_t> d;

    template <typename T>
    operator T &() {
        d.resize(d.size() + sizeof(T));
        auto p = d.data() + d.size() - sizeof(T);
        auto &r = *(T *)p;
        return r;
    }
    template <typename T>
    auto operator=(const T &in) {
        T &v = *this;
        v = in;
        return sizeof(T);
    }
    auto operator=(const std::string &in) {
        *this += in;
        return in.size() + 1;
    }
    void operator+=(const mem_stream &s) {
        d.append_range(s.d);
    }
    void operator+=(const std::string &s) {
        d.append_range(s);
        d.push_back(0);
    }
    template <typename T>
    T &at(int i) {
        if (i < 0) {
            return *(T *)(d.data() + d.size() + i);
        } else {
            return *(T *)(d.data() + i);
        }
    }
    auto size() const { return d.size(); }
};

decltype(auto) visit(auto &&var, auto &&...f) {
    return ::std::visit(overload{FWD(f)...}, var);
}

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
            f.open(fn, primitives::templates2::mmap_file<uint8_t>::ro{});
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

            auto begin(this auto &&d) {return d.m.begin();}
            auto end(this auto &&d) {return d.m.end();}
            bool empty() const { return m.empty(); }
            auto &operator[](this auto &&d, const std::string &s) {
                return d.m[s];
            }
            auto &operator[](this auto &&d, const std::u8string &s) {
                return d.m[(const char *)s.c_str()];
            }
            auto to_json() const {
                nlohmann::json ja;
                for (auto &&[tn,t] : m) {
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
                return ja;
            }
            void load_from_json(const path &fn) {
                auto ja = nlohmann::json::parse(read_file(fn));
                for (auto &&[tn,t] : ja.items()) {
                    for (auto &&[vn,v] : t.items()) {
                        for (auto &&[fn, fv] : v.items()) {
                            if (0) {
                            } else if (fv.is_number_integer()) {
                                m[tn][vn][fn] = fv.get<int>();
                            } else if (fv.is_number_float()) {
                                m[tn][vn][fn] = fv.get<float>();
                            } else if (fv.is_string()) {
                                m[tn][vn][fn] = fv.get<std::string>();
                            } else {
                                throw std::runtime_error{"bad json type"};
                            }
                        }
                    }
                }
            }
            void save(const path &fn, int codepage = 1251) {
                auto s_to_char20 = [&](char20 &dst, const std::string &in, int codepage = 1251) {
                    auto s = utf8_to_dbstr(in);
                    if (s.size() + 1 > sizeof(char20)) {
                        throw std::runtime_error{"too long string"};
                    }
                    memcpy(dst, s.c_str(), s.size());
                };

                mem_stream tabv,tabv_fields;
                tab &_ = tabv;
                int table_id{1};
                int total_fields{};
                int total_values{};
                std::map<std::string, std::map<std::string, field_type>> fields;
                for (auto &&[tn,td] : m) {
                    tab::table &t = tabv;
                    t.id = table_id;
                    s_to_char20(t.name, tn, 1251); // always 1251

                    for (auto &&[_,fd] : td) {
                        for (auto &&[fn,fv] : fd) {
                            fields[tn][fn] = (field_type)fv.index();
                        }
                        ++total_values;
                    }
                    for (auto &&[fn,ft] : fields[tn]) {
                        tab::field &f = tabv_fields;
                        f.id = ++total_fields;
                        f.table_id = table_id;
                        f.type = ft;
                        ft = (field_type)total_fields;
                        s_to_char20(f.name, fn, 1251); // always 1251 if we have any field in Russian
                    }

                    ++table_id;
                }
                tabv += tabv_fields;
                {
                    auto &tab_ = tabv.at<tab>(0);
                    tab_.n_tables = m.size();
                    tab_.n_fields = total_fields;
                }

                mem_stream indv, datv;
                ind &i = indv;
                i.n_values = total_values;
                table_id = 1;
                for (auto &&[tn, td] : m) {
                    for (auto &&[vn, fd] : td) {
                        ind::value &i = indv;
                        i.table_id = table_id;
                        s_to_char20(i.name, vn, codepage);
                        i.offset = datv.size();
                        for (auto &&[fn, fv] : fd) {
                            dat::field_value_base &_ = datv;
                            auto sz = visit(fv,
                                [&](const int &v) { return datv = v; },
                                [&](const float &v) { return datv = v; },
                                [&](const std::string &v) { return datv = utf8_to_dbstr(v); });
                            auto &v = datv.at<dat::field_value_base>(-(sizeof(dat::field_value_base) + sz));
                            v.field_id = (int)fields[tn].find(fn)->second;
                            v.size = sz;
                        }
                        i.size = datv.size() - i.offset;
                    }
                    ++table_id;
                }

                write_file(path{fn} += ".tab", tabv.d);
                write_file(path{fn} += ".ind", indv.d);
                write_file(path{fn} += ".dat", datv.d);
            }
        };

        // converts string to utf8
        // filters out values with empty name ""
        auto to_map() const {
            auto prepare_string = [](auto &&in) {
                auto s = str2utf8(in);
                // we have some erroneous table values (records) with spaces
                // we can trim only field values, but don't do it at the moment
                //boost::trim(s);
                return s;
            };

            db2_internal m;
            auto tbl = tab_.data->tables();
            for (auto &&t : tbl) {
                auto &jt = m[prepare_string(t.name)];
                auto fields = tab_.data->fields(t.id);
                for (auto &&v : ind_.data->values(t.id)) {
                    auto vn = prepare_string(v.name);
                    if (vn.empty()) {
                        continue;
                    }
                    std::decay_t<decltype(jt)>::mapped_type jv;
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
                        boost::trim(fn); // trim field name
                        if (jv.contains(fn)) {
                            // we analyze such cases manually
                            throw std::logic_error{"duplicate field: "s + fn};
                        }
                        switch (f->type) {
                        case db2::field_type::integer:
                            jv[fn] = *(int *)p;
                            break;
                        case db2::field_type::float_:
                            jv[fn] = *(float *)p;
                            break;
                        case db2::field_type::string:
                            jv[fn] = boost::trim_copy(prepare_string((const char *)p)); // trim field value
                            break;
                        default:
                            throw std::logic_error{"bad type"};
                        }
                        p += vb->size;
                    }
                    if (jt.contains(vn)) {
                        if (!jv.contains("DELETED") || std::get<int>(jv["DELETED"]) == 0) {
                            // we analyze such cases manually
                            throw std::logic_error{"duplicate value: "s + vn};
                        }
                    }
                    jv.erase("DELETED"); // we erase this unused? field (maybe only from quest db?
                    jt[vn] = jv;
                }
            }
            return m;
        }
    };

    auto open() {
        return files{*this, fn};
    }
};
