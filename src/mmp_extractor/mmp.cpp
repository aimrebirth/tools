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

#include "mmp.h"

#define FREAD(var) fread(&var, 1, sizeof(var), f)
#define FREAD_N(var, n) fread(&var, 1, n, f)

size_t file_size(FILE *f)
{
    auto old = ftell(f);
    fseek(f, 0, SEEK_END);
    auto sz = ftell(f);
    fseek(f, old, SEEK_SET);
    return sz;
}

void mmp::load(FILE *f)
{
    FREAD(unk1);
    FREAD(unk2);
    FREAD(width);
    FREAD(height);
    FREAD(unk3);
    FREAD(unk4);
    FREAD(unk5);
    FREAD(unk6);
    FREAD(unk7);
    FREAD(unk8);

    int n_seg = width / 64 * height / 64;
    int sz = n_seg * sizeof(segment);
    int fsz = file_size(f);
    int off = fsz - sz;
    fseek(f, off, SEEK_SET);

    segments = vector<segment>(n_seg);
    for (int i = 0; i < n_seg; i++)
        FREAD(segments[i]);

    assert(ftell(f) == fsz);
}