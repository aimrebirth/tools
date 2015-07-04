/*
 * AIM tm_converter
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

#include <algorithm>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdint.h>
#include <string>

#include <common.h>

using namespace std;

void convert_simple(buffer &dst, buffer &src, int width, int height)
{
    int size = width * height * 2;
    for (int i = 0; i < size; i++)
    {
        uint8_t c;
        READ(src, c);
        uint8_t lo = c & 0x0F;
        uint8_t hi = (c & 0xF0) >> 4;
        dst.write(uint8_t((lo << 4) | lo));
        dst.write(uint8_t((hi << 4) | hi));
    }
}

void tm2tga(string fn)
{
    int width, height;
    int dxt5 = 0;

    buffer src(readFile(fn));
    READ(src, width);
    READ(src, height);
    src.seek(0x10);
    src.read(&dxt5, 1);
    src.seek(0x4C);
    
    // http://paulbourke.net/dataformats/tga/
    buffer dst;
    dst.write(uint8_t(0xE)); // idlength (comment length)
    dst.write(uint8_t(0)); // colourmaptype
    dst.write(uint8_t(2)); // datatypecode
    dst.write(uint16_t(0)); // colourmaporigin
    dst.write(uint16_t(0)); // colourmaplength
    dst.write(uint8_t(0)); // colourmapdepth
    dst.write(uint16_t(0)); // x_origin
    dst.write(uint16_t(0)); // y_origin
    dst.write(uint16_t(width)); // width
    dst.write(uint16_t(height)); // height
    dst.write(uint8_t(32)); // bitsperpixel
    dst.write(uint8_t(0x28)); // imagedescriptor

    const char *label = "AIMTMConverter";
    dst.write(label, strlen(label));

    if (dxt5)
    {
        //convert_dxt5(dst, src, width, height);
        throw std::logic_error("dxt5 converter is not implemented!");
    }
    else
        convert_simple(dst, src, width, height);

    transform(fn.begin(), fn.end(), fn.begin(), ::tolower);
    fn = fn.substr(0, fn.rfind(".tm")) + ".tga";
    writeFile(fn, dst.buf());
}

int main(int argc, char *argv[])
try
{
    if (argc != 2)
    {
        printf("Usage: %s file.tm\n", argv[0]);
        return 1;
    }
    tm2tga(argv[1]);
    return 0;
}
catch (std::exception &e)
{
    printf("%s\n", argv[1]);
    printf("error: %s\n", e.what());
    return 1;
}
catch (...)
{
    printf("%s\n", argv[1]);
    printf("error: unknown exception\n");
    return 1;
}
