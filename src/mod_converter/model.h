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
    F_UNK0  =   0x4,
};

enum class AdditionalParameter : uint32_t
{
    None,
    DetalizationCoefficient
};

enum class ModelRotation : uint32_t
{
    None,
    Vertical,
    Horizontal,
    Other
};

enum class BlockType : uint32_t
{
    VisibleObject,
    HelperObject,
    BitmapAlpha,
    BitmapGrass,
    ParticleEmitter
};

enum class EffectType : uint32_t
{
    Texture = 0x0,
    TextureWithGlareMap = 0x1,
    TextureWithGlareMapAndMask = 0x32,
    AlphaTextureDoubleSided = 0x6,
    MaterialOnly = 0x14,
};

struct Vector4
{
    float x;
    float y;
    float z;
    float w;

    std::string print() const;
};

struct vertex
{
    float vX;
    float vZ;
    float vY;

    float unk0;

    float nX;
    float nZ;
    float nY;

    float t1;
    float t2;

    void load(const buffer &b, uint32_t flags);

    std::string printVertex() const;
    std::string printNormal() const;
    std::string printTex() const;
};

struct triangle
{
    uint16_t x;
    uint16_t y;
    uint16_t z;
};

struct animation
{
    // +1 +0.5 -0.5 +1
    struct segment
    {
        struct unk_float6
        {
            float unk[6];
        };

        uint32_t n;
        uint32_t unk0;
        uint32_t unk1;

        std::vector<uint16_t> model_polygons;
        std::vector<unk_float6> unk2;

        void loadHeader(const buffer &b);
        void loadData(const buffer &b);
    };

    uint32_t type;
    char name[0xC];
    segment segments[4];

    virtual void load(const buffer &b);
};

struct damage_model
{
    uint32_t n_polygons;
    float unk8[3];
    char name[0x3C];
    std::vector<uint16_t> model_polygons;
    uint8_t unk6;
    uint32_t flags;
    uint32_t n_vertex;
    uint32_t n_triangles;
    std::vector<vertex> vertices;
    std::vector<triangle> damage_triangles;

    virtual void load(const buffer &b);
};

struct material
{
    Vector4 diffuse;
    Vector4 ambient;
    Vector4 specular;
    Vector4 emissive;
    float power;
};

struct rotation
{
    ModelRotation type;
    float speed;
    // center of rotating axis
    float x;
    float y;
    float z;
};

struct additional_parameters
{
    AdditionalParameter params;
    float detalization_koef;
};

struct block
{
    // header
    BlockType type;
    std::string name;
    std::string tex_mask;
    std::string tex_spec;
    std::string tex3;
    std::string tex4;
    union // LODs
    {
        struct
        {
            uint8_t lod1 : 1;
            uint8_t lod2 : 1;
            uint8_t lod3 : 1;
            uint8_t lod4 : 1;
            uint8_t      : 4;
        } _;
        uint32_t LODs;
    };
    uint32_t unk2[3];
    uint32_t unk3;
    uint32_t size;
    uint32_t unk4[10];

    // data
    uint32_t n_animations;
    material material;

    //unk (anim + transform settings?)
    EffectType effect;
    uint32_t unk7;
    float unk9;
    uint32_t unk10;
    uint32_t auto_animation;
    float animation_cycle;
    float unk8;
    uint32_t unk11;
    uint32_t unk12;
    uint32_t triangles_mult_7;
    //

    additional_parameters additional_params;
    uint32_t n_damage_models;
    rotation rot;
    uint32_t flags;
    uint32_t n_vertex;
    uint32_t n_triangles;
    std::vector<vertex> vertices;
    std::vector<triangle> triangles;

    // animations
    std::vector<animation> animations;
    std::vector<damage_model> damage_models;

    void load(const buffer &b);
    std::string printMtl(const std::string &mtl_name) const;
    std::string printObj(const std::string &mtl_name) const;
};

struct model
{
    int n_blocks;
    char header[0x40];
    std::vector<block> blocks;

    void load(const buffer &b);
};
