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

#include "types.h"

GameType gameType = GameType::Aim2;

void weather::load(buffer &b)
{
    READ_STRING(b, name);
    READ_STRING(b, unk0);
    READ(b, unk1);
    READ(b, smoke_1);
    READ(b, smoke_3);
    READ(b, smokeType);
    READ(b, unk2);
    READ_STRING(b, cloud_layer1);
    READ_STRING(b, cloud_layer2);
    READ(b, cloud_layer1_speed);
    READ(b, cloud_layer2_speed);
    READ(b, cloud_layer1_direction);
    READ(b, cloud_layer2_direction);
    READ_STRING(b, sun);
    READ(b, general_color);
    READ(b, sun_color);
    READ(b, moon_color);
    READ_STRING(b, moon);
    READ(b, probability);
    READ_STRING(b, day_night_gradient_name);
    READ_STRING(b, dawn_dusk_gradient_name);
    READ(b, dawn_dusk_color);
    READ(b, effects);
    READ(b, smoke_2);
    READ(b, smoke_4);
    READ(b, slider_3);
    READ(b, slider_1);
    READ(b, unk8);
}

void weather_group::load(buffer &b)
{
    READ(b, n_segs);
    segments.resize(n_segs);
    READ(b, name);
    for (auto &s : segments)
        s.load(b);
}

void water::load(buffer &b)
{
    READ(b, unk0);
    READ_STRING(b, name1);
    READ(b, unk1);
    READ(b, unk2);
    READ(b, unk3);
    READ(b, unk4);
    READ_STRING(b, name2);
    READ(b, unk5);
}

void water_group::load(buffer &b)
{
    while (!b.eof())
    {
        water w;
        w.load(b);
        segments.push_back(w);
    }
}
