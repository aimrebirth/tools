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

#include <memory>
#include <stdint.h>
#include <string>
#include <vector>

#define READ(b, var) b.read(var)
#define READ_N(b, var, sz) b.read(var, sz)

#define READ_STRING(b, var) var = b.read_string()
#define READ_STRING_N(b, var, sz) var = b.read_string(sz)

#define READ_WSTRING(b, var) var = b.read_wstring()
#define READ_WSTRING_N(b, var, sz) var = b.read_wstring(sz)

#define READ_PASCAL_STRING(b, var) var = b.read_pascal_string()

#define WRITE(b, var) b.write(&var)

std::string version();
void writeFile(const std::string &fn, const std::vector<uint8_t> &data);

class buffer
{
public:
    buffer();
    buffer(size_t size);
    buffer(const std::string &s);
    buffer(const std::vector<uint8_t> &buf, uint32_t data_offset = 0);
    buffer(const buffer &rhs, uint32_t size);
    buffer(const buffer &rhs, uint32_t size, uint32_t offset);

    template <typename T>
    uint32_t read(T &dst, uint32_t size = 1) const
    {
        return _read((void *)&dst, size * sizeof(T), 0);
    }
    std::string read_string(uint32_t blocksize = 0x20) const;
    std::wstring read_wstring(uint32_t blocksize = 0x20) const;
    template <typename T>
    uint32_t readfrom(T *dst, uint32_t size, uint32_t offset) const
    {
        return _read(dst, size * sizeof(T), offset);
    }

    template <typename T>
    uint32_t write(const T &src)
    {
        return _write(&src, sizeof(T));
    }
    template <typename T>
    uint32_t write(const T *src, uint32_t size)
    {
        return _write(src, size * sizeof(T));
    }
    template <typename T>
    uint32_t write(const T *src)
    {
        return _write(src, sizeof(T));
    }
    uint32_t write(const std::string &s)
    {
        auto sz = s.size() + 1;
        _write(s.c_str(), sz);
        skip(0x20 - sz);
        return 0x20;
    }

    template <class T, class SizeType = uint32_t>
    void read_vector(std::vector<T> &v, int n) const
    {
        v.clear();
        v.reserve(n);
        for (int i = 0; i < n; i++)
        {
            T t;
            t.load(*this);
            v.push_back(t);
        }
    }

    template <class T, class SizeType = uint32_t>
    void read_vector(std::vector<T> &v) const
    {
        SizeType n = 0;
        read(n);
        read_vector<T, SizeType>(v, n);
    }

    std::string read_pascal_string() const
    {
        uint32_t n = 0;
        read(n);
        std::string s(n, 0);
        _read(s.data(), n);
        return s;
    }

    void seek(uint32_t size) const; // setpos
    void skip(int n) const;
    uint32_t end() const { return end_; }
    bool eof() const;
    bool check_pos(int index) const;
    void reset() const;

    uint32_t index() const;
    uint32_t size() const;
    const std::vector<uint8_t> &buf() const;

    const uint8_t *getPtr() const { return ptr; }

    uint32_t _read(void *dst, uint32_t size, uint32_t offset = 0) const;
    uint32_t _write(const void *src, uint32_t size);

private:
    std::shared_ptr<std::vector<uint8_t>> buf_;
    mutable uint32_t index_ = 0;
    mutable uint8_t *ptr = 0;
    mutable uint32_t data_offset = 0;
    mutable uint32_t size_ = 0;
    uint32_t end_;
};
