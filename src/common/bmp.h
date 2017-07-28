#pragma once

#include <stdint.h>

#pragma pack(push, 1)

struct bmp_header
{
    int16_t bfType;
    int32_t bfSize;
    int16_t bfReserved1;
    int16_t bfReserved2;
    int32_t bfOffBits;
};

struct bmp_info_header
{
    int32_t biSize;
    int32_t biWidth;
    int32_t biHeight;
    int16_t biPlanes;
    int16_t biBitCount;
    int32_t biCompression;
    int32_t biSizeImage;
    int32_t biXPelsPerMeter;
    int32_t biYPelsPerMeter;
    int32_t biClrUsed;
    int32_t biClrImportant;
};

struct rgb_quad
{
    int8_t rgbBlue;
    int8_t rgbGreen;
    int8_t rgbRed;
    int8_t rgbReserved;
};

struct bmp_info
{

    bmp_info_header bmiHeader;
    rgb_quad bmiColors[1];
};

#pragma pack(pop)
