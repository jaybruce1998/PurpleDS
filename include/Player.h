#pragma once

#include "Item.h"
#include "Move.h"
#include "Battler.h"
#include <nds.h>

class Player {
public:
    static short SHINY_CHANCE;
    
    Move* tmHms[58];
    u8 numCaught = 0;
    u32 money = 0;
    bool trainersBeaten[309];
    bool leadersBeaten[9];
    bool gioRivalsBeaten[9];
    bool objectsCollected[126];
    bool pokedex[152];
    Battler* team[6];
    std::vector<Battler*> pc;
    std::string name;
    bool ballin = true;

    Player() = default;
    explicit Player(const std::string &name);
    Player(const std::string &pcS, const std::string &partyS, const std::string &dexS, const std::string &itemS,
           const std::string &tmS);

    bool give(Move *move);
    void give(Item *item);
    void give(Item *item, int n);
    void addItem(Item* item);
    void removeItem(Item* item);
    size_t getTeamSize() const;
    void registerBattler(Battler *battler);
    void give(Battler *battler);
    bool hasItem(Item *item) const;
    bool hasMove(Move *move) const;
    void healTeam();
    void sell(Item *item, int q);
    bool use(Item *item);
};
