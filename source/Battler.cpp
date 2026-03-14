#include "Battler.h"
#include "Gui.h"
#include "Monster.h"
#include "Move.h"
#include "Player.h"
#include "BattleState.h"
#include "Utils.h"
#include "Types.h"
#include "debug.h"

// External global gui instance
extern Gui* g_gui;

u8 Battler::GROUPS[153] = {
3,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,2,2,1,1,2,2,1,1,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,3,3,0,0,0,0,0,0,0,0,0,0,0,0,3,3,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,3,3,0,0,0,1,1,1,1,1,1,1,3,3,1,1,1,1,1,1,1,3,3,2,1,1,1,1,1,1,3,3,1,1,1,1,1,3,3,3,3,3,1,1,1,1,1,1,1,1,1,1,3,3,3,3,3,3,3,3,3,0,3};

int Battler::xpNeeded(int level) {
    switch(group)
    {
        case 0: return static_cast<int>(6 * std::pow(level, 3) / 5 - 15 * level * level + 100 * level - 140);
        case 1: return level * level * level;
        case 2: return static_cast<int>(4 * (std::pow(level, 3)) / 5);
        case 3: return static_cast<int>(5 * (std::pow(level, 3)) / 4);
    }
    throw std::runtime_error("Invalid group: " + std::to_string(group));
}

int Battler::newStat(int base, int dv, int sxp, int c) {
    return static_cast<int>(((base + dv) * 2 + std::sqrt(sxp) / 4) * level / 100 + c);
}

void Battler::calculateStats() {
    hp++;
    mhp = newStat(bHp, hpDv, hpXp, level + 10);
    spd = newStat(bSpd, spdDv, spdXp, 5);
    atk = newStat(bAtk, atkDv, atkXp, 5);
    def = newStat(bDef, defDv, defXp, 5);
    spatk = newStat(bSpatk, spatkDv, spatkXp, 5);
    spdef = newStat(bSpdef, spdefDv, spdefXp, 5);
}

void Battler::become(Monster *m) {
    //debugCheckpoint((m->name).c_str());
    
    if (name == nickname) {
        nickname = m->name;
    }
    dexNum = m->dexNum;
    learnset = LevelUpMove::LEVEL_UP_MOVES[dexNum];
    evolution = Evolution::EVOLUTIONS[dexNum];
    name = m->name;
    types = m->types;
    bAtk = m->atk;
    bDef = m->def;
    bSpatk = m->spatk;
    bSpdef = m->spdef;
    bHp = m->hp;
    bSpd = m->spd;
    lsi = 0;
}

void Battler::finishSetup() {
    nickname = name;
    status = "";
    group = GROUPS[dexNum];
    xp = xpNeeded(level);
    mxp = xpNeeded(level + 1);
    calculateStats();
    hp = mhp;
    for (int i = 0; i < 4; i++) {
        moves[i] = nullptr;
        pp[i] = 0;
        mpp[i] = 0;
    }
    for (int md = 0; lsi < static_cast<int>(learnset.size()); lsi++) {
        if (learnset[lsi].level > level) {
            break;
        }
        moves[md] = learnset[lsi].move;
        md = (md + 1) % 4;
    }
    for (int i = 0; i < 4; i++) {
        if (moves[i] == nullptr) {
            break;
        }
        pp[i] = moves[i]->pp;
        mpp[i] = pp[i];
    }
}

Battler::Battler(int level, Monster *m) {
    become(m);
    this->level = level;
    atkDv = utils::randInt(0, 16);
    defDv = utils::randInt(0, 16);
    spatkDv = utils::randInt(0, 16);
    spdefDv = utils::randInt(0, 16);
    hpDv = utils::randInt(0, 16);
    spdDv = utils::randInt(0, 16);
    shiny = utils::randInt(0, Player::SHINY_CHANCE) == 0;
    finishSetup();
}

Battler::Battler(Battler *b) {
    dexNum = b->dexNum - 1;
    if (dexNum > 132 && dexNum < 136) {
        dexNum = 133;
    } else if (dexNum < 1 || Evolution::EVOLUTIONS[dexNum] == nullptr) {
        dexNum++;
    } else {
        dexNum--;
        if (Evolution::EVOLUTIONS[dexNum] == nullptr) {
            dexNum++;
        }
    }
    become(&Monster::MONSTERS[dexNum]);
    level = 1;
    int n = 0;
    int s = Player::SHINY_CHANCE / 2;
    atkDv = b->atkDv == 15 ? 15 : utils::randInt(0, 16);
    if (atkDv == 15) n++;
    defDv = b->defDv == 15 ? 15 : utils::randInt(0, 16);
    if (defDv == 15) n++;
    spatkDv = b->spatkDv == 15 ? 15 : utils::randInt(0, 16);
    if (spatkDv == 15) n++;
    spdefDv = b->spdefDv == 15 ? 15 : utils::randInt(0, 16);
    if (spdefDv == 15) n++;
    hpDv = b->hpDv == 15 ? 15 : utils::randInt(0, 16);
    if (hpDv == 15) n++;
    spdDv = b->spdDv == 15 ? 15 : utils::randInt(0, 16);
    if (spdDv == 15) n++;
    shiny = utils::randInt(0, b->shiny ? static_cast<int>(std::sqrt(s)) : s) == 0;
    finishSetup();
    g_gui->print(std::to_string(n) + " perfect DVs! " +
                        (shiny ? "And it's shiny!!" : (n == 0 ? " Wow that sucks!" : "Not too shabby...")));
}

