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
        char comment[0x70]; // icon????
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
        uint32_t offset{};
        uint32_t size{};

        void end(auto v) {
            size = v - offset;
        }
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
    struct mech {
        std::string name;
        std::string org;
        std::string comment;
        uint32_t name_offset;
        uint32_t n_mechs_offset;
        uint32_t mechs_offset;
        uint32_t post_comment_offset;
        uint32_t offset;
        uint32_t size;
    };
    path fn;
    uint32_t n_mech_groups_offset;
    uint32_t mech_groups_offset;
    std::map<std::string, mech> mechs;
    std::map<std::string, map_building> map_building_goods;

    mmo_storage2(const path &fn) : fn{fn} {
    }
    void load() {
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
            sections.objects.end(s.p - f.p);
        }
        // mech_groups
        {
            sections.mech_groups.offset = s.p - f.p;
            n_mech_groups_offset = s.p - f.p;
            uint32_t n_mech_groups = s;
            // two ints
            // and ten more ints?
            s.skip(0x30);
            mech_groups_offset = s.p - f.p;
            while (n_mech_groups--) {
                auto start = s.p - f.p;
                mech_group &o = s;

                auto &m = mechs[o.name];
                m.name = o.name;
                m.org = o.org;
                m.comment = o.comment;
                m.name_offset = (uint8_t*)o.name - f.p;
                m.n_mechs_offset = (uint8_t*)&o.n_mechs - f.p;
                m.post_comment_offset = (uint8_t*)s.p - f.p;

                if (strcmp(o.name, "MINVACH-6") == 0) {
                    int a = 5;
                    a++;
                }
                if (strcmp(o.name, "DEKON") == 0) {
                    int a = 5;
                    a++;
                }
                if (strcmp(o.name, "ETRANNA-1") == 0) {
                    int a = 5;
                    a++;
                }
                if (strcmp(o.name, "SHUN-2") == 0) {
                    int a = 5;
                    a++;
                }

                // mech are spawned on random path when leaving the base
                // probably far from player
                //
                // 0 = stationary
                // 0 = CGuardMechGroup?
                // 1 = roaming between bases (and trading?)
                // 1 = CTransMechGroup!
                // 2 = patrol? or roaming and killing?
                // 2 = CHunterMechGroup!
                // 3 = ? free mechs? spawn pos? flag? free roaming? always 0 except single ORG_FREE mob in location5
                // 3 = CPeopleMechGroup
                // 4 = ? platforms? spawn pos? probably only single appearence in the game - location2 GL_PLATFORM
                // 4 = CGuardMechGroup?
                switch (o.type) {
                case 0: // alive
                {
                    uint32_t sector_id = s; // road id? or map sector id
                    float height = s;    // height?

                    int a = 5;
                    a++;
                } break;
                case 1: {
                    uint32_t road_id = s; // road id? or map sector id
                    float unused_guessed = s;    // height?

                    int a = 5;
                    a++;
                } break;
                // patrol? because transes have type '1'
                // and seems pathes are generated ingame between bases
                case 2: {
                    std::vector<uint32_t> t; // current path?
                    uint32_t len = s;
                    t.resize(len);
                    s.read(t);
                } break;
                case 3: // 3 = free mechanoids only?
                {
                    // dead in other mech id?
                    // object id? base id?
                    uint32_t t = s; // other mech id?
                    if (t != 0) {
                        int a = 5;
                        a++;
                    }

                    int a = 5;
                    a++;
                } break;
                case 4: {
                    // dead in other mech id?
                    // object id? base id?
                    uint32_t t = s; // other mech id?

                    int a = 5;
                    a++;
                } break;
                default:
                    assert(false);
                }

                auto n = o.n_mechs;
                m.mechs_offset = s.p - f.p;
                while (n--) {
                    struct mech {
                        char cfg_name[0x20];
                    };
                    mech m = s;
                }
                bool hidden = s;
                m.offset = start;
                m.size = (s.p - f.p) - start;
            }
            sections.mech_groups.end(s.p - f.p);
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
            sections.map_goods.end(s.p - f.p);
        }
        // music & sounds section
        {
            sections.music_and_sounds.offset = s.p - f.p;
            uint32_t size = s;
            s.skip(size);
            sections.music_and_sounds.end(s.p - f.p);
        }
    }
};
