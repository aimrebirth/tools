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

#include <bmp.h>
#include <tga.h>

#include <primitives/filesystem.h>

#include <assert.h>
#include <deque>
#include <vector>

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

    T &operator()(int i)
    {
        return data[i];
    }
    const T &operator()(int i) const
    {
        return (*const_cast<mat *>(this))(i);
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

    auto begin() { return data.begin(); }
    auto end() { return data.end(); }
    auto begin() const { return data.begin(); }
    auto end() const { return data.end(); }

    // left/right
    mat mirror()
    {
        int cols = width;
        int rows = height;
        mat<uint32_t> m(width, height);
        for (int row = 0; row < rows; row++)
        {
            for (int col = 0; col < cols; col++)
            {
                auto &o = operator()(row * cols + col);
                auto &n = m(row * cols + (cols - 1 - col));
                n = o;
            }
        }
        return m;
    }

    // up/down
    mat flip()
    {
        int cols = width;
        int rows = height;
        mat<uint32_t> m(width, height);
        for (int row = 0; row < rows; row++)
        {
            for (int col = 0; col < cols; col++)
            {
                auto &o = operator()(row * cols + col);
                auto &n = m((rows - 1 - row) * cols + col);
                n = o;
            }
        }
        return m;
    }

private:
    std::vector<T> data;
    int width;
    int height;
};

inline void write_mat_bmp(const path &filename, int width, int height, int bits, const uint8_t *b, size_t s)
{
    auto f = primitives::filesystem::fopen(filename, "wb");
    if (f == nullptr)
        return;
    bmp_header h = { 0 };
    h.bfType = 0x4D42;
    h.bfSize = sizeof(bmp_header) + sizeof(bmp_info_header) + s;
    h.bfOffBits = sizeof(bmp_header) + sizeof(bmp_info_header);
    bmp_info_header i = { 0 };
    i.biSize = sizeof(i);
    i.biWidth = width;
    i.biHeight = height;
    i.biPlanes = 1;
    i.biBitCount = bits;
    i.biCompression = 0;
    i.biSizeImage = 0;
    i.biXPelsPerMeter = 0;
    i.biYPelsPerMeter = 0;
    i.biClrUsed = 0;
    i.biClrImportant = 0;
    fwrite(&h, sizeof(bmp_header), 1, f);
    fwrite(&i, sizeof(bmp_info_header), 1, f);
    fwrite(b, s, 1, f);
    fclose(f);
}

template<class T>
void write_mat_bmp(const path &filename, const mat<T> &m)
{
    write_mat_bmp(filename, m.getWidth(), m.getHeight(), sizeof(T) * CHAR_BIT, (const uint8_t *)&m(0, 0), m.size() * sizeof(T));
}

template<class T>
void write_mat_tga(const path &filename, const mat<T> &m)
{
    auto f = primitives::filesystem::fopen(filename, "wb");
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
