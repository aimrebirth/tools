/*
 * AIM mmp_extractor
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

struct mmp
{
    struct segment
    {
        uint32_t MagicNumber;
        uint32_t MiniLayer1[1089];
        uint32_t MiniLayer2[1089];
        uint32_t MiniLayer3[1089];
        uint32_t MiniLayer4[1089];
        float Heightmap[4225];
        uint32_t Infomap[4225];
        uint32_t Colormap[4225];
        uint32_t Shadowmap[4225];
        uint32_t Normalmap[4225];
    };

    uint32_t unk1;
    char unk2[0x80];
    uint32_t width;
    uint32_t height;
    uint32_t unk3;
    char unk4[0xA0];
    uint32_t unk5[7];
    char unk6[0xA0];
    char unk7[0x1AC];
    char unk8[0x1AC];
    uint32_t offset; // the beginning of segments

    vector<segment> segments;

    void load(FILE *f);
};