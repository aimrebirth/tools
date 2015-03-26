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

#include <assert.h>
#include <iostream>
#include <map>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <vector>

using namespace std;

enum FieldType
{
    T_STRING,
    T_INTEGER,
    T_FLOAT,
};
string getSqlType(uint32_t ft);

struct table
{
    uint32_t id;
    char name[0x14];
    uint32_t unk1;
    uint32_t unk2;
    uint32_t unk3;
    uint32_t unk4;

    void load(FILE *f);
};

struct field
{
    uint32_t table_id;
    uint32_t id;
    char name[0x14];
    uint32_t unk1;
    uint32_t unk2;
    uint32_t unk3;
    uint32_t type;

    void load(FILE *f);
};

struct tab
{
    uint32_t number_of_tables;
    uint32_t number_of_fields;

    map<uint32_t, table> tables;
    map<uint32_t, field> fields;

    void load(FILE *f);
};

struct field_value
{
    uint32_t field_id;
    uint32_t size;

    int i = 0;
    float f = 0.f;
    string s;
};

struct value
{
    struct s_file
    {
        uint32_t index = 0;
        const vector<char> buf;

        s_file(const vector<char> &buf)
            : buf(buf)
        {}
        uint32_t read(void *dst, uint32_t size)
        {
            if (index >= buf.size())
                return 0;
            if (index + size > buf.size())
                size = buf.size() - index;
            memcpy(dst, buf.data() + index, size);
            index += size;
            return size;
        }
        void skip(int n)
        {
            index += n;
        }
    };

    uint32_t table_id;
    char name[0x14];
    uint32_t unk1;
    uint32_t unk2;
    uint32_t unk3;
    uint32_t offset;
    uint32_t data_size;

    //
    vector<char> buf;
    //
    uint32_t number_of_fields;
    vector<field_value> fields;

    void extract_fields(const tab &tab);

    void load_index(FILE *f);
    void load_data(FILE *f);
};

struct db
{
    uint32_t number_of_values;

    tab t;
    vector<value> values;

    void load(FILE *f);
};