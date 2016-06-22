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

#include <assert.h>
#include <deque>
#include <vector>

#include <bmp.h>
#include <tga.h>

template <class T>
class mat
{
public:
    using type = T;

public:
    mat(int w = 0, int h = 0)
    {
        width = w < 0 ? 0 : w;
        height = h < 0 ? 0 : h;
        data.resize(width * height, T());
    }

    T &operator()(int row, int col)
    {
        assert(!(row >= height || col >= width || row < 0 || col < 0));
        return data[row * width + col];
    }
    const T &operator()(int row, int col) const
    {
        return (*const_cast<mat *>(this))(row, col);
    }

    void clean()
    {
        std::fill(data.begin(), data.end(), T());
    }

    int getWidth() const { return width; }
    int getHeight() const { return height; }
    int size() const { return width * height; }
    int getBytesLength() const { return size() * sizeof(T); }
    int getPos(const T *const elem) const { return elem - &data[0]; }
    const std::vector<T> &getData() const { return data; }
    std::vector<T> &getData() { return data; }

private:
    std::vector<T> data;
    int width;
    int height;
};

template<class T>
void write_mat_bmp(const std::string &filename, const mat<T> &m)
{
    FILE *f = fopen(filename.c_str(), "wb");
    if (f == nullptr)
        return;
    BITMAPFILEHEADER h = { 0 };
    h.bfType = 0x4D42;
    h.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + m.size() * sizeof(T);
    h.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    BITMAPINFOHEADER i = { 0 };
    i.biSize = sizeof(i);
    i.biWidth = m.getWidth();
    i.biHeight = m.getHeight();
    i.biPlanes = 1;
    i.biBitCount = sizeof(T) * 8;
    i.biCompression = 0;
    i.biSizeImage = 0;
    i.biXPelsPerMeter = 0;
    i.biYPelsPerMeter = 0;
    i.biClrUsed = 0;
    i.biClrImportant = 0;
    fwrite(&h, sizeof(BITMAPFILEHEADER), 1, f);
    fwrite(&i, sizeof(BITMAPINFOHEADER), 1, f);
    fwrite(&m(0, 0), m.size() * sizeof(T), 1, f);
    fclose(f);
}

template<class T>
void write_mat_tga(const std::string &filename, const mat<T> &m)
{
    FILE *f = fopen(filename.c_str(), "wb");
    if (f == nullptr)
        return;

    tga t;
    t.width = m.getWidth();
    t.height = m.getHeight();

    // header
    fwrite(&t, sizeof(tga), 1, f);
    fwrite(t.label(), t.idlength, 1, f);

    // data
    fwrite(&m(0, 0), m.size() * sizeof(T), 1, f);
    fclose(f);
}
