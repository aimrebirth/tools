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

#include "mmap.h"
#include "../model/model.h"

#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <string>

#pragma pack(push, 1)
struct mod {
    using char20 = char[0x20];
    struct block {
        struct header {
            using texture = char20;

            BlockType type;
            char20 name;
            texture mask;
            texture spec;
            texture tex3;
            texture tex4;
            uint32_t all_lods;
            uint32_t unk2[3];
            uint32_t unk3;
            uint32_t size;
            float unk4[10];
        };

        header h;
        uint32_t n_animations;
        material m;
        MaterialType mat_type;
    };

    int n_blocks;
    char header_[0x40];

    auto blocks() {
        auto base = (uint8_t*)&header_ + sizeof(header_);
        std::vector<block*> b;
        auto n = n_blocks;
        while (n--) {
            b.push_back((block*)base);
            base += sizeof(block::header) + b.back()->h.size;
        }
        return b;
    }
};
#pragma pack(pop)

auto read_model(const path &fn) {
    primitives::templates2::mmap_file<uint8_t> f{fn, primitives::templates2::mmap_file<uint8_t>::rw{}};
    auto &m = *(mod*)f.p;
    auto v = m.blocks();
    std::set<std::string> textures;
    for (auto &&b : v) {
        if (b->h.type != BlockType::VisibleObject) {
            continue;
        }
        textures.insert(b->h.mask);
        textures.insert(b->h.spec);
        // we do not use tex3,tex4 atm
        //
        switch (b->mat_type) {
        case MaterialType::Texture: // works in m1
        case MaterialType::TextureWithGlareMap: // works in m1
        case MaterialType::AlphaTextureNoGlare:
        case MaterialType::AlphaTextureWithOverlap:
        case MaterialType::TextureWithGlareMap2: // works in m1
        case MaterialType::AlphaTextureDoubleSided:
        case MaterialType::MaterialOnly:
        case MaterialType::TextureWithDetalizationMap:
        case MaterialType::DetalizationObjectStone:
        case MaterialType::TextureWithDetalizationMapWithoutModulation:
        case MaterialType::TiledTexture:
            break;
        case MaterialType::TextureWithGlareMapAndMask:
            b->mat_type = MaterialType::TextureWithGlareMap;
            break;
        case MaterialType::TextureWithMask:
            b->mat_type = MaterialType::Texture;
            break;
        default:
            std::cout << b->h.name << ": "
                        << "warning: unknown material type " << (int)b->mat_type << " \n";
            break;
        }
    }
    write_lines(path{fn}+=".textures.txt",textures);
}

void convert_model(const path &fn)
{
    read_model(fn);
}

int main(int argc, char *argv[])
{
    cl::opt<path> p(cl::Positional, cl::desc("<mod file>"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    if (fs::is_regular_file(p))
        convert_model(p);
    else if (fs::is_directory(p))
    {
        auto files = enumerate_files(p, false);
        for (auto &f : FilesSorted(files.begin(), files.end()))
        {
            if (f.has_extension())
                continue;
            std::cout << "processing: " << f << "\n";
            try
            {
                convert_model(f);
            }
            catch (std::exception &e)
            {
                std::cout << "error: " << e.what() << "\n";
            }
        }
    }
    else
        throw std::runtime_error("No such file or directory: " + to_printable_string(normalize_path(p)));

    return 0;
}
