#include "Player.h"
#include "Gui.h"
#include "Item.h"
#include "Move.h"
#include "Trainer.h"
#include "WorldObject.h"
#include "Utils.h"
#include <algorithm>
#include "debug.h"

short Player::SHINY_CHANCE = 8192;

Player::Player(const std::string &name) : name(name) {
    for (u8 i = 0; i < 58; i++) tmHms[i] = nullptr;
    for (int i = 0; i < 309; i++) trainersBeaten[i] = false;
    for (int i = 0; i < 9; i++) leadersBeaten[i] = false;
    for (int i = 0; i < 9; i++) gioRivalsBeaten[i] = false;
    for (int i = 0; i < 126; i++) objectsCollected[i] = false;
    for (int i = 0; i < 152; i++) pokedex[i] = false;
    for (int i = 0; i < 6; i++) team[i] = nullptr;
    pc.clear();
    ballin = true;
}

Player::Player(const std::string &pcS, const std::string &partyS, const std::string &dexS, const std::string &itemS,
               const std::string &tmS) {
    pc.clear();
    if (!pcS.empty()) {
        for (const std::string &p : utils::split(pcS, ';')) {
            pc.push_back(new Battler(p));
        }
    }
    std::vector<std::string> a = utils::split(partyS, ';');
    for (size_t i = 0; i < a.size(); i++) {
        team[i] = new Battler(a[i]);
    }
    // Fill remaining slots with nullptr
    for (size_t i = a.size(); i < 6; i++) {
        team[i] = nullptr;
    }
    for (size_t i = 0; i < 152; i++) {
        pokedex[i] = dexS[i] == '1';
    }
    if (!itemS.empty()) {
        for (const std::string &s : utils::split(itemS, ';')) {
            a = utils::split(s, ',');
            give(Item::getItem(a[0]), std::stoi(a[1]));
        }
    }
    for (u8 i = 0; i < 58; i++) {
        tmHms[i] = nullptr;
    }
    a = utils::split(tmS, ',');
    for (size_t i = 1; i < a.size(); i++) {
        tmHms[i - 1] = Move::getMove(a[i]);
    }
}

bool Player::give(Move *move) {
    // Find empty slot in static array
    for (u8 i = 0; i < 58; i++) {
        if (tmHms[i] == nullptr) {
            tmHms[i] = move;
            g_gui->print("You got " + move->name + "!");
            return true;
        }
        else if (tmHms[i] == move) {
            g_gui->print("You already have " + move->name + "!");
            return false;
        }
    }
    g_gui->print("TM");
    return false;
}

void Player::addItem(Item* item)
{
    if(item->quantity>0)
        return;
    for(u8 i=0, j; i<80; i++)
        if(Item::ITEMS[i]==item)
            return;
        else if(Item::ITEMS[i]->quantity==0)
        {
            for(j=i+1; Item::ITEMS[j]!=item; j++);
            Item::ITEMS[j]=Item::ITEMS[i];
            Item::ITEMS[i]=item;
            return;
        }
}

void Player::give(Item *item) {
    if (item->name == "Master Ball") {
        ballin = false;
    }
    addItem(item);
    item->quantity++;
    g_gui->print("You got " + item->name + "!");
}

void Player::give(Item *item, int n) {
    addItem(item);
    item->quantity+=n;
}

void Player::registerBattler(Battler *battler) {
    if (!pokedex[battler->dexNum]) {
        pokedex[battler->dexNum] = true;
        numCaught++;
        g_gui->print("That's a new species!");
    }
}

void Player::give(Battler *battler) {
    std::string nick = g_gui->promptText("What is " + battler->name + "'s new nickname?");
    if (!nick.empty()) {
        nick.erase(std::remove(nick.begin(), nick.end(), ','), nick.end());
        if (nick.empty()) {
            battler->nickname = battler->name;
        } else {
            battler->nickname = nick.size() < 11 ? nick : nick.substr(0, 10);
        }
    }
    g_gui->print("You got " + battler->nickname + "!");
    if (team[5] != nullptr) {
        pc.push_back(battler);
        g_gui->print("It was sent to the PC!");
    } else {
        for (size_t i = 0; i < 6; i++) {
            if (team[i] == nullptr) {
                team[i] = battler;
                break;
            }
        }
    }
    registerBattler(battler);
}

bool Player::hasItem(Item *item) const {
    return item->quantity > 0;
}

bool Player::hasMove(Move *move) const {
    for (Move *existing : tmHms) {
        if (existing == move) {
            return true;
        }
    }
    return false;
}

void Player::healTeam() {
    for (int i = 0; i < 6; i++) {
        if (team[i] == nullptr) {
            return;
        }
        team[i]->fullyHeal();
    }
}

void Player::removeItem(Item* item)
{
    if(item->quantity>0)
        return;
    for(u8 i=0, j; i<80; i++)
        if(Item::ITEMS[i]==item)
        {
            for(j=i+1; j<80 && Item::ITEMS[j]->quantity>0; j++)
                Item::ITEMS[j-1]=Item::ITEMS[j];
            Item::ITEMS[j-1]=item;
            return;
        }
}

void Player::sell(Item *item, int q) {
    q = std::min(q, static_cast<int>(item->quantity));
    money += item->price / 2 * q;
    item->quantity -= q;
    removeItem(item);
}

bool Player::use(Item *item) {
    item->quantity--;
    removeItem(item);
    return true;
}
