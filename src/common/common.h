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

#define READ(b, var) b.read(&var, sizeof(var))
#define READ_NOTHROW(b, var) b.read(&var, sizeof(var), true)
#define READ_N(b, var, sz) b.read(&var, sz)

std::string version();
std::vector<uint8_t> readFile(const std::string &fn);

class buffer
{
public:
    buffer();
    buffer(const std::vector<uint8_t> &buf, uint32_t data_offset = 0);
    buffer(buffer &rhs, uint32_t size);
    buffer(buffer &rhs, uint32_t size, uint32_t offset);

    uint32_t read(void *dst, uint32_t size, bool nothrow = false);
    void skip(int n);
    bool eof() const;
    bool check(int index) const;

    uint32_t getIndex() const;
    uint32_t getSize() const;

private:
    std::shared_ptr<const std::vector<uint8_t>> buf;
    uint32_t index = 0;
    uint8_t *ptr = 0;
    uint32_t data_offset = 0;
    uint32_t size = 0;
};
