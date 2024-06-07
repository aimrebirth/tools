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

#include <iostream>
#include <map>
#include <string>

// MultiByteToWideChar: https://msdn.microsoft.com/en-us/library/windows/desktop/dd319072(v=vs.85).aspx
// code pages: https://msdn.microsoft.com/en-us/library/windows/desktop/dd317756(v=vs.85).aspx
// https://www.ibm.com/docs/en/rational-soft-arch/9.6.1?topic=overview-locales-code-pages-supported
static const std::map<std::string, int> code_pages
{
    { "en", 0 },
    { "cs", 1250 },
    { "ru", 1251 },
    { "de", 1252 },
    { "fr", 1252 },
    { "et", 1257 },
};

std::string str2utf8(const std::string &codepage_str, int cp);
std::wstring str2utf16(const std::string &codepage_str, int cp);

std::string str2str(const std::string &codepage_str, int cp_from, int cp_to);

struct progress_bar {
    const size_t max_elements;
    const int displaylen;
    int displaycur{};
    int i{};

    void step() {
        auto progress_bar_pos = std::round((double)++i / max_elements * displaylen);
        for (int i = displaycur; i < progress_bar_pos; ++i) {
            std::cout << "#";
        }
        displaycur = progress_bar_pos;
    }
};
