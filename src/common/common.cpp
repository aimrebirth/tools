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

#include "common.h"

#include <primitives/exceptions.h>

#ifdef _WIN32
#include <Windows.h>
#endif

std::string str2utf8(const std::string &codepage_str, int cp)
{
#ifdef _WIN32
    auto utf16_str = str2utf16(codepage_str, cp);
    int utf8_size = WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(),
        utf16_str.length(), nullptr, 0,
        nullptr, nullptr);
    std::string utf8_str(utf8_size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, utf16_str.c_str(),
        utf16_str.length(), &utf8_str[0], utf8_size,
        nullptr, nullptr);
    return utf8_str;
#else
    SW_UNIMPLEMENTED;
#endif
}

std::wstring str2utf16(const std::string &codepage_str, int cp)
{
#ifdef _WIN32
    int size;
    std::wstring utf16_str;

    size = MultiByteToWideChar(cp, MB_PRECOMPOSED, codepage_str.c_str(),
        codepage_str.length(), nullptr, 0);
    utf16_str = std::wstring(size, '\0');
    MultiByteToWideChar(cp, MB_PRECOMPOSED, codepage_str.c_str(),
        codepage_str.length(), &utf16_str[0], size);

    return utf16_str;
#else
    SW_UNIMPLEMENTED;
#endif
}

std::string str2str(const std::string &codepage_str, int cp_from, int cp_to) {
#ifdef _WIN32
    auto utf16_str = str2utf16(codepage_str, cp_from);
    int dest_size = WideCharToMultiByte(cp_to, 0, utf16_str.c_str(), utf16_str.length(), nullptr, 0, nullptr, nullptr);
    std::string dest_str(dest_size, '\0');
    WideCharToMultiByte(cp_to, 0, utf16_str.c_str(), utf16_str.length(), &dest_str[0], dest_size, nullptr, nullptr);
    return dest_str;
#else
    SW_UNIMPLEMENTED;
#endif
}
