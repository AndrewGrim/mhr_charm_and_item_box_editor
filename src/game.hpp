#ifndef GAME_HPP
    #define GAME_HPP

    #include "process.hpp"

    enum Rarity : u32 {
        RARITY_1 = 0x10100000,
        RARITY_2 = 0x10100001,
        RARITY_3 = 0x10100002,
        RARITY_4 = 0x10100003,
        RARITY_5 = 0x10100004,
        RARITY_6 = 0x10100005,
        RARITY_7 = 0x10100006,
        RARITY_4_NOVICE = 0x10100007,
        RARITY_3_KINSHIP = 0x10100008,
        RARITY_2_VETERAN = 0x10100009,
        RARITY_2_LEGACY = 0x1010000A
    };

    enum : usize {
        OFFSET_ITEM_BOX = 0x78,
        OFFSET_INVENTORY_LIST = 0x18,
        OFFSET_INVENTORY_SIZE = 0x18,
        OFFSET_ITEMS = 0x10,
        OFFSET_ITEM_INFO = 0x20,
        OFFSET_ITEM_COUNT = 0x14,
        OFFSET_ITEM_ID = 0x10,
        OFFSET_NEXT = 0x08,
        OFFSET_EQUIPMENT_BOX = 0x80,
        OFFSET_EQUIPMENT_LIST = 0x28,
        OFFSET_EQUIPMENT_ITEMS = 0x10,
        OFFSET_EQUIPMENT_SIZE = 0x18,
        OFFSET_EQUIPMENT_LIST_START = 0x20,
        OFFSET_HAND_MONEY = 0x58,
        OFFSET_ZENNY = 0x18,
        OFFSET_VILLAGE_POINTS = 0x60,
        OFFSET_POINTS = 0x10,

        OFFSET_EQ_TYPE = 0x2C,
        OFFSET_RARITY = 0x30,
        OFFSET_IDENTIFIER = 0x1BC,
        OFFSET_SLOTS = 0x70,
        OFFSET_LEVEL_1_SLOTS = 0x24,
        OFFSET_LEVEL_2_SLOTS = 0x28,
        OFFSET_LEVEL_3_SLOTS = 0x2C,
        OFFSET_SKILLS = 0x78,
        OFFSET_SKILL_1_ID = 0x20,
        OFFSET_SKILL_2_ID = 0x21,
        OFFSET_LEVELS = 0x80,
        OFFSET_SKILL_1_LVL = 0x20,
        OFFSET_SKILL_2_LVL = 0x24,
    };

    struct Item {
        u32 id;
        u32 count;
        usize combo_index = -1;
        usize added_order = -1;
        String name;

        Item(u32 id, u32 count, String name) : id(id), count(count), name(name) {}
    };

    struct ItemHidden {
        u32 id;
        u32 count;
        usize combo_index;
        usize added_order;

        ItemHidden(u32 id, u32 count, usize combo_index, usize added_order)
        : id(id), count(count), combo_index(combo_index), added_order(added_order) {}
    };

    struct Charm {
        u32 index = 0;
        u32 rarity = 0x10100000;
        u32 skill_1 = 0;
        u32 skill_2 = 0;
        u32 skill_1_level = 0;
        u32 skill_2_level = 0;
        u32 level_1_slots = 0;
        u32 level_2_slots = 0;
        u32 level_3_slots = 0;
        u32 slots[3] = { 0 };
    };

    enum EquipmentType : u32 {
        EQUIPMENT_TYPE_EMPTY = 0,
        EQUIPMENT_TYPE_TALISMAN = 3,
    };

    struct Game {
        usize data_manager = 0x0;
        Process process;

        Game(Process process) : process(process) {
            #ifdef __WIN32__
                DWORD old;

                if (!VirtualProtectEx(process.id, (void*)DATA_MANAGER_ADDRESS, sizeof(usize), PAGE_EXECUTE_READWRITE, &old)) {
                    assert(false && "VirtualProtectEx (1) failed.");
                }

                if (!process.read(DATA_MANAGER_ADDRESS, &data_manager, sizeof(usize))) {
                    assert(false && "ReadProcessMemory failed when trying to get data_manager ptr.");
                }

                if (!VirtualProtectEx(process.id, (void*)DATA_MANAGER_ADDRESS, sizeof(usize), old, &old)) {
                    assert(false && "VirtualProtectEx (2) failed.");
                }
            #else
                if (!process.read(DATA_MANAGER_ADDRESS, &data_manager, sizeof(usize))) {
                    assert(false && "ReadProcessMemory failed when trying to get data_manager ptr.");
                }
            #endif
        }

        ~Game() {}

        void getItemBox(std::vector<Item> &item_list, HashMap<u32, String> &item_names) {
            item_list.clear();

            usize item_box = 0x0;
            process.read(data_manager + OFFSET_ITEM_BOX, &item_box, sizeof(usize));

            usize inventory_list = 0x0;
            process.read(item_box + OFFSET_INVENTORY_LIST, &inventory_list, sizeof(usize));

            usize items = 0x0;
            process.read(inventory_list + OFFSET_ITEMS, &items, sizeof(usize));
            items += OFFSET_ITEM_INFO;

            u32 size = 0;
            process.read(inventory_list + OFFSET_INVENTORY_SIZE, &size, sizeof(u32));

            for (u32 i = 0; i < size; i++, items += OFFSET_NEXT) {
                usize item = 0x0;
                process.read(items, &item, sizeof(usize));

                usize item_data = 0x0;
                process.read(item + OFFSET_ITEM_INFO, &item_data, sizeof(usize));

                if (item_data) {
                    u32 id, count;

                    process.read(item_data + OFFSET_ITEM_ID, &id, sizeof(id));
                    process.read(item_data + OFFSET_ITEM_COUNT, &count, sizeof(count));

                    item_list.push_back(Item(id, count, item_names[id]));
                    item_list[item_list.size() - 1].added_order = item_list.size() - 1;
                }
            }
        }

        void setItemBox(std::vector<Item> &item_list) {
            usize item_box = 0x0;
            process.read(data_manager + OFFSET_ITEM_BOX, &item_box, sizeof(usize));

            usize inventory_list = 0x0;
            process.read(item_box + OFFSET_INVENTORY_LIST, &inventory_list, sizeof(usize));

            usize items = 0x0;
            process.read(inventory_list + OFFSET_ITEMS, &items, sizeof(usize));
            items += OFFSET_ITEM_INFO;

            u32 size = 0;
            process.read(inventory_list + OFFSET_INVENTORY_SIZE, &size, sizeof(u32));

            for (u32 i = 0; i < size; i++, items += OFFSET_NEXT) {
                usize item = 0x0;
                process.read(items, &item, sizeof(usize));

                usize item_data = 0x0;
                process.read(item + OFFSET_ITEM_INFO, &item_data, sizeof(usize));

                if (item_data) {
                    u32 id = item_list[i].id, count = item_list[i].count;

                    process.write(item_data + OFFSET_ITEM_ID, &id, sizeof(id));
                    process.write(item_data + OFFSET_ITEM_COUNT, &count, sizeof(count));
                }
            }
        }

        void getCharms(std::vector<Charm> &charm_list) {
            charm_list.clear();

            usize equipment_box = 0x0;
            process.read(data_manager + OFFSET_EQUIPMENT_BOX, &equipment_box, sizeof(equipment_box));

            usize equipment_list = 0x0;
            u32 size = 0;
            process.read(equipment_box + OFFSET_EQUIPMENT_LIST, &equipment_list, sizeof(equipment_list));
            process.read(equipment_list + OFFSET_EQUIPMENT_SIZE, &size, sizeof(size));
            process.read(equipment_list + OFFSET_EQUIPMENT_ITEMS, &equipment_list, sizeof(equipment_list));

            equipment_list += OFFSET_EQUIPMENT_LIST;
            for (u32 i = 0; i < size; ++i, equipment_list += OFFSET_NEXT) {
                usize item = 0x0;
                process.read(equipment_list, &item, sizeof(item));

                if (item) {
                    EquipmentType equipment_type = EQUIPMENT_TYPE_EMPTY;
                    process.read(item + OFFSET_EQ_TYPE, &equipment_type, sizeof(equipment_type));

                    if (equipment_type == EQUIPMENT_TYPE_TALISMAN) {
                        Charm charm;

                        u32 value = 0;
                        process.read(item + OFFSET_RARITY, &value, sizeof(u32));

                        charm.index = i;
                        charm.rarity = value;

                        usize subptr = 0;
                        process.read(item + OFFSET_SLOTS, &subptr, sizeof(usize));

                        process.read(subptr + OFFSET_LEVEL_1_SLOTS, &value, sizeof(u32));
                        charm.level_1_slots = value;
                        process.read(subptr + OFFSET_LEVEL_2_SLOTS, &value, sizeof(u32));
                        charm.level_2_slots = value;
                        process.read(subptr + OFFSET_LEVEL_3_SLOTS, &value, sizeof(u32));
                        charm.level_3_slots = value;

                        process.read(item + OFFSET_SKILLS, &subptr, sizeof(usize));

                        process.read(subptr + OFFSET_SKILL_1_ID, &value, sizeof(u8));
                        charm.skill_1 = value & 0xFF;
                        process.read(subptr + OFFSET_SKILL_2_ID, &value, sizeof(u8));
                        charm.skill_2 = value & 0xFF;

                        process.read(item + OFFSET_LEVELS, &subptr, sizeof(usize));

                        process.read(subptr + OFFSET_SKILL_1_LVL, &value, sizeof(u32));
                        charm.skill_1_level = value;
                        process.read(subptr + OFFSET_SKILL_2_LVL, &value, sizeof(u32));
                        charm.skill_2_level = value;

                        for (u32 i = 0; i < charm.level_3_slots; i++) {
                            for (u32 &slot : charm.slots) {
                                if (!slot) { slot = 3; break; }
                            }
                        }

                        for (u32 i = 0; i < charm.level_2_slots; i++) {
                            for (u32 &slot : charm.slots) {
                                if (!slot) { slot = 2; break; }
                            }
                        }

                        for (u32 i = 0; i < charm.level_1_slots; i++) {
                            for (u32 &slot : charm.slots) {
                                if (!slot) { slot = 1; break; }
                            }
                        }

                        charm_list.push_back(charm);
                    }
                }
            }
        }

        String formatCharm(Charm charm, std::vector<String> &skill_names) {
            return String::format(
                "%s | %s | %u-%u-%u",
                skill_names[charm.skill_1], skill_names[charm.skill_2],
                charm.slots[0], charm.slots[1], charm.slots[2]
            );
        }

        u32 findNextEmptyEquipmentSlot() {
            usize equipment_box = 0x0;
            process.read(data_manager + OFFSET_EQUIPMENT_BOX, &equipment_box, sizeof(equipment_box));

            usize equipment_list = 0x0;
            u32 size = 0;
            process.read(equipment_box + OFFSET_EQUIPMENT_LIST, &equipment_list, sizeof(equipment_list));
            process.read(equipment_list + OFFSET_EQUIPMENT_SIZE, &size, sizeof(size));
            process.read(equipment_list + OFFSET_EQUIPMENT_ITEMS, &equipment_list, sizeof(equipment_list));

            equipment_list += OFFSET_EQUIPMENT_LIST;
            for (u32 i = 0; i < size; ++i, equipment_list += OFFSET_NEXT) {
                usize item = 0x0;
                process.read(equipment_list, &item, sizeof(item));

                if (item) {
                    EquipmentType equipment_type = EQUIPMENT_TYPE_EMPTY;
                    process.read(item + OFFSET_EQ_TYPE, &equipment_type, sizeof(equipment_type));

                    if (equipment_type == EQUIPMENT_TYPE_EMPTY) {
                        return i;
                    }
                }
            }
            assert(false && "No more room in the equipement box!");
            return 0;
        }

        void setCharms(std::vector<Charm> &charm_list) {
            usize equipment_box = 0x0;
            process.read(data_manager + OFFSET_EQUIPMENT_BOX, &equipment_box, sizeof(equipment_box));

            usize equipment_list = 0x0;
            u32 size = 0;
            process.read(equipment_box + OFFSET_EQUIPMENT_LIST, &equipment_list, sizeof(equipment_list));
            process.read(equipment_list + OFFSET_EQUIPMENT_SIZE, &size, sizeof(size));
            process.read(equipment_list + OFFSET_EQUIPMENT_ITEMS, &equipment_list, sizeof(equipment_list));

            equipment_list += OFFSET_EQUIPMENT_LIST;

            for (Charm &charm : charm_list) {
                usize charm_offset = equipment_list + (charm.index * OFFSET_NEXT);
                usize item = 0x0;
                process.read(charm_offset, &item, sizeof(item));

                if (item) {
                    EquipmentType equipment_type = EQUIPMENT_TYPE_EMPTY;
                    process.read(item + OFFSET_EQ_TYPE, &equipment_type, sizeof(equipment_type));

                    u32 value = charm.rarity;
                    process.write(item + OFFSET_RARITY, &value, sizeof(u32));

                    value = EQUIPMENT_TYPE_TALISMAN;
                    process.write(item + OFFSET_EQ_TYPE, &value, sizeof(u32));

                    usize subptr = 0x0;
                    process.read(item + OFFSET_SLOTS, &subptr, sizeof(usize));

                    value = charm.level_1_slots;
                    process.write(subptr + OFFSET_LEVEL_1_SLOTS, &value, sizeof(u32));

                    value = charm.level_2_slots;
                    process.write(subptr + OFFSET_LEVEL_2_SLOTS, &value, sizeof(u32));

                    value = charm.level_3_slots;
                    process.write(subptr + OFFSET_LEVEL_3_SLOTS, &value, sizeof(u32));

                    process.read(item + OFFSET_SKILLS, &subptr, sizeof(usize));

                    value = charm.skill_1;
                    process.write(subptr + OFFSET_SKILL_1_ID, &value, sizeof(u8));

                    value = charm.skill_2;
                    process.write(subptr + OFFSET_SKILL_2_ID, &value, sizeof(u8));

                    process.read(item + OFFSET_LEVELS, &subptr, sizeof(usize));

                    value = charm.skill_1_level;
                    process.write(subptr + OFFSET_SKILL_1_LVL, &value, sizeof(u32));

                    value = charm.skill_2_level;
                    process.write(subptr + OFFSET_SKILL_2_LVL, &value, sizeof(u32));
                }
            }
        }

        u32 getZenny() {
            usize hand_money = 0x0;
            process.read(data_manager + OFFSET_HAND_MONEY, &hand_money, sizeof(hand_money));
            u32 zenny = 0;
            process.read(hand_money + OFFSET_ZENNY, &zenny, sizeof(zenny));
            return zenny;
        }

        void setZenny(u32 new_value) {
            usize hand_money = 0x0;
            process.read(data_manager + OFFSET_HAND_MONEY, &hand_money, sizeof(hand_money));
            process.write(hand_money + OFFSET_ZENNY, &new_value, sizeof(new_value));
        }

        u32 getPoints() {
            usize village_points = 0x0;
            process.read(data_manager + OFFSET_VILLAGE_POINTS, &village_points, sizeof(village_points));
            u32 points = 0;
            process.read(village_points + OFFSET_POINTS, &points, sizeof(points));
            return points;
        }

        void setPoints(u32 new_value) {
            usize village_points = 0x0;
            process.read(data_manager + OFFSET_VILLAGE_POINTS, &village_points, sizeof(village_points));
            process.write(village_points + OFFSET_POINTS, &new_value, sizeof(new_value));
        }
    };

#endif
