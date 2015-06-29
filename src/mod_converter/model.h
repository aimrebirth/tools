/*
 * AIM mod_converter
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

#include <stdint.h>
#include <string>
#include <vector>

class buffer;

enum
{
    F_WIND  =   0x4,
};

struct vertex
{
    float vX;
    float vZ;
    float vY;

    float wind;

    float nX;
    float nZ;
    float nY;

    float t1;
    float t2;

    void load(buffer &b, uint32_t flags);

    std::string printVertex() const;
    std::string printNormal() const;
    std::string printTex() const;
};

typedef uint16_t triangle;

struct unk_float3x4
{
    float unk[4][3];
};

struct unk_float6
{
    float unk[6];
};

struct segment
{
    uint32_t type;

    virtual void load(buffer &b) = 0;
};

struct segment1 : public segment
{
    char name[0xC];
    uint32_t unk0[4][3];
    std::vector<triangle> triangles;
    std::vector<unk_float3x4> unk1;
    
    virtual void load(buffer &b);
};

struct segment2 : public segment
{
    struct repeater
    {
        uint32_t unk2;
        float unk8[3];
        char unk3[0x3C];
        std::vector<uint16_t> triangles2;
        uint8_t unk6;
        uint32_t flags;
        uint32_t n_vertex;
        uint32_t n_triangles;
        std::vector<vertex> vertices;
        std::vector<uint16_t> triangles;
        
        virtual void load(buffer &b);
    };

    char name[0xC];
    uint32_t unk0[4][3];
    std::vector<triangle> triangles;
    std::vector<unk_float6> unk1;
    std::vector<unk_float6> unk1_1;
    std::vector<repeater> unk2;
    
    virtual void load(buffer &b);
};

struct segment6 : public segment1
{
    virtual void load(buffer &b);
};

struct fragment
{
    // header
    uint32_t type;
    char name0[0x20];
    char name1[0x20];
    char name2[0x20];
    char name3[0x20];
    char name4[0x20];
    uint32_t unk0;
    uint32_t unk1;
    uint32_t unk2[2];
    uint32_t unk3;
    uint32_t size;
    uint32_t unk4[10];

    // data
    uint32_t n_segments;
    char header[0x68];
    uint32_t triangles_mult_7;
    char unk10[0x20];
    uint32_t flags;
    uint32_t n_vertex;
    uint32_t n_triangles;
    std::vector<vertex> vertices;
    std::vector<uint16_t> triangles;

    // segments
    std::vector<segment *> segments;

    void load(buffer &b);
};

struct model
{
    int n_fragments;
    char header[0x40];
    std::vector<fragment> fragments;

    void load(buffer &b);
    void writeObj(std::string fn);
};
