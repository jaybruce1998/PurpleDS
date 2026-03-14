#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "WorldObject.h"
#include "Item.h"
#include "Move.h"
#include "Monster.h"
#include "Battler.h"
#include "PokeMap.h"
#include "Gui.h"
#include "Player.h"
#include "Utils.h"
#include "sprites_data.hpp"
#include "debug.h"

short WorldObject::wid = 0;
Item* WorldObject::POKEFLUTE = nullptr;

void WorldObject::buildWorldObjects() {
    POKEFLUTE = Item::getItem("Pokeflute");
    FILE* file = fopen("nitro:/WorldObject.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    const int buffer_size = 200;
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Process 47 world object lines
    for (int i = 0; i < 47; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            //if(i%5==0)debugCheckpoint(buffer);
            std::vector<std::string> a = utils::split(std::string(buffer), ';');
            PokeMap::POKEMAPS[a[0]]->addItems(a);
        }
    }
    //debugCheckpoint("Ok");
    // Process 10 encounter lines
    for (int i = 0; i < 10; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            std::vector<std::string> a = utils::split(std::string(buffer), ';');
            
            // First 2 encounters are Snorlax encounters
            if (i < 2) {
                PokeMap::POKEMAPS[a[0]]->addSnorlaxEncounter(a[1]);
            } else {
                PokeMap::POKEMAPS[a[0]]->addEncounter(a[1]);
            }
        }
    }
    
    free(buffer);
    fclose(file);
}

void WorldObject::buildWorldObjects(Player *p, const std::string &wobS) {
    for (int i = 0; i < 57; i++) p->objectsCollected[i] = false;
    
    FILE* file = fopen("nitro:/WorldObject.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    const int buffer_size = 1024;
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Process 47 world object lines
    for (int i = 0; i < 47; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            std::vector<std::string> a = utils::split(std::string(buffer), ';');
            PokeMap *pm = PokeMap::POKEMAPS[a[0]];
            for (size_t i = 1; i < a.size(); i++) {
                if (wobS[wid] == '1') {
                    p->objectsCollected[wid++] = true;
                } else {
                    pm->addItem(a[i]);
                }
            }
        }
    }
    
    // Process 10 encounter lines
    for (int i = 0; i < 10; i++) {
        if (fgets(buffer, buffer_size, file)) {
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            std::vector<std::string> a = utils::split(std::string(buffer), ';');
            if (wobS[wid] == '1') {
                p->objectsCollected[wid++] = true;
            } else {
                // First 2 encounters are Snorlax encounters
                if (i < 2) {
                    PokeMap::POKEMAPS[a[0]]->addSnorlaxEncounter(a[1]);
                } else {
                    PokeMap::POKEMAPS[a[0]]->addEncounter(a[1]);
                }
            }
        }
    }
    
    free(buffer);
    fclose(file);
}

WorldObject::WorldObject(const std::string &s) {
    id = wid++;
    std::vector<std::string> a = utils::split(s, ',');
    std::vector<std::string> f = utils::split(a[0], ' ');
    
    // For 3DS, use sprite map to get raw sprite data
    std::string spriteKey = f[0] + "_0";  // Items typically use direction 0
    
    // Check if sprite exists in map
    if (SPRITE_DATA.count(spriteKey)) {
        // bi will be a pointer to the sprite data
        bi = SPRITE_DATA[spriteKey];
    } else {
        // Fallback to RED_0 sprite
        bi = RED_0_data;
    }
    
    x = std::stoi(f[1]);
    y = std::stoi(f[2]);
    if (a[1].rfind("Move", 0) == 0) {
        move = Move::getMove(a[1].substr(5));
    } else if (a[1].rfind("Pokem", 0) == 0) {
        std::vector<std::string> p = utils::split(a[1].substr(8), ' ');
        level = std::stoi(p[0]);
        mon = Monster::MONSTER_MAP[p[1]];
    } else {
        item = Item::getItem(a[1]);
    }
}

std::optional<bool> WorldObject::stepOn(Gui *gui) {
    Player *p = gui->player;
    if (move != nullptr) {
        p->give(move);
    } else if (item != nullptr) {
        p->give(item);
    } else {
        p->give(new Battler(level, mon));
    }
    interacted = true;
    return true;
}

WorldObject::WorldEncounter::WorldEncounter(const std::string &s) : WorldObject(s) {}

std::optional<bool> WorldObject::WorldEncounter::stepOn(Gui *gui) {
    return std::nullopt;
}

WorldObject::SnorlaxEncounter::SnorlaxEncounter(const std::string &s) : WorldEncounter(s) {}

std::optional<bool> WorldObject::SnorlaxEncounter::stepOn(Gui *gui) {
    Player *p = gui->player;
    if (p->hasItem(WorldObject::POKEFLUTE)) {
        return std::nullopt;
    }
    gui->print("It's fast asleep...");
    return false;
}
