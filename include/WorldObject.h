#pragma once

#include "Item.h"
#include "Move.h"
#include "Monster.h"
#include <optional>
#include <nds.h>

class Gui;

class WorldObject {
public:
    static short wid;
    static void buildWorldObjects();
    static void buildWorldObjects(class Player *p, const std::string &wobS);
    
    static Item *POKEFLUTE;

    bool interacted = false;
    const u8* bi = nullptr;
    u8 x = 0;
    u8 y = 0;
    u8 level = 0;
    short id = 0;
    Move *move = nullptr;
    Item *item = nullptr;
    Monster *mon = nullptr;

    WorldObject() = default;
    explicit WorldObject(const std::string &s);
    virtual ~WorldObject() = default;
    virtual std::optional<bool> stepOn(Gui *gui);

    class WorldEncounter;
    class SnorlaxEncounter;
};

class WorldObject::WorldEncounter : public WorldObject {
public:
    explicit WorldEncounter(const std::string &s);
    std::optional<bool> stepOn(Gui *gui) override;
};

class WorldObject::SnorlaxEncounter : public WorldEncounter {
public:
    explicit SnorlaxEncounter(const std::string &s);
    std::optional<bool> stepOn(Gui *gui) override;
};
