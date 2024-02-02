/*
* AIM script2txt
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

#include <buffer.h>
#include <common.h>

#include <sstream>

template <typename T>
inline bool replace_all(T &str, const T &from, const T &to)
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

struct script
{
    uint32_t file_size;
    uint32_t unk0; // stack size? always 16000? section bits? magic? max size?
    uint32_t raw_text_size;
    uint32_t nlines;
    std::vector<uint8_t> raw_text;
    uint32_t line_start_pos_len; // always 800
    std::vector<uint32_t> line_start_pos;

    //
    std::vector<std::string> lines;

    void load(const buffer &b)
    {
        READ(b, file_size);
        READ(b, unk0);
        READ(b, raw_text_size);
        READ(b, nlines);
        raw_text.resize(raw_text_size);
        if (raw_text_size)
            READ_N(b, raw_text[0], raw_text.size());
        READ(b, line_start_pos_len);
        line_start_pos.resize(line_start_pos_len);
        READ_N(b, line_start_pos[0], line_start_pos.size());

        if (!b.eof())
        {
            std::stringstream ss;
            ss << std::hex << b.index() << " != " << std::hex << b.size();
            throw std::logic_error(ss.str());
        }

        fix_text();
    }

    void fix_text()
    {
        std::string line;
        for (size_t i = 0; i < raw_text.size(); i++)
        {
            if (raw_text[i] == 0)
            {
                lines.push_back(line);
                line.clear();
            }
            else
                line.push_back(raw_text[i]);
        }
    }

    std::string get_text() const
    {
        std::string s;
        for (auto &line : lines)
        {
            if (line != "\n")
                s += line + "\n";
        }

        replace_all(s, "PROC", "PROC ");
        replace_all(s, "ENFD", "END");
        replace_all(s, "\nEN\n", "\n");
        replace_all(s, "?", " ");
        s += "\nEND\n";

        // remove wrong braces
        int braces = 0;
        for (auto &c : s)
        {
            switch (c)
            {
            case '{':
                braces++;
                break;
            case '}':
                if (braces == 0)
                    c = ' ';
                else
                    braces--;
                break;
            }
        }
        return s;
    }
};
