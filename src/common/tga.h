#pragma once

#include "buffer.h"

#include <stdint.h>
#include <string.h>

#pragma pack(push, 1)

// http://paulbourke.net/dataformats/tga/
struct tga
{
    uint8_t idlength; // label length
    uint8_t colourmaptype = 0;
    uint8_t datatypecode = 2;
    uint16_t colourmaporigin = 0;
    uint16_t colourmaplength = 0;
    uint8_t colourmapdepth = 0;
    uint16_t x_origin = 0;
    uint16_t y_origin = 0;
    uint16_t width;
    uint16_t height;
    uint8_t bitsperpixel = 32;
    uint8_t imagedescriptor = 0x28;

    tga()
    {
        idlength = (uint8_t)strlen(label());
    }
    constexpr const char *label() const
    {
        return "AIMTMConverter";
    }

    void write(buffer &b)
    {
        b.write(idlength);
        b.write(colourmaptype);
        b.write(datatypecode);
        b.write(colourmaporigin);
        b.write(colourmaplength);
        b.write(colourmapdepth);
        b.write(x_origin);
        b.write(y_origin);
        b.write(width);
        b.write(height);
        b.write(bitsperpixel);
        b.write(imagedescriptor);
        b.write(label(), idlength);
    }
};

#pragma pack(pop)
