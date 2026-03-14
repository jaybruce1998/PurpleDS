#pragma once

#include "Battler.h"
#include "Item.h"
#include <unordered_set>

class Gui;
class Player;

class BattleState {
public:
    static Move *STRUGGLE;
    static Move *CONFUSION;
    static Move *RECHARGE;
    static Move *ROAR;
    static Move *TELEPORT;
    static Move *WHIRLWIND;
    static short EXPERIENCE_TABLE[153];
    static Item *LUCKY_EGG;

    static int guiChoice(int m);
    static int wildBattle(Battler** monsters, Battler *wildMon);
    static int trainerBattle(Battler** monsters, const std::string &tName, Battler** tMonsters);

    Battler *monster = nullptr;
    int coins = 0;
    u8 accStage = 0;
    u8 evsnStage = 0;
    u8 spdStage = 0;
    u8 atkStage = 0;
    u8 defStage = 0;
    u8 spatkStage = 0;
    u8 spdefStage = 0;
    int critMul = 1;
    int spd = 0;
    int atk = 0;
    int def = 0;
    int spatk = 0;
    int spdef = 0;
    u8 sTurns = 0;
    u8 tTurns = 0;
    u8 cTurns = 0;
    u8 disabled = 0;
    u8 dTurns = 0;
    u8 spdefTurns = 0;
    u8 defTurns = 0;
    int subHp = 0;
    u8 poisonDamage = 0;
    u8 types = 0; // Bit-packed: low 4 bits = type1, high 4 bits = type2 (15 = no type)
    bool crit = false;
    bool wasPhysical = false;
    bool flinched = false;
    bool immune = false;
    bool seeded = false;
    bool canLower = true;
    bool raging = false;
    Move *nextMove = nullptr;
    Move *lastMove = nullptr;
    int lastDamage[2]{0, 0};
    Move* moves[4];

    BattleState() = default;
    explicit BattleState(Battler *monster);
    BattleState(Battler *monster, BattleState *b);

    bool goBefore(Move *m, BattleState *b, Move *o);
    bool canMove();
    bool doMove(Move *m, BattleState *b);
    bool endTurn();

private:
    static int getMoveDex(BattleState *b);
    static int randomMoveDex(BattleState *b);
    static bool blackedOut(Battler** monsters);
    static int coinsPickup(BattleState *pState, BattleState *oState);
    static bool canSwitch(Battler** monsters);
    static int switchMon(Battler** monsters, int i, bool mustSwitch);
    static void seed(BattleState *sucker, BattleState *sucked);
    void gainXp(Player *p, const std::unordered_set<Battler*> &participants, Battler *fainted, bool trainer, int lucky);
    static double effect(int stage);
    double speed();
    static double accuracy(int stage);
    double stab(Move *m);
    bool hasType(const std::string &type);
    int calcDamage(Move *m, BattleState *b);
    bool setLastMove(Move *m, int d);
    bool disable(Move *m, int a, int b);
    bool shouldStruggle();
    void resetStats();
    void processMiss(Move *m);
    std::string es;
};
