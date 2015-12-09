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

#include <sstream>

#include <buffer.h>
#include <common.h>

struct script
{
    uint32_t file_size;
    uint32_t unk0;
    uint32_t raw_text_size;
    uint32_t unk1;
    std::vector<uint8_t> raw_text;
    uint32_t array_len;
    std::vector<uint32_t> unk2;

    //
    std::vector<std::string> lines;

    void load(buffer &b)
    {
        READ(b, file_size);
        READ(b, unk0);
        READ(b, raw_text_size);
        READ(b, unk1);
        raw_text.resize(raw_text_size);
        READ_N(b, raw_text[0], raw_text.size());
        READ(b, array_len);
        unk2.resize(array_len);
        READ_N(b, unk2[0], unk2.size());

        if (!b.eof())
        {
            std::stringstream ss;
            ss << std::hex << b.index() << " != " << std::hex << b.size();
            throw std::logic_error(ss.str());
        }
        
        fix_text();
        beautify();
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

    void beautify()
    {
        const std::string space = "    ";
        int brace_count = 0;
        int proc_started = 0;
        for (auto &line : lines)
        {
            if (brace_count > 0 || proc_started > 0)
            {
                auto space_count = brace_count + proc_started;
                if (line == "}" ||
                    (brace_count == 0 && proc_started == 1 &&
                        line.find("END") != line.npos))
                    space_count--;
                std::string s;
                for (int i = 0; i < space_count; i++)
                    s += space;
                line = s + line;
            }
            if (line.find("PROC") != line.npos &&
                line.find("()") != line.npos)
            {
                proc_started++;
                continue;
            }
            if (line.find("END") != line.npos && proc_started == 1)
            {
                proc_started--;
            }
            for (auto &c : line)
            {
                if (c == '{')
                    brace_count++;
                else if (c == '}')
                    brace_count--;
                if (brace_count < 0)
                {
                    c = '\n';
                    brace_count++;
                }
            }
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
        replace_all(s, "IF(", "IF (");
        replace_all(s, "\nIF", "\n\nIF");
        replace_all(s, "PROC", "PROC ");
        replace_all(s, "END\nPROC", "END\n\nPROC");
        return s;
    }
};
