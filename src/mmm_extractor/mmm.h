/*
 * AIM mmm_extractor
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

#include <stdint.h>
#include <string>
#include <vector>

#include <common.h>

struct dxt5_record
{
    char unk[0x10];
};

struct mmm
{
    uint32_t unk1;
    uint32_t unk2;
    uint32_t width;
    uint32_t height;
    std::vector<dxt5_record> data;

    void load(buffer &b);
};