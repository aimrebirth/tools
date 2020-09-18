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

#include <buffer.h>

#include <primitives/filesystem.h>

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

enum class FieldType : uint32_t
{
    String,
    Integer,
    Float,
};
std::string getSqlType(FieldType type);

struct table
{
    uint32_t id;
    std::string name;
    uint32_t unk4;

    void load(const buffer &b);
};

struct field
{
    uint32_t table_id = -1;
    uint32_t id;
    std::string name;
    FieldType type;

    void load(const buffer &b);
};

struct tab
{
    uint32_t number_of_tables;
    uint32_t number_of_fields;

    std::map<uint32_t, table> tables;
    std::map<uint32_t, field> fields;

    void load(const buffer &b);
};

struct field_value
{
    uint32_t field_id;
    uint32_t size;

    int i = 0;
    float f = 0.f;
    std::string s;
};

struct value
{
    uint32_t table_id;
    std::string name;
    uint32_t offset;
    uint32_t data_size;
    std::vector<field_value> fields;

    void load_index(const buffer &b);
    void load_fields(const tab &tab, buffer &b);
};

struct db
{
    uint32_t number_of_values = 0;

    tab t;
    std::vector<value> values;

    void load(const buffer &b);
    void open(const path &basename);
};
