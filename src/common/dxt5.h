/*
 * AIM tools
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

#include "buffer.h"
#include "color.h"
#include "mat.h"

struct dxt5_block
{
    union
    {
        uint64_t alpha_part;
        uint8_t a[8];
    };
    union
    {
        uint64_t color_part;
        uint16_t c[4];
    };

    //
    uint8_t alpha_table[8];
    color color_table[4];
    union
    {
        color pixels[16];
        color pixel_mat[4][4];
    };

    dxt5_block() {}
    void load(const buffer &b)
    {
        READ(b, alpha_part);
        READ(b, color_part);
    }
    void unpack()
    {
        // alpha
        auto &a = alpha_table;
        a[0] = this->a[0];
        a[1] = this->a[1];
        if (a[0] > a[1])
        {
            a[2] = double(6 * a[0] + 1 * a[1]) / 7.0;
            a[3] = double(5 * a[0] + 2 * a[1]) / 7.0;
            a[4] = double(4 * a[0] + 3 * a[1]) / 7.0;
            a[5] = double(3 * a[0] + 4 * a[1]) / 7.0;
            a[6] = double(2 * a[0] + 5 * a[1]) / 7.0;
            a[7] = double(1 * a[0] + 6 * a[1]) / 7.0;
        }
        else
        {
            a[2] = double(4 * a[0] + 1 * a[1]) / 5.0;
            a[3] = double(3 * a[0] + 2 * a[1]) / 5.0;
            a[4] = double(2 * a[0] + 3 * a[1]) / 5.0;
            a[5] = double(1 * a[0] + 4 * a[1]) / 5.0;
            a[6] = 0;
            a[7] = 255;
        }

        // color
        auto &c = color_table;
        c[0] = color(this->c[0]);
        c[1] = color(this->c[1]);
        if (this->c[0] > this->c[1])
        {
            c[2] = interpolate(c[0], c[1], 2.f / 3.f);
            c[3] = interpolate(c[0], c[1], 1.f / 3.f);
        }
        else
        {
            c[2] = interpolate(c[0], c[1], 1.f / 2.f);
            c[3].data = 0;
        }

        // result
        for (int p = 0; p < 16; p++)
        {
            pixels[p] = c[(color_part >> (32 + p * 2)) & 0b11];
            pixels[p].a = a[(alpha_part >> (16 + p * 3)) & 0b111];
        }
    }
    color interpolate(color c0, color c1, float m)
    {
        color r;
        for (int i = 0; i < 4; i++)
            r.byte[i] = c0.byte[i] * m + c1.byte[i] * (1 - m);
        return r;
    }
};

struct dxt5
{
    uint32_t width;
    uint32_t height;
    std::vector<dxt5_block> blocks;

    void load(const buffer &b)
    {
        READ(b, width);
        READ(b, height);
        load_blocks(b);
    }
    void load_blocks(const buffer &b)
    {
        blocks.resize(width * height / 16);
        for (auto &d : blocks)
        {
            d.load(b);
            d.unpack();
        }
    }
    mat<uint32_t> unpack_mmm()
    {
        mat<uint32_t> m(width, height);
        auto big_xsegs = width / 64;
        auto big_ysegs = height / 64;
        for (int seg = 0; seg < blocks.size(); seg++)
        {
            auto &d = blocks[seg];
            int big_seg = seg / 256;
            auto big_xseg = big_seg % big_xsegs;
            auto big_yseg = big_seg / big_xsegs;
            auto xseg = seg % 16 + big_xseg * 16;
            auto yseg = seg % 256 / 16 + big_yseg * 16;
            for (int i = 0; i < 4; i++)
                memcpy(&m(height - 1 - (yseg * 4 + i), xseg * 4), d.pixel_mat[i], 16);
        }
        return m;
    }
    mat<uint32_t> unpack_tm()
    {
        mat<uint32_t> m(width, height);
        auto xsegs = width / 4;
        for (int seg = 0; seg < blocks.size(); seg++)
        {
            auto &d = blocks[seg];
            auto xseg = seg % xsegs;
            auto yseg = seg / xsegs;
            for (int i = 0; i < 4; i++)
                memcpy(&m(height - 1 - (yseg * 4 + i), xseg * 4), d.pixel_mat[i], 16);
        }
        return m;
    }
};
