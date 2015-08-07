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

#include "mmp.h"

#include <algorithm>
#include <fstream>
#include <iomanip>

#include "bmp.h"

template<class T>
void write_mat_bmp(const std::string &filename, const mat<T> &m)
{
    FILE *f = fopen(filename.c_str(), "wb");
    if (f == nullptr)
        return;
    BITMAPFILEHEADER h = { 0 };
    h.bfType = 0x4D42;
    h.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO) + m.size() * sizeof(T);
    h.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);
    BITMAPINFO i = { 0 };
    i.bmiHeader.biSize = sizeof(i.bmiHeader);
    i.bmiHeader.biWidth = m.getWidth();
    i.bmiHeader.biHeight = m.getHeight();
    i.bmiHeader.biPlanes = 1;
    i.bmiHeader.biBitCount = 32;
    i.bmiHeader.biCompression = 0;
    i.bmiHeader.biSizeImage = 0;
    i.bmiHeader.biXPelsPerMeter = 0;
    i.bmiHeader.biYPelsPerMeter = 0;
    i.bmiHeader.biClrUsed = 0;
    i.bmiHeader.biClrImportant = 0;
    fwrite(&h, sizeof(BITMAPFILEHEADER), 1, f);
    fwrite(&i, sizeof(BITMAPINFO), 1, f);
    fwrite(&m(0, 0), m.size() * sizeof(T), 1, f);
    fclose(f);
}

void write_mat_tga(const std::string &filename, const mat<uint8_t> &m)
{
    // http://paulbourke.net/dataformats/tga/
    buffer dst;
    dst.write(uint8_t(0xE)); // idlength (comment length)
    dst.write(uint8_t(0)); // colourmaptype
    dst.write(uint8_t(3)); // datatypecode
    dst.write(uint16_t(0)); // colourmaporigin
    dst.write(uint16_t(0)); // colourmaplength
    dst.write(uint8_t(0)); // colourmapdepth
    dst.write(uint16_t(0)); // x_origin
    dst.write(uint16_t(0)); // y_origin
    dst.write(uint16_t(m.getWidth())); // width
    dst.write(uint16_t(m.getHeight())); // height
    dst.write(uint8_t(8)); // bitsperpixel
    dst.write(uint8_t(0x28)); // imagedescriptor
    
    const char *label = "AIMMPExtractor";
    dst.write(label, strlen(label));

    dst.write(m.getData().data(), m.getWidth() * m.getHeight());

    writeFile(filename, dst.buf());
}

void water_segment::load(buffer &b)
{
    while (!b.eof())
    {
        water w;
        w.load(b);
        segments.push_back(w);
    }
}

void water::load(buffer &b)
{
    READ(b, unk0);
    READ(b, name1);
    READ(b, unk1);
    READ(b, unk2);
    READ(b, unk3);
    READ(b, unk4);
    READ(b, name2);
    READ(b, unk5);
}

void weather_segment::load(buffer &b)
{
    READ(b, n_segs);
    segments.resize(n_segs);
    READ(b, name);
    for (auto &s : segments)
        s.load(b);
}

void weather::load(buffer &b)
{
    READ(b, name);
    READ(b, unk0);
    READ(b, unk1);
    READ(b, smoke_1);
    READ(b, smoke_3);    
    READ(b, smokeType);
    READ(b, unk2);
    READ(b, cloud_layer1);
    READ(b, cloud_layer2);
    READ(b, cloud_layer1_speed);
    READ(b, cloud_layer2_speed);
    READ(b, cloud_layer1_direction);
    READ(b, cloud_layer2_direction);
    READ(b, sun);
    READ(b, general_color);
    READ(b, sun_color);
    READ(b, moon_color);
    READ(b, moon);
    READ(b, probability);
    READ(b, day_night_gradient_name);
    READ(b, dawn_dusk_gradient_name);
    READ(b, dawn_dusk_color);
    READ(b, effects);
    READ(b, smoke_2);
    READ(b, smoke_4);
    READ(b, slider_3);
    READ(b, slider_1);
    READ(b, unk8);
}

header_segment *header::create_segment(buffer &b)
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

void header::load(buffer &b)
{
    READ(b, unk0);
    READ(b, name1);
    READ(b, name2);
    READ(b, width);
    READ(b, height);
    READ(b, n_header_segs);
    segments.resize(n_header_segs);
    READ(b, name);
    for (auto &s : segments)
    {
        s = create_segment(b);
        buffer b2(b, s->len);
        if (!b2.eof())
            s->load(b2);
    }
}

void segment::load(buffer &b)
{
    READ(b, desc);
    buffer b2(b);
    b2.seek(desc.offset);
    READ(b2, d);
}

void mmp::load(buffer &b)
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
    buffer b(readFile(filename));
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
    int k = segment::len;

    for (auto &s : segments)
    {
        for (auto &i : s.d.Infomap)
            textures[i.getTexture()]++;
    }
    textures.erase(0);
    auto textures_per_color = std::max(1U, textures.size() / 3);
    auto color_step = 200 / textures.size();
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
        alpha_maps[t.second.g] = mat<uint32_t>(h.width, h.height);

    // merge
    heightmap = decltype(heightmap)(h.width, h.height);
    texmap = decltype(texmap)(h.width, h.height);
    texmap_colored = decltype(texmap_colored)(h.width, h.height);
    colormap = decltype(colormap)(h.width, h.height);
    for (int y = 0; y < h.height; y++)
    {
        auto ys = y / k * xsegs;
        auto yc = y % k * k;
        for (int x = 0; x < h.width; x++)
        {
            auto xs = x / k;
            auto xc = x % k;
            auto y_rev = h.height - y - 1;
            const auto &data = segments[ys + xs].d;
            auto height = data.Heightmap[yc + xc];
            auto t = data.Infomap[yc + xc].getTexture();
            auto t_norm = textures_map[t];

            texmap(y_rev, x) = t_norm;
            alpha_maps[t_norm.g](y_rev, x) = color{ 0,255,0,0 };
            texmap_colored(y_rev, x) = textures_map_colored[t];
            colormap(y_rev, x) = data.Colormap[yc + xc];

            if (x == 0 && y == 0)
            {
                h_min = height;
                h_max = height;
            }
            else
            {
                h_min = std::min(h_min, height);
                h_max = std::max(h_max, height);
            }
        }
    }

    alpha_maps.erase(0);
    scale16 = 0xffff / (h_max - h_min);

    for (int y = 0; y < h.height; y++)
    {
        auto ys = y / k * xsegs;
        auto yc = y % k * k;
        for (int x = 0; x < h.width; x++)
        {
            auto xs = x / k;
            auto xc = x % k;
            auto height = segments[ys + xs].d.Heightmap[yc + xc];
            auto val = (height - h_min) * scale16;
            heightmap(y, x) = val;
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

