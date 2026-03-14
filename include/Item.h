#pragma once

#include "Utils.h"
#include <cstdint>

// Item flag constants
#define ITEM_BATTLE_FLAG 1
#define ITEM_WILD_FLAG 2
#define ITEM_WORLD_FLAG 4

class Item {
public:
    static Item* ITEMS[80];

    static void buildItems();
    static Item* getItem(const std::string& name);

    std::string name;
    int price = 0;
    uint8_t quantity = 0;
    uint8_t flags = 0;  //1=battle, 2=wild, 4=world, 8=mon, 16=move

    Item() = default;
    explicit Item(const std::vector<std::string> &a);
    explicit Item(const Item &i);

    bool equals(const Item &o) const;
    std::string toString() const;
    
    // Flag accessors
    bool hasBattleFlag() const { return flags & ITEM_BATTLE_FLAG; }
    bool hasWildFlag() const { return flags & ITEM_WILD_FLAG; }
    bool hasWorldFlag() const { return flags & ITEM_WORLD_FLAG; }
    
    void setBattleFlag(bool set) { if (set) flags |= ITEM_BATTLE_FLAG; else flags &= ~ITEM_BATTLE_FLAG; }
    void setWildFlag(bool set) { if (set) flags |= ITEM_WILD_FLAG; else flags &= ~ITEM_WILD_FLAG; }
    void setWorldFlag(bool set) { if (set) flags |= ITEM_WORLD_FLAG; else flags &= ~ITEM_WORLD_FLAG; }
};
