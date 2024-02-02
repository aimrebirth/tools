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

#include "pak.h"

#include <algorithm>
#include <assert.h>
#include <stdio.h>
#include <iostream>

#include "decode.h"

#define FREAD(var) fread(&var, 1, sizeof(var), f)

void header::load(FILE *f)
{
    FREAD(unk1);
    FREAD(unk2);
    FREAD(number_of_files);
    FREAD(unk3);
    FREAD(number_of_chunks);
    FREAD(chunk_size);
    FREAD(unk5);
}

void record::load(FILE *f)
{
    char n[0x50];
    FREAD(n);
    name = n;
    FREAD(pos);
    FREAD(len);
}

void record::write(string name, const vector<char> &data) const
{
    name += "\\" + string(this->name);
    string dir = name.substr(0, name.rfind('\\'));
    system(string("mkdir " + dir + " 2> nul").c_str());
    FILE *f = fopen(name.c_str(), "wb");
    if (!f)
        return;
    fwrite(data.data(), 1, data.size(), f);
    fclose(f);
}

int record::read(pak *pak, void *output, int size)
{
    int file_start_pos = offset + pos;
    int segment = file_start_pos / pak->h.chunk_size;
    offset += size;

    pak->segments[segment].decompress(segment);

    auto file_start_pos2 = file_start_pos - segment * pak->h.chunk_size;
    auto size3 = size;
    if (pak->h.chunk_size - file_start_pos2 < size)
        size3 = pak->h.chunk_size - file_start_pos2;
    memcpy(output, pak->segments[segment].decoded + file_start_pos2, size3);

    auto size_diff = size - size3;
    uint32_t diff = 0;
    for (char *out = (char *)output + size3; size_diff > 0; out += diff)
    {
        segment++;
        pak->segments[segment].decompress(segment);

        diff = pak->h.chunk_size;
        if (diff >= size_diff)
            diff = size_diff;
        memcpy(out, pak->segments[segment].decoded, 4 * (diff >> 2));
        size_diff -= diff;
        memcpy(
            out + 4 * (diff >> 2),
            pak->segments[segment].decoded + 4 * (diff >> 2),
            diff & 3);
    }
    return size;
}

void segment::load_header(FILE *f)
{
    FREAD(unk1);
    FREAD(algorithms);
    FREAD(offset);
}

void segment::load_segment()
{
    auto f = file;

    fseek(f, offset, SEEK_SET);
    /*if (algorithms == 0)
    {
        std::cerr << "Something is wrong. Maybe you trying to open aim2 files?\n";
        std::cerr << "They can be opened with SDK extractor.\n";
        throw std::runtime_error("error");
    }*/

    FREAD(size1);
    size2 = size1;
    if (algorithms & DA_2)
    {
        FREAD(size2);
        fread(&decoded[0], 1, size2, f);
    }
    else
    {
        fread(&encoded[0], 1, size1, f);
    }
}

void segment::decompress(int segment_id)
{
    load_segment();

    if (algorithms & DA_2) {
        decode_f2((char*)decoded, size2, (char*)encoded);
    }
    if (algorithms & RLE_2_bytes) {
        decode_rle((uint16_t *)encoded, size1, (uint16_t *)decoded);
    } else if (algorithms & RLE_1_byte) {
        decode_rle((uint8_t *)encoded, size1, (uint8_t *)decoded);
    }
    if (algorithms == None) {
        decoded = encoded;
    }
}

void pak::load(FILE *f)
{
    h.load(f);
    encoded.resize(h.chunk_size * 4);
    decoded.resize(h.chunk_size * 4);

    int n = h.number_of_files;
    while (n--)
    {
        record rec;
        rec.load(f);
        files[rec.name] = rec;
    }

    n = h.number_of_chunks;
    while (n--)
    {
        segment t;
        t.load_header(f);
        t.file = f;
        t.encoded = encoded.data();
        t.decoded = decoded.data();
        segments.push_back(t);
    }
}
