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

#include <buffer.h>

using namespace std;

enum class FieldType : uint32_t
{
    String,
    Integer,
    Float,
};
string getSqlType(FieldType type);

struct table
{
    uint32_t id;
    std::string name;
    uint32_t unk4;

    void load(buffer &b);
};

struct field
{
    uint32_t table_id;
    uint32_t id;
    std::string name;
    FieldType type;

    void load(buffer &b);
};

struct tab
{
    uint32_t number_of_tables;
    uint32_t number_of_fields;

    map<uint32_t, table> tables;
    map<uint32_t, field> fields;

    void load(buffer &b);
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
    uint32_t table_id;
    std::string name;
    uint32_t offset;
    uint32_t data_size;
    vector<field_value> fields;

    void load_index(buffer &b);
    void load_fields(const tab &tab, buffer &b);
};

struct db
{
    uint32_t number_of_values;

    tab t;
    vector<value> values;

    void load(buffer &b);
};