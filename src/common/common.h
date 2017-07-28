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

#include <primitives/filesystem.h>

template <typename T>
bool replace_all(T &str, const T &from, const T &to)
{
    bool replaced = false;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != T::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
        replaced = true;
    }
    return replaced;
}

inline bool replace_all(std::string &str, const std::string &from, const std::string &to)
{
    return replace_all<std::string>(str, from, to);
}
