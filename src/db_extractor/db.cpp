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

#define FREAD(var) fread(&var, 1, sizeof(var), f)
#define SREAD(var) s.read(&var, sizeof(var))
#define SREAD_N(var, sz) s.read(&var, sz)

string getSqlType(uint32_t ft)
{
    switch (ft)
    {
    case T_STRING:
        return "TEXT";
    case T_INTEGER:
        return "INTEGER";
    case T_FLOAT:
        return "REAL";
    default:
        assert(false);
    }
    return "";
}

void table::load(FILE *f)
{
    FREAD(id);
    FREAD(name);
    FREAD(unk1);
    FREAD(unk2);
    FREAD(unk3);
    FREAD(unk4);
}

void field::load(FILE *f)
{
    FREAD(table_id);
    FREAD(id);
    FREAD(name);
    FREAD(unk1);
    FREAD(unk2);
    FREAD(unk3);
    FREAD(type);
}

void tab::load(FILE *f)
{
    FREAD(number_of_tables);
    FREAD(number_of_fields);

    auto n = number_of_tables;
    while (n--)
    {
        table t;
        t.load(f);
        tables[t.id] = t;
    }

    n = number_of_fields;
    while (n--)
    {
        field t;
        t.load(f);
        fields[t.id] = t;
    }
}

void value::load_index(FILE *f)
{
    FREAD(table_id);
    FREAD(name);
    FREAD(unk1);
    FREAD(unk2);
    FREAD(unk3);
    FREAD(offset);
    FREAD(data_size);
    buf.resize(data_size);
}

void value::load_data(FILE *f)
{
    fseek(f, offset, SEEK_SET);
    fread(buf.data(), buf.size(), 1, f);
}

void value::extract_fields(const tab &tab)
{
    s_file s(buf);

    while (1)
    {
        field_value fv;
        if (SREAD(fv.field_id) == 0)
            break;
        SREAD(fv.size);
        auto i = tab.fields.find(fv.field_id);
        if (i == tab.fields.end())
            continue;
        switch (i->second.type)
        {
        case T_STRING:
            fv.s.resize(fv.size);
            SREAD_N(fv.s[0], fv.s.size());
            break;
        case T_INTEGER:
            SREAD(fv.i);
            if (fv.size > sizeof(fv.i))
                s.skip(fv.size - sizeof(fv.i));
            break;
        case T_FLOAT:
            SREAD(fv.f);
            if (fv.size > sizeof(fv.i))
                s.skip(fv.size - sizeof(fv.i));
            break;
        default:
            assert(false);
        }
        fields.push_back(fv);
    }
}

void db::load(FILE *f)
{
    FREAD(number_of_values);

    auto n = number_of_values;
    while (n--)
    {
        value t;
        t.load_index(f);
        values.push_back(t);
    }
}