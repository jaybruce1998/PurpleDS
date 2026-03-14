#pragma once

#include "Monster.h"
#include "LevelUpMove.h"
#include "Evolution.h"
#include <nds.h>

class Player;

class Battler : public Monster {
public:
    static u8 GROUPS[153];

    std::string nickname;
    std::string status;
    u8 group;
    short level = 0;
    int mhp = 0;
    int xp = 0;
    int mxp = 0;
    u8 atkDv = 0;
    u8 defDv = 0;
    u8 spatkDv = 0;
    u8 spdefDv = 0;
    u8 hpDv = 0;
    u8 spdDv = 0;
    int atkXp = 0;
    int defXp = 0;
    int spatkXp = 0;
    int spdefXp = 0;
    int hpXp = 0;
    int spdXp = 0;
    short bAtk = 0;
    short bDef = 0;
    short bSpatk = 0;
    short bSpdef = 0;
    short bHp = 0;
    short bSpd = 0;
    u8 lsi = 0;
    Move* moves[4];
    u8 pp[4];
    u8 mpp[4];
    bool shiny = false;
    std::vector<LevelUpMove> learnset;
    Evolution *evolution = nullptr;

    Battler() = default;
    Battler(int level, Monster *m);
    Battler(Battler *b);
    explicit Battler(const std::string &s);

    int xpNeeded(int level);
    void fullyHeal();
    void setInformation() const;
    void setMoveStrings() const;
    void append(std::string &out) const;
    std::string toString() const;

    void refreshStats();

    void gainXp(Player *p, Move** lastMoves, int xp, int atk, int def, int spatk, int spdef, int hp, int spd);
    bool newMove(Move *m);
    void learn(Move** lastMoves, Move *m);
    bool useStone(const std::string &s);

private:
    int newStat(int base, int dv, int sxp, int c);
    void calculateStats();
    void become(Monster *m);
    void finishSetup();
    void learnLevelUpMoves(Move** lastMoves);
};
