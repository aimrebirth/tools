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

#include <fstream>

#include <buffer.h>

void create_sql(std::string path, const db &db)
{
    std::ofstream ofile(path + ".sql");
    if (!ofile)
        return;

    std::string master_table_name = "DB_TABLE_LIST";
    const std::string id = "ID";
    const std::string row_type = "TEXT_ID";

    // db master table
    ofile << "drop table if exists " << master_table_name << ";\n";
    ofile << "create table \"" << master_table_name << "\"\n";
    ofile << "(\n";
    ofile << "  \"" + str2utf8(id) + "\" INTEGER,\n";
    ofile << "  \"" + str2utf8(row_type) + "\" TEXT\n";
    ofile << ");\n";
    ofile << "\n";

    // db tables
    for (auto &table : db.t.tables)
    {
        auto &t = table.second;
        std::string name = str2utf8(t.name);
        ofile << "drop table if exists " << name << ";\n";
        ofile << "create table \"" << name << "\"\n";
        ofile << "(\n";
        std::string s;
        s += "  \"" + str2utf8(id) + "\" INTEGER,\n";
        s += "  \"" + str2utf8(row_type) + "\" TEXT,\n";
        for (auto &f : db.t.fields)
        {
            if (f.second.table_id != t.id)
                continue;
            s += "  \"" + str2utf8(f.second.name) + "\" " + getSqlType(f.second.type) + ",\n";
        }
        s.resize(s.size() - 2);
        ofile << s << "\n";
        ofile << ");\n";
        ofile << "\n";
    }

    // db master table
    for (auto &table : db.t.tables)
    {
        static int idx = 1;
        ofile << "insert into \"" << master_table_name << "\" values (";
        ofile << "'" << idx++ << "', ";
        ofile << "'" << str2utf8(table.second.name) << "'";
        ofile << ");\n";
    }

    // db tables
    std::map<int,int> idx;
    for (auto &v : db.values)
    {
        auto tbl = db.t.tables.find(v.table_id);
        if (tbl == db.t.tables.end())
            continue;
        ofile << "insert into \"" << str2utf8(tbl->second.name) << "\" (";
        std::string s;
        s += "'" + str2utf8(id) + "', ";
        s += "'" + str2utf8(row_type) + "', ";
        for (auto &f : v.fields)
        {
            auto fld = db.t.fields.find(f.field_id);
            s += "'" + str2utf8(fld->second.name) + "', ";
        }
        s.resize(s.size() - 2);
        ofile << s << ") values (";
        s.clear();
        s += "'" + std::to_string(idx[v.table_id]++) + "', ";
        s += "'" + str2utf8(v.name) + "', ";
        for (auto &f : v.fields)
        {
            s += "'";
            auto fld = db.t.fields.find(f.field_id);
            switch (fld->second.type)
            {
                case FieldType::String:
                    s += str2utf8(f.s.c_str());
                    break;
                case FieldType::Integer:
                    s += std::to_string(f.i);
                    break;
                case FieldType::Float:
                    s += std::to_string(f.f);
                    break;
                default:
                    assert(false);
            }
            s += "', ";
        }
        s.resize(s.size() - 2);
        ofile << s << ");\n";
    }
}

int main(int argc, char *argv[])
try
{
    if (argc != 2)
    {
        std::cout << "Usage:\n" << argv[0] << " path/to/aim_game/data/db" << "\n" << argv[0] << " path/to/aim_game/data/quest" << "\n";
        return 1;
    }
    path p = argv[1];
    db db;
    db.open(p);
    create_sql(p.string(), db);
    return 0;
}
catch (std::exception &e)
{
    printf("error: %s\n", e.what());
    return 1;
}
catch (...)
{
    printf("error: unknown exception\n");
    return 1;
}
