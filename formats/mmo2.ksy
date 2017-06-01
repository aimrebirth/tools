meta:
    id: mmo
    application: aim2
    endian: le

seq:
  - id: objects
    type: objects
  - id: mech_groups
    type: mech_groups
  - id: map_goods
    type: map_goods
  - id: unk0
    type: u4
  - id: map_music
    type: map_music
  - id: map_sounds
    type: map_sounds

    # aim2 only, add condition
  - id: organizations
    type: organizations_segment
  - id: prices
    type: prices_segment
    #

types:
    prices_segment:
        seq:
          - id: length
            type: u4
          - id: unk0
            type: u4

          - id: prices
            type: prices

          - id: n_building_prices
            type: u4
          - id: building_prices
            type: building_price
            repeat: expr
            repeat-expr: n_building_prices

    prices:
        seq:
          - id: n_prices
            type: u4
          - id: prices
            type: price
            repeat: expr
            repeat-expr: n_prices

    price:
        seq:
          - id: name
            type: aim_string
          - id: type
            type: u4
            enum: item_type

            # expand later!
            # now kaitai does not have bitfields
          - id: modificator_mask
            type: u4

          - id: price
            type: f4
          - id: unk2
            type: f4
          - id: probability
            type: f4

        enums:
            item_type:
                1: glider
                2: equipment
                3: weapon
                4: ammo

    building_price:
        seq:
          - id: name
            type: aim_string
          - id: prices
            type: prices

    organizations_segment:
        seq:
          - id: length
            type: u4
          - id: n_orgs
            type: u4
          - id: organizations
            type: organization
            repeat: expr
            repeat-expr: n_orgs
          - id: n_bases
            type: u4
          - id: organizations
            type: organization_base
            repeat: expr
            repeat-expr: n_bases


    organization_base:
        seq:
          - id: base_name
            type: aim_string
          - id: org_name
            type: aim_string
          - id: unk0
            type: u4

    organization:
        seq:
          - id: unk0
            type: u4
          - id: name
            type: aim_string
          - id: unk2
            size: 0xE0
          - id: configs
            type: organization_config
            repeat: expr
            repeat-expr: 3

    organization_config:
        seq:
          - id: configs
            type: aim_string_vector

    map_sounds:
        seq:
          - id: n
            type: u4
          - id: string
            type: map_sound
            repeat: expr
            repeat-expr: n

    map_sound:
        seq:
          - id: name
            type: aim_string
          - id: unk1
            type: vector4
          - id: unk2
            type: u4
          - id: unk3
            type: vector4

    map_music:
        seq:
          - id: name1
            type: aim_string
          - id: name2
            type: aim_string
          - id: names1
            type: aim_string_vector
          - id: names2
            type: aim_string_vector


    map_goods:
        seq:
          - id: segment_length
            type: u4
          - id: unk2
            type: u4

            # aim1 only, add condition
          #- id: unk3
            #type: u4
            #

          - id: n
            type: u4
          - id: building_goods
            repeat: expr
            repeat-expr: n
            type: building_goods_items

    building_goods_items:
        seq:
          - id: building_goods
            type: building_goods

            # aim2 only, add condition
          - id: unk4
            type: u4
            #

    building_goods:
        seq:
          - id: building
            type: aim_string
          - id: n
            type: u4
          - id: goods
            repeat: expr
            repeat-expr: n
            type: good

    good:
        seq:
          - id: name
            type: aim_string

            # aim1 only, add condition
          #- id: unk1
            #size: 0x40
            #

            # aim2 only, add condition
          - id: unk3
            type: f4
            #

          - id: price
            type: f4

            # aim1 only, add condition
          #- id: unk2
            #type: f4
            # repeat 10
            #

            # aim2 only, add condition
          - id: unk2_1
            type: f4
            repeat: expr
            repeat-expr: 2
          - id: unk2_2
            type: u4
            repeat: expr
            repeat-expr: 2
            #

    mech_groups:
        seq:
            # aim2 only, add condition
          - id: length # segment_length?
            type: u4
            #

          - id: n
            type: u4
          - id: prefix
            size: 0x30
          - id: groups
            repeat: expr
            repeat-expr: n
            type: mech_group

    mech_group:
        seq:
          - id: name
            type: aim_string
          - id: org
            type: aim_string
          - id: type1
            type: u4
          - id: len1
            type: u4
          - id: name1
            size: 0x70

          - id: unk_3_4
            type: u4
            if: type1 == 3 or type1 == 4

          - id: unk_2
            type: u4vector
            if: type1 == 2

          - id: unk0_1_0
            type: u4
            if: type1 == 0 or type1 == 1
          - id: unk1_1_0
            type: u4
            if: type1 == 0 or type1 == 1

          - id: configs
            repeat: expr
            repeat-expr: len1
            type: aim_string

          - id: unk1
            type: u1

    objects:
        seq:
          - id: n_segments
            type: u4
          - id: segments
            repeat: expr
            repeat-expr: n_segments
            type: segments

    segments:
        seq:
          - id: type
            type: u4
            enum: segment_type
          - id: length
            type: u4
          - id: n_objects
            type: u4

          - id: roads
            if: type == segment_type::road
            repeat: expr
            repeat-expr: n_objects
            type: map_object_with_array

          - id: boundaries
            if: type == segment_type::boundary
            repeat: expr
            repeat-expr: n_objects
            type: map_object_with_array

          - id: stones
            if: type == segment_type::stone
            repeat: expr
            repeat-expr: n_objects
            type: map_object

          - id: trees
            if: type == segment_type::tree
            repeat: expr
            repeat-expr: n_objects
            type: map_object

          - id: helpers
            if: type == segment_type::helper
            repeat: expr
            repeat-expr: n_objects
            type: map_object

          - id: buildings
            if: type == segment_type::building
            repeat: expr
            repeat-expr: n_objects
            type: map_object

          - id: lamps
            if: type == segment_type::lamp
            repeat: expr
            repeat-expr: n_objects
            type: map_object

          - id: images
            if: type == segment_type::image
            repeat: expr
            repeat-expr: n_objects
            type: map_object

          - id: anomalies
            if: type == segment_type::anomaly
            repeat: expr
            repeat-expr: n_objects
            type: map_object

          - id: towers
            if: type == segment_type::tower
            repeat: expr
            repeat-expr: n_objects
            type: map_object

          - id: sound_zones
            if: type == segment_type::sound_zone
            repeat: expr
            repeat-expr: n_objects
            type: map_object

          - id: sounds
            if: type == segment_type::sound
            repeat: expr
            repeat-expr: n_objects
            type: sound

        enums:
            segment_type:
                1:  texture
                3:  stone
                4:  tree
                7:  helper
                8:  road
                11: building
                12: image
                13: lamp
                19: sound
                20: anomaly
                22: tower
                23: boundary
                24: sound_zone
                21: unk0
                27: unk1

    sound:
        seq:
          - id: common
            type: common
          - id: unk0
            type: u4
            repeat: expr
            repeat-expr: 11
          - id: name
            type: str
            encoding: cp1251
            size: 0x14


    map_object_with_array:
        seq:
          - id: map_object
            type: map_object
          - id: len
            type: u4
          - id: unk0
            repeat: expr
            repeat-expr: len
            type: u4

    map_object:
        seq:
          - id: common
            type: common
          - id: name1
            type: aim_string
          - id: name2
            type: aim_string

    common:
        seq:
          - id: m_rotate_z
            type: vector4
            repeat: expr
            repeat-expr: 3
          - id: position
            type: vector4
    vector4:
        seq:
          - id: x
            type: f4
          - id: y
            type: f4
          - id: z
            type: f4
          - id: w
            type: f4

    aim_string_vector:
        seq:
          - id: n
            type: u4
          - id: string
            type: aim_string
            repeat: expr
            repeat-expr: n

    aim_string:
        seq:
          - id: string
            type: str
            encoding: cp1251
            size: 0x20

    u4vector:
        seq:
          - id: length
            type: u4
          - id: values
            repeat: expr
            repeat-expr: length
            type: u4

