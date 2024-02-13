/*
 * AIM obj_extractor
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

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <stdio.h>
#include <string>

#define _USE_MATH_DEFINES
#include <math.h>

#include <objects.h>
#include "other.h"

#include <Polygon4/DataManager/Database.h>
#include <Polygon4/DataManager/Storage.h>
#include <Polygon4/DataManager/Types.h>
#include <primitives/filesystem.h>
#include <primitives/sw/main.h>
#include <primitives/sw/settings.h>
#include <primitives/sw/cl.h>

#include <buffer.h>
#include <types.h>

using namespace polygon4;
using namespace polygon4::detail;

#define RAD2GRAD(x) (x) = (x) / M_PI * 180.0
#define ASSIGN(x, d) isnan(x) ? d : x

std::string prefix;
int inserted_all = 0;

struct mmo_storage
{
    path name;
    Objects objects;
    MechGroups mechGroups;
    MapGoods mapGoods; // trading & production system
    MapMusic mapMusic;
    MapSounds mapSounds;
    // aim2
    std::vector<Organization> organizations;
    std::vector<OrganizationBase> organizationBases;
    Prices prices;

    uint32_t music_and_sounds_section_size = 0;

    void load(const buffer &b)
    {
        objects.load(b);
        mechGroups.load(b);
        if (b.eof()) // custom maps
            return;
        mapGoods.load(b);
        READ(b, music_and_sounds_section_size);
        mapMusic.load(b);
        mapSounds.load(b);
        if (gameType == GameType::Aim2)
        {
            uint32_t len = 0;
            READ(b, len);

            b.read_vector(organizations);
            b.read_vector(organizationBases);

            prices.load(b);
        }

        if (!b.eof())
        {
            std::stringstream ss;
            ss << std::hex << b.index() << " != " << std::hex << b.size();
            throw std::logic_error(ss.str());
        }
    }
};

mmo_storage read_mmo(const path &fn)
{
    buffer f(read_file(fn));
    mmo_storage s;
    s.name = fn;
    s.load(f);
    return s;
}

void write_mmo(Storage *storage, const mmo_storage &s, const std::string &mapname1)
{
    std::string map_name = mapname1.empty() ? to_printable_string(s.name.filename().stem()) : mapname1;
    if (!prefix.empty())
        map_name = prefix + "." + map_name;
    std::transform(map_name.begin(), map_name.end(), map_name.begin(), ::tolower);

    int map_id = 0;
    for (auto &m : storage->maps)
    {
        if (m.second->text_id == map_name)
        {
            map_id = m.first;
            break;
        }
    }

    if (map_id == 0)
    {
        auto m = storage->addMap();
        m->text_id = map_name;
        map_id = m->getId();
        //throw SW_RUNTIME_ERROR("error: map '" + map_name + "' is not found in the database");
    }

    auto this_map = storage->maps[map_id];

    auto calc_yaw = [](auto &v)
    {
        auto yaw = atan2(-v[0].y, -v[0].x);
        yaw = RAD2GRAD(yaw);
        return yaw;
    };

    int inserted = 0;
    int exist = 0;
    for (auto &seg : s.objects.segments)
    {
        auto setup_object = [&this_map, &calc_yaw](auto &mb, auto &object)
        {
            // protect against nans
            object.m_rotate_z[2].z = ASSIGN(object.m_rotate_z[2].z, 1);

            mb.text_id = object.name2;
            mb.map = this_map;
            mb.x = ASSIGN(object.position.x, 0);
            mb.y = ASSIGN(object.position.y, 0);
            mb.z = ASSIGN(object.position.z, 0);
            mb.roll = 0;
            mb.pitch = 0;
            mb.yaw = calc_yaw(object.m_rotate_z);
            mb.scale = ASSIGN(object.m_rotate_z[2].z, 1);
        };

        auto process_buildings = [&storage, &map_id, &inserted, &exist, &setup_object](auto *segment)
        {
            std::set<std::string> objs;
            std::map<std::string, int> bld_ids;
            for (auto &object : segment->objects)
                objs.insert(object.name1);
            for (auto &o : objs)
            {
                auto iter = std::find_if(storage->buildings.begin(), storage->buildings.end(), [&](const auto &p)
                {
                    return p.second->text_id == o;
                });
                if (iter == storage->buildings.end())
                {
                    auto bld = storage->addBuilding();
                    bld->text_id = o;
                    bld_ids[o] = bld->getId();
                }
                else
                {
                    bld_ids[o] = iter->second->getId();
                }
            }
            for (auto &object : segment->objects)
            {
                MapBuilding mb;
                mb.building = storage->buildings[bld_ids[object.name1]];
                setup_object(mb, object);
                auto i = find_if(storage->mapBuildings.begin(), storage->mapBuildings.end(), [&](const auto &p)
                {
                    return *p.second == mb;
                });
                if (i == storage->mapBuildings.end())
                {
                    auto mb2 = storage->addMapBuilding(storage->maps[map_id]);
                    mb.setId(mb2->getId());
                    *mb2 = mb;
                    inserted++;
                }
                else
                {
                    exist++;
                }
            }
        };

        switch (seg->segment_type)
        {
        case ObjectType::BUILDING:
            process_buildings(&dynamic_cast<SegmentObjects<::Building>&>(*seg));
            break;
        case ObjectType::TOWER:
            process_buildings(&dynamic_cast<SegmentObjects<::Tower>&>(*seg));
            break;
        }

        auto process_objects = [&storage, &calc_yaw, &map_id, &inserted, &exist, &setup_object](auto *segment)
        {
            std::set<std::string> objs;
            std::map<std::string, int> bld_ids;
            for (auto &object : segment->objects)
                objs.insert(object.name1);
            for (auto &o : objs)
            {
                auto iter = find_if(storage->objects.begin(), storage->objects.end(), [&](const auto &p)
                {
                    return p.second->text_id == o;
                });
                if (iter == storage->objects.end())
                {
                    auto bld = storage->addObject();
                    bld->text_id = o;
                    bld_ids[o] = bld->getId();
                }
                else
                    bld_ids[o] = iter->second->getId();
            }
            for (auto &object : segment->objects)
            {
                polygon4::detail::MapObject mb;
                mb.object = storage->objects[bld_ids[object.name1]];
                setup_object(mb, object);
                auto i = find_if(storage->mapObjects.begin(), storage->mapObjects.end(), [&](const auto &p)
                {
                    return *p.second == mb;
                });
                if (i == storage->mapObjects.end())
                {
                    auto mb2 = storage->addMapObject(storage->maps[map_id]);
                    mb.setId(mb2->getId());
                    *mb2 = mb;
                    inserted++;
                }
                else
                {
                    exist++;
                }
            }
        };

        switch (seg->segment_type)
        {
        case ObjectType::TREE:
            process_objects(&dynamic_cast<SegmentObjects<::Tree>&>(*seg));
            break;
        case ObjectType::STONE:
            process_objects(&dynamic_cast<SegmentObjects<::Stone>&>(*seg));
            break;
        case ObjectType::LAMP:
            process_objects(&dynamic_cast<SegmentObjects<::Lamp>&>(*seg));
            break;
        case ObjectType::BOUNDARY:
            process_objects(&dynamic_cast<SegmentObjects<::Boundary>&>(*seg));
            break;
        }
    }
    inserted_all += inserted;
    std::cout << "inserted: " << inserted << ", exist: " << exist << "\n";
}

int main(int argc, char *argv[])
{
    cl::opt<bool> m2("m2", cl::desc("m2 .mmo file"));
    cl::opt<bool> print_mechanoids("print_mechanoids", cl::desc("print mechanoids"));
    cl::opt<path> db_path("db", cl::desc("database file"));
    cl::opt<std::string> mapname("map", cl::desc("map name"));
    cl::alias db_pathA("d", cl::aliasopt(db_path));
    cl::opt<path> p(cl::Positional, cl::desc("<.mmo file or directory with .mmo files>"), cl::value_desc("file or directory"), cl::Required);

    cl::ParseCommandLineOptions(argc, argv);

    gameType = m2 ? GameType::Aim2 : GameType::Aim1;

    auto action = [&p](auto f)
    {
        if (fs::is_regular_file(p))
            f(p, read_mmo(p));
        else if (fs::is_directory(p))
        {
            auto files = enumerate_files_like(p, ".*\\.[Mm][Mm][Oo]", false);
            for (auto &file : files)
            {
                std::cerr << "processing: " << file << "\n";
                f(file, read_mmo(file));
            }
        }
        else
            throw std::runtime_error("Bad fs object");
    };

    if (print_mechanoids)
    {
        action([](const path &file, const mmo_storage &m)
        {
            for (auto &mg : m.mechGroups.mechGroups)
            {
                std::cout << mg.name;
                std::cout << " " << mg.org;
                std::cout << " " << mg.mechanoids.size();
                std::cout << " " << to_printable_string(file.filename().stem());
                std::cout << "\n";
            }
        });
    }
    else if (db_path.empty())
    {
        action([](const path &, const auto &m) {});
    }
    else
    {
        bool e = fs::exists(db_path);
        auto storage = initStorage();
        auto database = std::make_unique<polygon4::Database>(db_path);
        if (!e)
        {
            storage->create(*database);
            storage->save(*database, {});
        }
        storage->load(*database, {});
        action([&storage, &mapname](const path &, const auto &m) {write_mmo(storage.get(), m, mapname); });
        if (inserted_all) {
            storage->save(*database, {});
            database->save();
        }
    }

    return 0;
}
