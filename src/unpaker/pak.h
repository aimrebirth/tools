/*
 * AIM 1 unpaker
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

#pragma once

#include <map>
#include <stdint.h>
#include <string>
#include <vector>

using namespace std;

struct header
{
    uint32_t unk1;
    uint16_t unk2;
    uint16_t number_of_files;
    uint16_t unk3;
    uint32_t number_of_chunks;
    int32_t chunk_size;
    uint32_t unk5;

    void load(FILE *f);
};

struct record
{
    std::string name;
    uint32_t pos;
    uint32_t len;

    //
    vector<char> data;
    int offset = 0;

    void load(FILE *f);
    void write(string name, const vector<char> &data) const;
    int read(struct pak *pak, void *output, int size);
};

struct segment
{
    enum decode_algorithm : uint32_t
    {
        None = 0x0,
        RLE_2_bytes = 0x1,
        RLE_1_byte  = 0x2,
        DA_1 = 0x4, // not used
        DA_2 = 0x8,
    };

    uint32_t unk1; // some file offset? trash?
    decode_algorithm algorithms;
    uint32_t offset;

    uint32_t size1;
    uint32_t size2;

    //
    FILE *file = 0;
    uint8_t* encoded;
    uint8_t* decoded;

    void load_header(FILE *f);
    void load_segment();
    void decompress(int segment_id);
};

struct pak
{
    header h;
    vector<segment> segments;
    map<string, record> files;

    //
    vector<uint8_t> encoded;
    vector<uint8_t> decoded;

    void load(FILE *f);
};
