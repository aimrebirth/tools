/*
 * AIM mmp_extractor
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

#define NOMINMAX

#include "mmp.h"

#include <algorithm>
#include <fstream>
#include <iomanip>

#include <primitives/filesystem.h>

void water_segment::load(const buffer &b)
{
    wg.load(b);
}

void weather_segment::load(const buffer &b)
{
    wg.load(b);
}

header_segment *header::create_segment(const buffer &b)
{
    HeaderSegmentType type;
    READ(b, type);

    header_segment *segment = 0;
    switch (type)
    {
    case HeaderSegmentType::water:
        segment = new water_segment;
        break;
    case HeaderSegmentType::weather:
        segment = new weather_segment;
        break;
    default:
        throw std::logic_error("unknown header segment type " + std::to_string((int)type));
        break;
    }
    if (segment)
    {
        segment->type = type;
        READ(b, segment->unk0);
        READ(b, segment->len);
    }
    return segment;
}

void header::load(const buffer &b)
{
    READ(b, unk0);
    READ_WSTRING(b, name1);
    READ_WSTRING(b, name2);
    READ(b, width);
    READ(b, height);
    READ(b, n_header_segs);
    segments.resize(n_header_segs);
    READ_STRING_N(b, name, 0xA0);
    for (auto &s : segments)
    {
        s = create_segment(b);
        buffer b2(b, s->len);
        if (!b2.eof())
            s->load(b2);
    }
}

void segment::load(const buffer &b)
{
    READ(b, desc);
    buffer b2(b);
    b2.seek(desc.offset);
    READ(b2, d);
}

void mmp::load(const buffer &b)
{
    h.load(b);
    xsegs = (h.width - 1) / 64;
    if ((h.width - 1) % 64 != 0)
        xsegs++;
    ysegs = (h.height - 1) / 64;
    if ((h.height - 1) % 64 != 0)
        ysegs++;
    int n_segs = xsegs * ysegs;
    segments.resize(n_segs);
    for (auto &s : segments)
        s.load(b);

    // check whether all segments were read
    if (segments.size())
    {
        auto len = segments[0].desc.offset + segments.size() * sizeof(segment::data);
        if (len != b.size())
            throw std::logic_error("Some segments were not read");
    }
}

void mmp::load(const std::string &fn)
{
    filename = fn;
    buffer b(read_file(filename));
    load(b);
}

void mmp::loadTextureNames(const std::string &fn)
{
    std::ifstream ifile(fn);
    while (ifile)
    {
        int id;
        std::string name;
        ifile >> id >> name;
        textures_names[id] = name;
    }
}

void mmp::process()
{
    for (auto &s : segments)
    {
        for (auto &i : s.d.Infomap)
            textures[i.getTexture()]++;
    }
    textures.erase(0);
    auto textures_per_color = std::max(1U, textures.size() / 3);
    auto color_step = 200 / std::max(1U, textures.size());
    for (int i = 0; i < textures.size(); i++)
    {
        int color_id = i / textures_per_color;
        color c = { 0 };
        if (color_id == 0)
            c.r = 255 - 200 / textures_per_color * (i % textures_per_color);
        else if (color_id == 1)
            c.g = 255 - 200 / textures_per_color * (i % textures_per_color);
        else if (color_id == 2)
            c.b = 255 - 200 / textures_per_color * (i % textures_per_color);
        auto iter = textures.begin();
        std::advance(iter, i);
        textures_map_colored[iter->first] = c;
        color c2 = { 0 };
        c2.g = 255 - i * color_step;
        textures_map[iter->first] = c2;
    }
    alpha_maps[0] = mat<uint32_t>(h.width, h.height);
    for (auto &t : textures_map)
    {
        alpha_maps[t.second.g] = mat<uint32_t>(h.width, h.height);
    }

    // merge
    heightmap = decltype(heightmap)(h.width, h.height);
    texmap = decltype(texmap)(h.width, h.height);
    texmap_colored = decltype(texmap_colored)(h.width, h.height);
    colormap = decltype(colormap)(h.width, h.height);

    h_min = std::numeric_limits<Height>::max();
    h_max = std::numeric_limits<Height>::min();
    for (auto &s : segments)
    {
        const auto &data = s.d;
        int y1 = s.desc.Ymin / 10;
        int y2 = s.desc.Ymax / 10;
        if (y2 > h.height)
            y2 = h.height;
        for (int y = 0; y1 < y2; y1++, y++)
        {
            int x1 = s.desc.Xmin / 10;
            int x2 = s.desc.Xmax / 10;
            auto dx = x2 - x1;
            if (x2 > h.width)
                x2 = h.width;
            for (int x = 0; x1 < x2; x1++, x++)
            {
                auto p = y * dx + x;
                auto y_rev = h.height - y1 - 1; // for bmp reversion
                //auto y_rev = y1;

                auto t = data.Infomap[p].getTexture();
                auto t_norm = textures_map.find(t);
                if (t_norm != textures_map.end())
                {
                    texmap(y_rev, x1) = t_norm->second;
                    alpha_maps[t_norm->second.g](y_rev, x1) = color{ 0,255,0,0 };
                }

                texmap_colored(y_rev, x1) = textures_map_colored[t];
                colormap(y_rev, x1) = data.Colormap[p];

                auto height = data.Heightmap[p];
                h_min = std::min(h_min, height);
                h_max = std::max(h_max, height);
            }
        }
    }

    alpha_maps.erase(0);
    scale16 = 0xffff / (h_max - h_min);
    const int unreal_koef = 51200;
    const int aim_koef = 10;
    const double diff = h_max - h_min;
    scale = aim_koef / (unreal_koef / diff);

    for (auto &s : segments)
    {
        int y1 = s.desc.Ymin / 10;
        int y2 = s.desc.Ymax / 10;
        if (y2 > h.height)
            y2 = h.height;
        for (int y = 0; y1 < y2; y1++, y++)
        {
            int x1 = s.desc.Xmin / 10;
            int x2 = s.desc.Xmax / 10;
            auto dx = x2 - x1;
            if (x2 > h.width)
                x2 = h.width;
            for (int x = 0; x1 < x2; x1++, x++)
            {
                auto height = s.d.Heightmap[y * dx + x];
                auto val = (height - h_min) * scale16 * scale;
                auto &old_height = heightmap(y1, x1);
                old_height = val;
            }
        }
    }
}

void mmp::writeFileInfo()
{
    std::ofstream ofile(filename + ".info.txt");
    if (!ofile)
        return;
    ofile << "width: " << h.width << "\n";
    ofile << "height: " << h.height << "\n";
    ofile << "x segments: " << xsegs << "\n";
    ofile << "y segments: " << ysegs << "\n";
    ofile << "h_min: " << h_min << "\n";
    ofile << "h_max: " << h_max << "\n";
    ofile << "h_diff: " << h_max - h_min << "\n";
    ofile << "scale16: " << scale16 << "\n";
    ofile << "scale: " << scale * 100 << "\n";
}

void mmp::writeTexturesList()
{
    std::ofstream ofile(filename + ".textures.txt");
    if (!ofile)
        return;
    for (auto &t : textures)
    {
        ofile << t.first;
        if (!textures_names.empty())
        {
            int c = textures_map[t.first].g;
            ofile << "\tcount: " << t.second;
            ofile << "\thex: 0x00" << std::hex << c << "0000" << std::dec;
            ofile << "\tg: " << c;
            ofile << "\tg: " << std::setprecision(3) << std::setw(3) << c / 256.f;
            ofile << "\t" << textures_names[t.first];
        }
        ofile << "\n";
    }
}

void mmp::writeHeightMap()
{
    auto fn = filename + ".heightmap16.raw";
    FILE *f = fopen(fn.c_str(), "wb");
    if (f == nullptr)
        return;
    fwrite(&heightmap(0, 0), heightmap.size() * sizeof(decltype(heightmap)::type), 1, f);
    fclose(f);
}

void mmp::writeTextureMap()
{
    auto fn = filename + ".texmap.bmp";
    write_mat_bmp(fn, texmap);
}

void mmp::writeTextureAlphaMaps()
{
    for (auto &t : alpha_maps)
    {
        auto fn = filename + ".texmap." + std::to_string(t.first) + ".bmp";
        write_mat_bmp(fn, t.second);
    }
}

void mmp::writeTextureMapColored()
{
    auto fn = filename + ".texmap.colored.bmp";
    write_mat_bmp(fn, texmap_colored);
}

void mmp::writeColorMap()
{
    auto fn = filename + ".colormap.bmp";
    write_mat_bmp(fn, colormap);
}
