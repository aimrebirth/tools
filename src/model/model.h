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

#include "types.h"

#include <primitives/yaml.h>

#include <stdint.h>
#include <string>
#include <vector>

const float scale_mult = 30.f;
const std::string texture_extension = ".TM.bmp";

class buffer;

enum
{
    F_USE_W_COORDINATE  =   0x4, // F_USE_QUANTERNION/F_QUANTERNION?
};

enum class AdditionalParameter : uint32_t
{
    None,
    DetalizationCoefficient,
};

enum class ModelRotation : uint32_t
{
    None,
    Vertical,
    Horizontal,
    Other,
};

enum class BlockType : uint32_t
{
    VisibleObject,
    HelperObject,
    BitmapAlpha,
    BitmapGrass,
    ParticleEmitter,
};

enum class MaterialType : uint32_t
{
    Texture                                                 = 0x0,
    TextureWithGlareMap                                     = 0x1,
    AlphaTextureNoGlare                                     = 0x2, // TextureWithoutGlareMap
    AlphaTextureWithOverlap                                 = 0x3,
    TextureWithGlareMap2                                    = 0x4, // ??? also <- 4
    AlphaTextureDoubleSided                                 = 0x6,
    DetalizationObjectGrass                                 = 0x8,
    Fire                                                    = 0x9,
    MaterialOnly                                            = 0x14,
    TextureWithDetalizationMap                              = 0x1A, // from Viewer: (AdditionalParameter 1)
    DetalizationObjectStone                                 = 0x1F,
    TextureWithDetalizationMapWithoutModulation             = 0x20, // from Viewer: (AdditionalParameter 1)
    TiledTexture                                            = 0x22,
    TextureWithGlareMapAndMask                              = 0x32,
    TextureWithMask                                         = 0x35,
    Fire2                                                   = 0x3D,
};

template <typename T>
struct aim_vector3 : vector3<T>
{
    using base = vector3<T>;

    void load(const buffer &b);
};

struct aim_vector4 : aim_vector3<float>
{
    using base = aim_vector3<float>;

    float w = 1.0f;

    std::string print() const;
    void load(const buffer &b, uint32_t flags = 0);
};

struct uv
{
    float u;
    float v;
};

struct vertex
{
    aim_vector4 coordinates;
    aim_vector3<float> normal;
    uv texture_coordinates;

    void load(const buffer &b, uint32_t flags);

    std::string printVertex(bool rotate_x_90 = false) const;
    std::string printNormal(bool rotate_x_90 = false) const;
    std::string printTex() const;
};

using face = aim_vector3<uint16_t>;

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
        std::vector<uint16_t> model_polygons;

        // unk
        uint32_t unk0;
        uint32_t unk1;
        std::vector<unk_float6> unk2;

        void loadHeader(const buffer &b);
        void loadData(const buffer &b);
    };

    uint32_t type;
    std::string name;
    segment segments[4];

    virtual void load(const buffer &b);
};

struct damage_model
{
    std::string name;
    std::vector<uint16_t> model_polygons;
    uint32_t flags;
    std::vector<vertex> vertices;
    std::vector<face> faces;

    uint8_t unk6;
    float unk8[3];

    virtual void load(const buffer &b);
};

struct mat_color
{
    float r;
    float g;
    float b;
    float alpha;

    std::string print() const;
};

struct material
{
    mat_color ambient;
    mat_color diffuse;
    mat_color specular;
    mat_color emissive;
    float power;

    void load(const buffer &b);
};

struct rotation
{
    ModelRotation type;
    float speed;
    vector3<float> center_of_rotating_axis;
};

struct additional_parameters
{
    AdditionalParameter params;
    float detalization_koef;
};

// mechlab .asc format?
struct block
{
    struct header
    {
        struct texture
        {
            std::string name;
            uint32_t number; // AimR

            void load(const buffer &b);
        };

        BlockType type;
        std::string name;
        texture mask;
        texture spec;
        texture tex3;
        texture tex4;
        union // LODs
        {
            struct
            {
                uint8_t lod1 : 1;
                uint8_t lod2 : 1;
                uint8_t lod3 : 1;
                uint8_t lod4 : 1;
                uint8_t : 4;
            } LODs;
            uint32_t all_lods;
        };

        // stuff
        uint32_t size;

        // unk
        uint32_t unk2[3];
        uint32_t unk3;
        float unk4[10];

        void load(const buffer &b);
    };

    // for save
    struct block_info
    {
        aim_vector4 min;
        aim_vector4 max;
    };

    header h;

    // data
    material mat;
    MaterialType mat_type;

    //unk (anim + transform settings?)
    uint32_t auto_animation;
    float animation_cycle;
    uint32_t triangles_mult_7; // Tri-mesh. flags?
    //

    additional_parameters additional_params;
    rotation rot;
    uint32_t flags;
    std::vector<vertex> vertices;
    std::vector<face> faces; // triangles

    // animations
    std::vector<animation> animations;
    std::vector<damage_model> damage_models;

    // unk
    uint32_t unk7;
    float unk9;
    uint32_t unk10;
    float unk8;
    uint32_t unk11;
    uint32_t unk12;

    void load(const buffer &b);
    void loadPayload(const buffer &b);

    std::string printMtl() const;
    std::string printObj(int group_offset, bool rotate_x_90 = false) const;
    block_info save(yaml &root) const;

    bool canPrint() const;
    bool isEngineFx() const;
};

struct model
{
    std::vector<block> blocks;

    void load(const buffer &b);

    void print(const std::string &fn) const;
    void printFbx(const std::string &fn) const;
    void save(yaml &root) const;
};
