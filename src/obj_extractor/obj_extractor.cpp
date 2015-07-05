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

#include "objects.h"
#include "other.h"

#include <Polygon4/Storage.h>

#include <common.h>

using namespace std;

std::string prefix;
GameType gameType;

struct storage
{
    string name;
    Objects objects;
    MechGroups mgs;
    MapGoods mg;
    MapMusic mm;
    MapSounds ms;
    // aim2
    Organizations orgs;
    OrganizationBases orgsBases;
    Prices prices;

    void load(buffer &b)
    {
        objects.load(b);
        mgs.load(b);
        if (b.eof()) // custom maps
            return;
        mg.load(b);
        mm.load(b);
        ms.load(b);
        if (gameType == GameType::Aim2)
        {
            orgs.load(b);
            orgsBases.load(b);
            prices.load(b);
        }
        
        if (!b.eof())
        {
            stringstream ss;
            ss << hex << b.index() << " != " << hex << b.size();
            throw std::logic_error(ss.str());
        }
    }
};

storage read_mmo(string fn)
{
    buffer f(readFile(fn));
    storage s;
    s.name = fn;
    s.load(f);
    return s;
}

void write_mmo(string db, const storage &s)
{
    using namespace polygon4;
    using namespace polygon4::detail;

    auto storage = initStorage(db);
    storage->load();

    auto p1 = s.name.rfind('\\');
    if (p1 == -1)
        p1 = 0;
    auto p2 = s.name.rfind('/');
    if (p2 == -1)
        p2 = 0;
    int p = max(p1, p2);
    string map_name = s.name.substr(p + 1);
    map_name = map_name.substr(0, map_name.find('.'));
    if (!prefix.empty())
        map_name = prefix + "." + map_name;
    transform(map_name.begin(), map_name.end(), map_name.begin(), ::tolower);

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
        printf("error: this map is not found in the database\n");
        return;
    }

    auto this_map = storage->maps[map_id];

    for (auto &seg : s.objects.segments)
    {
        if (seg->segment_type == SegmentType::SHELL)
        {
            SegmentObjects<Shell> *segment = (SegmentObjects<Shell> *)seg;
            set<string> objs;
            std::map<string, int> bld_ids;
            for (auto &object : segment->objects)
                objs.insert(object.name1);
            for (auto &o : objs)
            {
                auto iter = find_if(storage->buildings.begin(), storage->buildings.end(), [&](const decltype(Storage::buildings)::value_type &p)
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
                    bld_ids[o] = iter->second->getId();
            }
            for (auto &object : segment->objects)
            {
                MapBuilding mb;
                mb.text_id = object.name2;
                mb.building = storage->buildings[bld_ids[object.name1]];
                mb.map = this_map;
                mb.x = object.position.x;
                mb.y = object.position.y;
                mb.z = object.position.z;
                mb.yaw = atan2(object.m_rotate_z[1].x, object.m_rotate_z[0].x);
                mb.pitch = atan2(-object.m_rotate_z[2].x, sqrt(object.m_rotate_z[2].y * object.m_rotate_z[2].y + object.m_rotate_z[2].z * object.m_rotate_z[2].z));
                mb.roll = atan2(object.m_rotate_z[2].y, object.m_rotate_z[2].z);
                auto i = find_if(storage->mapBuildings.begin(), storage->mapBuildings.end(), [&](const decltype(Storage::mapBuildings)::value_type &p)
                {
                    return *p.second.get() == mb;
                });
                if (i == storage->mapBuildings.end())
                {
                    auto mb2 = storage->addMapBuilding(storage->maps[map_id].get());
                    mb.setId(mb2->getId());
                    *mb2.get() = mb;
                }
            }
        }
        if (seg->segment_type == SegmentType::SURFACE   ||
            seg->segment_type == SegmentType::STONE     ||
            seg->segment_type == SegmentType::EXPLOSION)
        {
            SegmentObjects<Surface> *segment = (SegmentObjects<Surface> *)seg;
            set<string> objs;
            std::map<string, int> bld_ids;
            for (auto &object : segment->objects)
                objs.insert(object.name1);
            for (auto &o : objs)
            {
                auto iter = find_if(storage->objects.begin(), storage->objects.end(), [&](const decltype(Storage::objects)::value_type &p)
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
                detail::MapObject mb;
                mb.text_id = object.name2;
                mb.map = this_map;
                mb.object = storage->objects[bld_ids[object.name1]];
                mb.x = object.position.x;
                mb.y = object.position.y;
                mb.z = object.position.z;
                mb.yaw = atan2(object.m_rotate_z[1].x, object.m_rotate_z[0].x);
                mb.pitch = atan2(-object.m_rotate_z[2].x, sqrt(object.m_rotate_z[2].y * object.m_rotate_z[2].y + object.m_rotate_z[2].z * object.m_rotate_z[2].z));
                mb.roll = atan2(object.m_rotate_z[2].y, object.m_rotate_z[2].z);
                auto i = find_if(storage->mapObjects.begin(), storage->mapObjects.end(), [&](const decltype(Storage::mapObjects)::value_type &p)
                {
                    return *p.second.get() == mb;
                });
                if (i == storage->mapObjects.end())
                {
                    auto mb2 = storage->addMapObject(storage->maps[map_id].get());
                    mb.setId(mb2->getId());
                    *mb2.get() = mb;
                }
            }
        }
    }
    storage->save();
}

int main(int argc, char *argv[])
try
{
    if (argc != 4)
    {
        cout << "Usage:\n" << argv[0] << " db.sqlite file.mmo" << "\n";
        return 1;
    }
    prefix = argv[3];
    if (prefix == "m1")
        gameType = GameType::Aim1;
    else if (prefix == "m2")
        gameType = GameType::Aim2;
    else
        throw std::runtime_error("unknown prefix (game type)");
    storage s = read_mmo(argv[2]);
    write_mmo(argv[1], s);
    return 0;
}
catch (std::exception &e)
{
    printf("error: %s\n", e.what());
    return 1;
}
catch (...)
{
    printf("error: unknown exception\n");
    return 1;
}