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

using namespace std;

struct storage
{
    Objects objects;
    MechGroups mgs;
    MapGoods mg;
    MapMusic mm;
    MapSounds ms;
};

storage read_mmo(string fn)
{
    storage s;
    FILE *f = fopen(fn.c_str(), "rb");
    if (!f)
        return s;
    s.objects.load(f);
    s.mgs.load(f);
    if (feof(f))
    {
        // custom maps?
        fclose(f);
        return s;
    }
    s.mg.load(f);
    s.mm.load(f);
    s.ms.load(f);
    fclose(f);
    return s;
}

void write_mmo(string db, const storage &s)
{
    using namespace polygon4;
    using namespace polygon4::detail;

    auto storage = initStorage(db);
    storage->load();

    int map_id = 1;

    for (auto &seg : s.objects.segments)
    {
        if (seg->segment_type == SegmentType::SHELL)
        {
            SegmentObjects<Shell> *segment = (SegmentObjects<Shell> *)seg;
            set<string> objs;
            std::map<string, int> bld_ids;
            std::map<string, int> coord_ids;
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
                    bld_ids[o] = bld->id;
                }
                else
                    bld_ids[o] = iter->second->id;
            }
            for (auto &object : segment->objects)
            {
                MapBuilding mb;
                mb.text_id = object.name2;
                mb.building = storage->buildings[bld_ids[object.name1]];
                auto iter = find_if(storage->coordinates.begin(), storage->coordinates.end(), [&](const decltype(Storage::coordinates)::value_type &p)
                {
                    Coordinate c;
                    c.x = object.position.x;
                    c.y = object.position.y;
                    c.z = object.position.z;
                    return *p.second.get() == c;
                });
                if (iter == storage->coordinates.end())
                {
                    auto c = storage->addCoordinate();
                    c->x = object.position.x;
                    c->y = object.position.y;
                    c->z = object.position.z;
                    mb.coordinate = c;
                }
                else
                    mb.coordinate = iter->second;
                auto i = find_if(storage->mapBuildings.begin(), storage->mapBuildings.end(), [&](const decltype(Storage::mapBuildings)::value_type &p)
                {
                    return *p.second.get() == mb;
                });
                if (i == storage->mapBuildings.end())
                {
                    auto mb2 = storage->addMapBuilding(storage->maps[map_id].get());
                    mb.id = mb2->id;
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
            std::map<string, int> coord_ids;
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
                    bld_ids[o] = bld->id;
                }
                else
                    bld_ids[o] = iter->second->id;
            }
            for (auto &object : segment->objects)
            {
                detail::MapObject mb;
                //mb.text_id = object.name2;
                mb.object = storage->objects[bld_ids[object.name1]];
                auto iter = find_if(storage->coordinates.begin(), storage->coordinates.end(), [&](const decltype(Storage::coordinates)::value_type &p)
                {
                    Coordinate c;
                    c.x = object.position.x;
                    c.y = object.position.y;
                    c.z = object.position.z;
                    return *p.second.get() == c;
                });
                if (iter == storage->coordinates.end())
                {
                    auto c = storage->addCoordinate();
                    c->x = object.position.x;
                    c->y = object.position.y;
                    c->z = object.position.z;
                    mb.coordinate = c;
                }
                else
                    mb.coordinate = iter->second;
                auto i = find_if(storage->mapObjects.begin(), storage->mapObjects.end(), [&](const decltype(Storage::mapObjects)::value_type &p)
                {
                    return *p.second.get() == mb;
                });
                if (i == storage->mapObjects.end())
                {
                    auto mb2 = storage->addMapObject(storage->maps[map_id].get());
                    mb.id = mb2->id;
                    *mb2.get() = mb;
                }
            }
        }
    }
    storage->save();
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Usage:\n" << argv[0] << " db.sqlite file.mmo" << "\n";
        return 1;
    }
    storage s = read_mmo(argv[2]);
    write_mmo(argv[1], s);
    return 0;
}