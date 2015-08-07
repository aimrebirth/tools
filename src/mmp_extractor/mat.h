#pragma once

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
    int getPos(const T *const elem) const { return elem - &data[0]; }
    const std::vector<T> &getData() const { return data; }
    std::vector<T> &getData() { return data; }

private:
    std::vector<T> data;
    int width;
    int height;
};