Battler::Battler(const std::string &s) {
    std::vector<std::string> a = utils::split(s, '/');
    std::vector<std::string> i = utils::split(a[0], ',');
    name = i[2];
    become(&Monster::MONSTERS[std::stoi(i[0])]);
    level = std::stoi(i[1]);
    nickname = i[2];
    status = i[3];
    xp = std::stoi(i[4]);
    hp = std::stoi(i[5]) - 1;
    lsi = std::stoi(i[6]);
    atkDv = std::stoi(i[7]);
    defDv = std::stoi(i[8]);
    spatkDv = std::stoi(i[9]);
    spdefDv = std::stoi(i[10]);
    hpDv = std::stoi(i[11]);
    spdDv = std::stoi(i[12]);
    shiny = i[13] == "1";
    atkXp = std::stoi(i[14]);
    defXp = std::stoi(i[15]);
    spatkXp = std::stoi(i[16]);
    spdefXp = std::stoi(i[17]);
    hpXp = std::stoi(i[18]);
    spdXp = std::stoi(i[19]);
    group = GROUPS[dexNum];
    mxp = xpNeeded(level + 1);
    calculateStats();
    for (int i = 0; i < 4; i++) {
        moves[i] = nullptr;
        pp[i] = 0;
        mpp[i] = 0;
    }
    for (int j = static_cast<int>(a.size()) - 2; j >= 0; j--) {
        i = utils::split(a[j + 1], ',');
        moves[j] = Move::getMove(i[0]);
        std::vector<std::string> q = utils::split(i[1], 'x');
        pp[j] = std::stoi(q[0]);
        if (q.size() == 2) {
            mpp[j] = std::stoi(q[1]);
        } else {
            mpp[j] = moves[j]->pp;
        }
    }
}

void Battler::learn(Move** lastMoves, Move *m) {
    g_gui->print(nickname + " is trying to learn " + m->name + "! Select a move to replace it with.");

    setMoveStrings();
    g_gui->longArrHeader = "Replace which move with " + m->name + "?";
    u8 v = g_gui->waitForChoice(4);
    g_gui->longArrHeader.clear();

    if (v == 4) {
        g_gui->print(nickname + " did not learn " + m->name + ".");
        return;
    }
    g_gui->print(nickname + " forgot how to use " + lastMoves[v]->name + "...");
    lastMoves[v] = m;
    moves[v] = m;
    mpp[v] = m->pp;
    if(mpp[v]<pp[v])
        pp[v]=mpp[v];
    g_gui->print(" and learned how to use " + m->name + "!");
}

bool Battler::newMove(Move *m) {
    for (auto *mv : moves) {
        if (mv == m) {
            return false;
        }
    }
    return true;
}

void Battler::learnLevelUpMoves(Move** lastMoves) {
    while (lsi < static_cast<int>(learnset.size())) {
        if (learnset[lsi].level <= level) {
            if (newMove(learnset[lsi].move)) {
                if (lastMoves[3] == nullptr) {
                    for (int i = 2; true; i--) {
                        if (lastMoves[i] != nullptr) {
                            moves[++i] = learnset[lsi].move;
                            lastMoves[i] = moves[i];
                            pp[i] = moves[i]->pp;
                            mpp[i] = pp[i];
                            g_gui->print(nickname + " learned " + moves[i]->name + "!");
                            break;
                        }
                    }
                } else {
                    learn(lastMoves, learnset[lsi].move);
                }
            }
            lsi++;
        } else {
            break;
        }
    }
}

void Battler::gainXp(Player *p, Move** lastMoves, int xp, int atk, int def, int spatk, int spdef, int hp, int spd) {
    this->xp += xp;
    atkXp += atk;
    defXp += def;
    spatkXp += spatk;
    spdefXp += spdef;
    hpXp += hp;
    spdXp += spd;
    g_gui->print(nickname + " gained " + std::to_string(xp) + " experience points!");
    while (this->xp >= mxp) {
        level++;
        g_gui->print(nickname + " grew to level " + std::to_string(level) + "!");
        mxp = xpNeeded(level + 1);
            learnLevelUpMoves(lastMoves);
            if (evolution != nullptr && evolution->stone.empty() && evolution->level <= level) {
                std::string n = nickname;
                become(&Monster::MONSTERS[evolution->evo]);
                g_gui->print(n + " evolved into " + name + "!");
                p->registerBattler(this);
                learnLevelUpMoves(lastMoves);
            }
        calculateStats();
    }
}

