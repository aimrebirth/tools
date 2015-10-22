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

#pragma once

#include <stdint.h>

typedef uint16_t color_rgb565;

struct color
{
    union
    {
        struct
        {
            uint8_t b;
            uint8_t g;
            uint8_t r;
            uint8_t a;
        };
        uint8_t byte[4];
        uint32_t data;
    };

    color() {}
    color(uint8_t b, uint8_t g, uint8_t r, uint8_t a) 
        : b(b), g(g), r(r), a(a)
    {}
    color(const color_rgb565 &c)
    {
        static uint8_t Table5[] = {
            0, 8, 16, 25, 33, 41, 49, 58, 66, 74, 82, 90, 99, 107, 115, 123, 132,
            140, 148, 156, 165, 173, 181, 189, 197, 206, 214, 222, 230, 239, 247, 255 };
        static uint8_t Table6[] = {
            0, 4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 45, 49, 53, 57, 61, 65, 69,
            73, 77, 81, 85, 89, 93, 97, 101, 105, 109, 113, 117, 121, 125, 130, 134, 138,
            142, 146, 150, 154, 158, 162, 166, 170, 174, 178, 182, 186, 190, 194, 198,
            202, 206, 210, 215, 219, 223, 227, 231, 235, 239, 243, 247, 251, 255 };

        b = Table5[c & 0x1F];
        g = Table6[(c >> 5) & 0x3F];
        r = Table5[(c >> 11) & 0x1F];
    }

    operator uint32_t() const
    {
        return data;
    }
};
