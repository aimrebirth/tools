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

#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>

#include <fstream>

void create_sql(path p, const polygon4::tools::db::processed_db &db)
{
    std::ofstream ofile(p += ".sql");
    if (!ofile)
        return;

    std::string master_table_name = "DB_TABLE_LIST";
    const std::string id = "ID";
    const std::string row_type = "TEXT_ID";

    // db master table
    ofile << "drop table if exists " << master_table_name << ";\n";
    ofile << "create table \"" << master_table_name << "\"\n";
    ofile << "(\n";
    ofile << "  \"" + id + "\" INTEGER,\n";
    ofile << "  \"" + row_type + "\" TEXT\n";
    ofile << ");\n";
    ofile << "\n";

    for (auto &[name,table] : db)
    {
        static int idx = 1;
        ofile << "insert into \"" << master_table_name << "\" values (";
        ofile << "'" << idx++ << "', ";
        ofile << "'" << name << "'";
        ofile << ");\n";
    }
    ofile << "\n";

    // db tables
    for (auto &[name, t] : db)
    {
        ofile << "drop table if exists " << name << ";\n";
        ofile << "create table \"" << name << "\"\n";
        ofile << "(\n";
        std::string s;
        s += "  \"" + id + "\" INTEGER,\n";
        s += "  \"" + row_type + "\" TEXT,\n";
        std::map<std::string, FieldType> fields;
        for (auto &[_, f] : t)
        {
            for (auto &[n, v] : f)
                fields[n] = (FieldType)v.index();
        }
        for (auto &[n, t] : fields)
            s += "  \"" + n + "\" " + getSqlType(t) + ",\n";
        s.resize(s.size() - 2);
        ofile << s << "\n";
        ofile << ");\n";
        ofile << "\n";
    }

    // db tables
    std::map<std::string,int> idx;
    for (auto &[tn, t] : db)
    {
        for (auto &[rn, row] : t)
        {
            ofile << "insert into \"" << tn << "\" (";
            std::string s;
            s += "'" + id + "', ";
            s += "'" + row_type + "', ";
            for (auto &[n, v] : row)
                s += "'" + n + "', ";
            s.resize(s.size() - 2);
            ofile << s << ") values (";
            s.clear();
            s += "'" + std::to_string(++idx[tn]) + "', ";
            s += "'" + rn + "', ";
            for (auto &[n, v] : row)
            {
                s += "'";
                switch ((FieldType)v.index())
                {
                case FieldType::String:
                    s += std::get<std::string>(v);
                    break;
                case FieldType::Integer:
                    s += std::to_string(std::get<int>(v));
                    break;
                case FieldType::Float:
                    s += std::to_string(std::get<float>(v));
                    break;
                default:
                    SW_UNIMPLEMENTED;
                }
                s += "', ";
            }
            s.resize(s.size() - 2);
            ofile << s << ");\n";
        }
    }
}

int main(int argc, char *argv[])
{
    cl::opt<path> db_fn(cl::Positional, cl::desc("<db file>"), cl::Required);
    cl::opt<int> codepage(cl::Positional, cl::desc("<codepage>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    db db;
    db.open(db_fn);
    create_sql(db_fn, db.process(codepage));
    return 0;
}