bool Battler::useStone(const std::string &s) {
    if (dexNum == 133) {
        if (s == "Water Stone") {
            evolution = new Evolution(s, 134);
        } else if (s == "Thunder Stone") {
            evolution = new Evolution(s, 135);
        } else if (s == "Fire Stone") {
            evolution = new Evolution(s, 136);
        } else {
            g_gui->print("Neat idea, but nope, sorry! Try a different stone...");
            return false;
        }
    } else if (evolution == nullptr || s != evolution->stone) {
        g_gui->print("Quit messing around!");
        return false;
    }
    std::string n = nickname;
    become(&Monster::MONSTERS[evolution->evo]);
    g_gui->print(n + " evolved into " + name + "!");
    return true;
}

void Battler::setMoveStrings() const {
    for (int i = 0; i < 4; i++) {
        if (moves[i] == nullptr) {
            g_gui->longArr[i]="";
            return;
        } else {
            g_gui->longArr[i] = moves[i]->name + " (" + std::to_string(pp[i]) + "/" + std::to_string(mpp[i]) + ")";
        }
    }
    g_gui->longArr[4]="";
}

void Battler::fullyHeal() {
    hp = mhp;
    status.clear();
    for (int i = 0; i < 4; i++) {
        if (moves[i] == nullptr) {
            return;
        }
        pp[i] = mpp[i];
    }
}

void Battler::refreshStats() {
    int oldMhp = mhp;
    calculateStats();
    if (oldMhp > 0 && mhp > 0) {
        hp = std::min(hp, mhp);
    }
}

void Battler::setInformation() const {
    g_gui->longArr[0]=nickname + "/" + name;
    g_gui->longArr[1]=(status.empty() ? "healthy" : status) + "/" + (shiny ? "SHINY" : "happy");
    
    // Build type string from bit-packed types
    std::string typeStr = Types::TYPES[getType1()];
    u8 type2 = getType2();
    if (type2 != 15) { // 15 = no type
        typeStr += std::string("/") + Types::TYPES[type2];
    }
    
    g_gui->longArr[2]="Level: " + std::to_string(level) + ", " + typeStr;
    g_gui->longArr[3]="XP: " + std::to_string(xp) + "/" + std::to_string(mxp);
    g_gui->longArr[4]=" ";
    g_gui->longArr[5]="HP: " + std::to_string(hp) + "/" + std::to_string(mhp) + " DV: (" + std::to_string(hpDv) + ")";
    g_gui->longArr[6]="Speed: " + std::to_string(spd) + " DV: (" + std::to_string(spdDv) + ")";
    g_gui->longArr[7]="Attack: " + std::to_string(atk) + " DV: (" + std::to_string(atkDv) + ")";
    g_gui->longArr[8]="Defense: " + std::to_string(def) + " DV: (" + std::to_string(defDv) + ")";
    g_gui->longArr[9]="Special Attack: " + std::to_string(spatk) + " DV: (" + std::to_string(spatkDv) + ")";
    g_gui->longArr[10]="Special Defense: " + std::to_string(spdef) + " DV: (" + std::to_string(spdefDv) + ")";
    g_gui->longArr[11]=" ";
    g_gui->longArr[12]="Moves:";
    g_gui->longArr[13]=moves[0]->name + " " + std::to_string(pp[0]) + "/" + std::to_string(mpp[0]);
    g_gui->longArr[14]=moves[1] == nullptr ? "" : moves[1]->name + " " + std::to_string(pp[1]) + "/" + std::to_string(mpp[1]);
    g_gui->longArr[15]=moves[2] == nullptr ? "" : moves[2]->name + " " + std::to_string(pp[2]) + "/" + std::to_string(mpp[2]);
    g_gui->longArr[16]=moves[3] == nullptr ? "" : moves[3]->name + " " + std::to_string(pp[3]) + "/" + std::to_string(mpp[3]);
    g_gui->longArr[17]="";
    g_gui->longArrHeader="Stats";
}

void Battler::append(std::string &out) const {
    std::ostringstream sb;
    sb << static_cast<int>(dexNum) << "," << static_cast<int>(level) << "," << nickname << "," << status << "," << xp << "," << hp << "," << static_cast<int>(lsi) << ","
       << static_cast<int>(atkDv) << "," << static_cast<int>(defDv) << "," << static_cast<int>(spatkDv) << "," << static_cast<int>(spdefDv) << "," << static_cast<int>(hpDv) << "," << static_cast<int>(spdDv) << ","
       << (shiny ? '1' : '0') << "," << atkXp << "," << defXp << "," << spatkXp << "," << spdefXp << "," << hpXp << ","
       << spdXp;
    for (int i = 0; i < 4; i++) {
        if (moves[i] == nullptr) {
            break;
        }
        sb << "/" << moves[i]->name << "," << static_cast<int>(pp[i]) << "x" << static_cast<int>(mpp[i]);
    }
    out += sb.str();
}

std::string Battler::toString() const {
    std::string s = nickname + " L" + std::to_string(level) + ", HP: " + std::to_string(hp) + "/" + std::to_string(mhp);
    if (status == "POISONED") {
        s += " PSN";
    } else if (status == "PARALYZED") {
        s += " PRZ";
    } else if (status == "SLEEPING") {
        s += " SLP";
    } else if (status == "FROZEN") {
        s += " FZN";
    } else if (status == "BURNED") {
        s += " BRN";
    }
    return s;
}
