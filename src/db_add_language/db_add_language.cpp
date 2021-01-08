/*
 * AIM db_extractor
 * Copyright (C) 2017 lzwdgc
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

#include <common.h>
#include <db.h>

#include <Polygon4/DataManager/Database.h>
#include <Polygon4/DataManager/Localization.h>
#include <Polygon4/DataManager/Storage.h>
#include <Polygon4/DataManager/Types.h>
#include <primitives/filesystem.h>
#include <primitives/executor.h>
#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <numeric>

#include <math.h>

// MultiByteToWideChar: https://msdn.microsoft.com/en-us/library/windows/desktop/dd319072(v=vs.85).aspx
// code pages: https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756(v=vs.85).aspx
const std::map<std::string, int> code_pages
{
    { "en", 0 },
    { "ru", 1251 },
    { "et", 1257 },
};

static int get_cp(const std::string &cp)
{
    auto i = code_pages.find(cp);
    if (i == code_pages.end())
        throw std::runtime_error("No code page for lang: " + cp);
    return i->second;
}

struct string_index
{
    P4String s;
    polygon4::detail::IdType i = -1;

    void setString(const std::string &rhs, int cp)
    {
        s = to_string(str2utf16(rhs, cp));
    }
};

using AimKV = std::map<std::string, string_index>;
using AimKVResolved = std::unordered_map<std::string, polygon4::detail::IdType>;
AimKVResolved kv_resolved;

template <class T>
static int levenshtein_distance(const T &s1, const T &s2)
{
    // To change the type this function manipulates and returns, change
    // the return type and the types of the two variables below.
    auto s1len = s1.size();
    auto s2len = s2.size();

    decltype(s1len) column_start = 1;

    auto column = new decltype(s1len)[s1len + 1];
    std::iota(column + column_start, column + s1len + 1, column_start);

    for (auto x = column_start; x <= s2len; x++) {
        column[0] = x;
        auto last_diagonal = x - column_start;
        for (auto y = column_start; y <= s1len; y++) {
            auto old_diagonal = column[y];
            auto possibilities = {
                column[y] + 1,
                column[y - 1] + 1,
                last_diagonal + (s1[y - 1] == s2[x - 1] ? 0 : 1)
            };
            column[y] = std::min(possibilities);
            last_diagonal = old_diagonal;
        }
    }
    auto result = column[s1len];
    delete[] column;
    return result;
}

static auto open(const path &p)
{
    db db;
    if (fs::exists(p / "quest.dat"))
        db.open(p / "quest");
    return db;
};

static AimKV get_kv(const db &db, int cp)
{
    auto iter_tbl = std::find_if(db.t.tables.begin(), db.t.tables.end(), [](auto &t) {
        return t.second.name == "INFORMATION";
    });
    if (iter_tbl == db.t.tables.end())
        throw std::runtime_error("Table INFORMATION was not found");

    auto find_field = [&db, &iter_tbl](const std::string &name)
    {
        auto i = std::find_if(db.t.fields.begin(), db.t.fields.end(), [&iter_tbl, &name](auto &t) {
            return t.second.table_id == iter_tbl->second.id && t.second.name == name;
        });
        if (i == db.t.fields.end())
            throw std::runtime_error("Field " + name + " was not found");
        return i->first;
    };
    auto nid = find_field("NAME");
    auto tid = find_field("TEXT");

    AimKV kv;
    for (auto &v : db.values)
    {
        if (v.table_id != iter_tbl->second.id || v.name.empty())
            continue;
        for (auto &f : v.fields)
        {
            if ((f.field_id == nid || f.field_id == tid) && !f.s.empty())
                kv[v.name].setString(f.s, cp);
        }
    }
    return kv;
}

static AimKVResolved get_kv_resolved(const path &d, const polygon4::Storage &storage)
{
    static const auto fn = "kv.resolved";

    AimKVResolved mres;
    if (fs::exists(fn))
    {
        std::ifstream f(fn);
        std::string s;
        polygon4::detail::IdType i;
        while (f)
        {
            f >> std::quoted(s);
            if (!f)
                break;
            f >> i;
            mres[s] = i;
        }
    }
    else
    {
        auto db1 = open(d / "ru" / "aim1");
        auto db2 = open(d / "ru" / "aim2");

        auto kv1 = get_kv(db1, get_cp("ru"));
        auto kv2 = get_kv(db2, get_cp("ru"));
        kv1.insert(kv2.begin(), kv2.end());
        auto sz = kv1.size();
        std::cout << "total kvs: " << sz << "\n";

        Executor e;
        int i = 0;
        for (auto &kv : kv1)
        {
            e.push([&storage, &i, &sz, &kv]()
            {
                std::cout << "total kvs: " << ++i << "/" << sz << "\n";
                std::map<int, polygon4::detail::IdType> m;
                for (auto &s : storage.strings)
                    m[levenshtein_distance(kv.second.s, s.second->string.ru)] = s.first;
                if (m.empty())
                    return;
                kv.second.i = m.begin()->second;
            });
        }
        e.wait();

        std::ofstream f(fn);
        for (auto &kv : kv1)
        {
            mres[kv.first] = kv.second.i;
            f << std::quoted(kv.first) << " " << kv.second.i << "\n";
        }
    }

    // make unique ids
    std::unordered_map<AimKVResolved::mapped_type, AimKVResolved::key_type> u;
    for (auto &kv : mres)
        u[kv.second] = kv.first;
    mres.clear();
    for (auto &kv : u)
        mres[kv.second] = kv.first;

    return mres;
}

static void process_lang(polygon4::Storage &s, const path &p, polygon4::String polygon4::LocalizedString::*field)
{
    auto db1 = open(p);
    auto db2 = open(p / "aim1");
    auto db3 = open(p / "aim2");

    AimKV kvm;
    auto get_kv = [&kvm, &p](auto &db)
    {
        AimKV kv1;
        if (db.number_of_values)
        {
            kv1 = ::get_kv(db, get_cp(to_printable_string(p.filename())));
            kvm.insert(kv1.begin(), kv1.end());
        }
    };
    get_kv(db1);
    get_kv(db2);
    get_kv(db3);

    std::multimap<int, std::string> dist;
    std::multimap<double, std::string> dist2;
    for (auto &kv : kvm)
    {
        auto i = kv_resolved.find(kv.first);
        if (i == kv_resolved.end())
            continue;
        auto &sold = s.strings[i->second]->string.*field;
        auto d = levenshtein_distance(sold, kv.second.s);
        dist.insert({ d, kv.first });
        //if (d == 0)
        //    continue;
        auto len_diff = abs((int)sold.size() - (int)kv.second.s.size());
        auto min_len = (sold.size() + kv.second.s.size()) / 2.0;
        //d -= len_diff;
        //if (d == 0)
        //    continue;
        dist2.insert({ d / double(min_len), kv.first });
    }

    std::string str;
    for (auto &d2 : dist2)
    {
        auto &kv = *kvm.find(d2.second);
        auto i = kv_resolved.find(kv.first);
        if (i == kv_resolved.end())
            continue;
        auto &sold = s.strings[i->second]->string.*field;
        //sold = kv.second.s;
        str += "id: " + std::to_string(i->second) + "\n";
        str += "kd: " + std::to_string(d2.first) + "\n";
        str += "key: " + i->first + "\n\n";
        str += "old:\n";
        str += sold + "\n";
        str += "\n";
        str += "new:\n";
        str += kv.second.s + "\n";
        str += "\n================================================\n\n";
    }
    /*for (auto &kv : kvm)
    {
        auto i = kv_resolved.find(kv.first);
        if (i == kv_resolved.end())
            continue;
        auto &sold = s.strings[i->second]->string.*field;
        //sold = kv.second.s;
        str += "id: " + std::to_string(i->second) + "\n";
        str += "key: " + i->first + "\n\n";
        str += "old:\n";
        str += wstring2string(sold) + "\n";
        str += "\n";
        str += "new:\n";
        str += wstring2string(kv.second.s) + "\n";
        str += "\n================================================\n\n";
    }*/
    write_file(p / (p.filename() += "_diff.txt"), str);
}

int main(int argc, char *argv[])
{
    cl::opt<path> db_fn(cl::Positional, cl::desc("<db file>"), cl::Required);
    cl::opt<path> dir_to_lang_dbs(cl::Positional, cl::desc("<dir to lang dbs>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    fs::current_path(dir_to_lang_dbs);

    auto storage = polygon4::initStorage();
    auto database = std::make_unique<polygon4::Database>(db_fn);
    storage->load(*database, {});
    kv_resolved = get_kv_resolved(dir_to_lang_dbs, *storage.get());

    // to check correctness
    process_lang(*storage.get(), dir_to_lang_dbs / "ru", &polygon4::LocalizedString::ru);

    for (auto &f : fs::directory_iterator(dir_to_lang_dbs))
    {
        if (!fs::is_directory(f))
            continue;

        auto p = f.path();

        if (0);
#define ADD_LANGUAGE(l, n) else if (p.filename() == #l && p.filename() != "ru") \
    {process_lang(*storage.get(), p, &polygon4::LocalizedString::l);}
#include <Polygon4/DataManager/Languages.inl>
#undef ADD_LANGUAGE
        else
        {
            std::cerr << "No such lang: " << to_printable_string(p.filename()) << "\n";
            continue;
        }
    }

    return 0;
}
