#pragma once

#include "mmap.h"

struct mmo_storage2 {
    struct object {
        uint32_t type;
        uint32_t size;
        uint32_t n_objects;
    };
    struct mech_group {
        char name[0x20];
        char org[0x20];
        // probably state
        // alive
        // dead
        // in the base
        // in the other mech cargo
        uint32_t type;
        uint32_t n_mechs;
        char comment[0x70];
    };
    struct map_good {
        char name[0x20];
        char cond[0x40];
        float buy_price;
        float sell_price;
        float unk[8];
        uint32_t unk1;

        map_good() = default;
        map_good(const std::string &name, const std::string &condition = {}) {
            *this = map_good{};
            strcpy(this->name, name.c_str());
            strcpy(this->cond, condition.c_str());
        }
    };

    // our own data
    struct section {
        uint32_t offset;
    };
    struct {
        section objects;
        section mech_groups;
        section map_goods;
        section music_and_sounds;
    } sections;
    struct map_building {
        uint32_t offset;
        std::map<std::string, uint32_t> building_goods;
    };
    std::map<std::string, map_building> map_building_goods;

    void load(auto &&fn) {
        primitives::templates2::mmap_file<uint8_t> f{fn};
        stream s{f};
        // objects
        {
            sections.objects.offset = s.p - f.p;
            uint32_t n_segments = s;
            while (n_segments--) {
                object o = s;
                s.skip(o.size);
            }
        }
        // mech_groups
        {
            sections.mech_groups.offset = s.p - f.p;
            uint32_t n_mech_groups = s;
            // two ints
            // and ten more ints?
            s.skip(0x30);
            while (n_mech_groups--) {
                mech_group o = s;

                switch (o.type) {
                case 0: // alive
                case 1: {
                    uint32_t sector_id = s; // road id? or sector id
                    float height = s;    // height?
                } break;
                case 2: {
                    std::vector<uint32_t> t; // current path?
                    uint32_t len = s;
                    t.resize(len);
                    s.read(t);
                } break;
                case 3: // 3 = free mechanoids only?
                case 4: {
                    uint32_t t = s; // other mech id?
                } break;
                default:
                    assert(false);
                }

                while (o.n_mechs--) {
                    struct mech {
                        char cfg_name[0x20];
                    };
                    mech m = s;
                }
                bool hidden = s;
            }
        }
        // map goods
        {
            sections.map_goods.offset = s.p - f.p;
            uint32_t size = s;
            uint32_t unk2 = s;
            float unk3 = s;
            uint32_t n_buildings = s;
            while (n_buildings--) {
                struct bld {
                    char name[0x20];
                };
                bld b = s;
                map_building_goods[b.name].offset = s.p - f.p;
                uint32_t n_goods = s;
                while (n_goods--) {
                    map_good g = s;
                    map_building_goods[b.name].building_goods[g.name] = s.p - f.p;
                }
            }
        }
        // music & sounds section
        {
            sections.music_and_sounds.offset = s.p - f.p;
            uint32_t size = s;
            s.skip(size);
        }
    }
};
