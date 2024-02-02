/*
 * AIM 1 unpaker
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

#include <filesystem>
#include <iostream>

#include "pak.h"

namespace fs = std::filesystem;

void unpak(string fn)
{
    FILE *f = fopen(fn.c_str(), "rb");
    if (!f)
        return;
    pak p;
    p.load(f);

    auto unpack = [&](auto &file)
    {
        cout << "Unpacking " << file.name << "\n";
        vector<char> buf(file.len);
        file.read(&p, &buf[0], file.len);
        file.write(fn + ".dir", buf);
    };

    for (auto &[n,f] : p.files)
        unpack(f);
    fclose(f);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cerr << "Usage: " << argv[0] << " <archive.pak or dir>" << "\n";
        return 1;
    }
    fs::path p = argv[1];
    if (fs::is_regular_file(p)) {
        unpak(p.string());
    } else if (fs::is_directory(p)) {
        for (auto &&d : fs::directory_iterator{p}) {
            if (d.path().extension() == ".pak") {
                std::cout << "processing: " << d.path() << "\n";
                try {
                    unpak(d.path().string());
                } catch (std::exception &e) {
                    std::cerr << e.what() << "\n";
                }
            }
        }
    } else {
        throw std::runtime_error("Bad fs object");
    }
    return 0;
}
