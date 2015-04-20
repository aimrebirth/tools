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

#include <iostream>
#include <stdint.h>

using namespace std;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        cout << "Usage:\n" << argv[0] << " script.scr";
        return 1;
    }
    string path = argv[1];
    FILE *f = fopen(path.c_str(), "rb");
    if (!f)
    {
        cout << "Cannot open the file: " << argv[1];
        return 2;
    }
    uint32_t size = 0;
    uint32_t dummy = 0;
    fread(&size, sizeof(uint32_t), 1, f);
    fread(&size, sizeof(uint32_t), 1, f);
    fread(&size, sizeof(uint32_t), 1, f);
    fread(&dummy, sizeof(uint32_t), 1, f);
    string buf(size, 0);
    fread(&buf[0], size, 1, f);
    fclose(f);
    path += ".txt";
    f = fopen(path.c_str(), "w");
    if (!f)
    {
        cout << "Cannot open the file: " << argv[1];
        return 3;
    }
    const char *ptr = &buf[0];
    while (ptr < &buf[0] + size)
    {
        fprintf(f, "%s\n", ptr);
        ptr += strlen(ptr) + 1;
    }
    fclose(f);
    return 0;
}