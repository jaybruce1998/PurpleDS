// 3DS Conversion of Pokemon Purple - Main Entry Point
// Based on original Main.cpp with SDL replaced by DS graphics system

#include "BattleState.h"
#include "Blocker.h"
#include "Encounter.h"
#include "Evolution.h"
#include "FlyLocation.h"
#include "Giver.h"
#include "Item.h"
#include "LevelUpMove.h"
#include "MartItem.h"
#include "Monster.h"
#include "Move.h"
#include "Trainer.h"
#include "Types.h"
#include "Warp.h"
#include "WorldObject.h"
#include "debug.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <sstream>

#include <nds.h>
#include <filesystem.h>
#include <fat.h>

#include "Gui.h"
#include "Player.h"
#include "TmLearnsets.h"
#include "Trader.h"

// Global SDL_Event for reuse (avoids creating multiple events per frame)
SDL_Event g_sdlEvent;

// SDL key constants
#define SDLK_LEFT 1
#define SDLK_RIGHT 2
#define SDLK_UP 3
#define SDLK_DOWN 4
#define SDLK_a 5
#define SDLK_d 6
#define SDLK_w 7
#define SDLK_s 8
#define SDLK_ESCAPE 9
#define SDLK_RETURN 10
#define SDLK_t 11
#define SDLK_y 12
#define SDLK_m 13
#define SDLK_i 14
#define SDLK_1 15
#define SDLK_2 16
#define SDLK_3 17
#define SDLK_4 18
#define SDLK_5 19
#define SDLK_6 20
#define SDLK_7 21
#define SDLK_8 22
#define SDLK_r 23
#define SDLK_BACKSPACE 24

#define SDL_BUTTON_LEFT 1

// Replace SDL with DS headers
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <fstream>
#include <iostream>
#include <vector>

// Include our DS graphics system
#include "font_data.hpp"
#include "tiles_data_compressed.h"
#include "sprites_data.hpp"
#include "battlers_data.h"

// External declarations for battler and sprite arrays
extern const unsigned char* battlers_bgr[];
extern const unsigned int battlers_bgr_sizes[];
extern const unsigned char RED_0_bgr[];
extern const unsigned int RED_0_bgr_size;

// Forward declarations
static void buildGameData();

static void buildGameData() {
    //debugCheckpoint("Step 1/16: Types.buildTypes() DONE");
    
    Monster::buildMonsters();
    //debugCheckpoint("Step 2/16: Monster.buildMonsters() DONE");
    
    Move::buildMoves();
    //debugCheckpoint("Step 3/16: Move.buildMoves() DONE");
    
    LevelUpMove::buildLevelUpMoves();
    //debugCheckpoint("Step 4/16: LevelUpMove.buildLevelUpMoves() DONE");
    
    Evolution::buildEvolutions();
    //debugCheckpoint("Step 5/16: Evolution.buildEvolutions() DONE");
    
    PokeMap::buildPokeMaps();
    //debugCheckpoint("Step 6/16: PokeMap.buildPokeMaps() DONE");
    
    FlyLocation::buildWorldMap();
    //debugCheckpoint("Step 7/16: FlyLocation.buildWorldMap() DONE");
    
    Warp::buildWarps();
    //debugCheckpoint("Step 8/16: Warp.buildWarps() DONE");
    
    Encounter::buildEncounterRates();
    //debugCheckpoint("Step 9/16: Encounter.buildEncounterRates() DONE");
    
    Item::buildItems();
    //debugCheckpoint("Step 10/16: Item.buildItems() DONE");
    
    MartItem::buildMartItems();
    //debugCheckpoint("Step 11/16: MartItem.buildMartItems() DONE");
    
    TmLearnsets::buildTmLearnsets();
    //debugCheckpoint("Step 12/16: TmLearnsets.buildTmLearnsets() DONE");
    
    Giver::buildGivers();
    //debugCheckpoint("Step 13/16: Giver.buildGivers() DONE");
    
    Blocker::buildBlockers();
    //debugCheckpoint("Step 14/16: Blocker.buildBlockers() DONE");
    
    Trader::buildTraders();
    //debugCheckpoint("Step 15/16: Trader.buildTraders() DONE");
    
    Npc::buildNpcs();
    //debugCheckpoint("Built everyone");
}

// Helper function to start game from save file
void startFromSave(std::stringstream& saveFile, Player*& player) {
    std::string pInfo;
    std::string pcS;
    std::string partyS;
    std::string trainS;
    std::string leadS;
    std::string gioRS;
    std::string wobS;
    std::string dexS;
    std::string itemS;
    std::string tmS;
    std::string fLoc;
    std::getline(saveFile, pInfo);
    std::getline(saveFile, pcS);
    std::getline(saveFile, partyS);
    std::getline(saveFile, trainS);
    std::getline(saveFile, leadS);
    std::getline(saveFile, gioRS);
    std::getline(saveFile, wobS);
    std::getline(saveFile, dexS);
    std::getline(saveFile, itemS);
    std::getline(saveFile, tmS);
    std::getline(saveFile, fLoc);
    player = new Player(pcS, partyS, dexS, itemS, tmS);
    for (size_t i = 0; i < 38; i++) {
        if (fLoc[i] == '1') {
            FlyLocation::FLY_LOCATIONS[FlyLocation::INDEX_MEANINGS[i + 1]]->visited = true;
        }
    }
    Trainer::buildTrainers(player, trainS, leadS, gioRS);
    WorldObject::buildWorldObjects(player, wobS);
    if (player->hasItem(Item::getItem("Shiny Charm"))) {
        Player::SHINY_CHANCE = 256;
    }
    
    // Create global Gui instance with save info to restore map position
    g_gui = new Gui(player, pInfo);
}

// Helper function to start game from scratch
void startFromScratch(Player*& player) {
    Trainer::buildTrainers();
    //debugCheckpoint("Built trainers");
    WorldObject::buildWorldObjects();
    player = new Player("Purple");
    
    // Create global Gui instance for new game
    g_gui = new Gui(player);
    std::string name = g_gui->promptText("Welcome to Pokemon Purple!\nWhat is your name?");
    if (!name.empty()) {
        name.erase(std::remove(name.begin(), name.end(), ','), name.end());
        if (name.empty()) {
            player->name = "Purple";
        } else {
            player->name = name.size() < 11 ? name : name.substr(0, 10);
        }
    }
    
    // Handle starter selection
    bool starterChosen = false;
    u16 bottomWidth = 256;
    u16 bottomHeight = 192;
    u16 topWidth = 256;
    u16 topHeight = 192;
    
    g_gui->fillRectangle(bottom_fb, BLACK, 0, 0, bottomWidth, bottomHeight);
    g_gui->fillRectangle(top_fb, WHITE, 0, 0, topWidth, topHeight);
    
    // Draw starter selection prompt on top screen
    drawText("Which starter will you choose?", 10, 10, BLACK, top_fb);
    
    // Draw battler sprites on bottom screen in triangle arrangement
    // Top sprite: Bulbasaur (index 1) at x=115, y=20
    drawBattler(bottom_fb, 1, 115, 20, false);
    drawBattler(bottom_fb, 4, 25, 130, false);
    drawBattler(bottom_fb, 7, 205, 130, false);
    copyBuffers();
    while (!starterChosen) {
        swiWaitForVBlank();
         
        // Handle input
        scanKeys();
        u32 kDown = keysDown();
         
        if (kDown & KEY_TOUCH) {
            // Get touch coordinates directly
            touchPosition touch;
            touchRead(&touch);
            
            // Bottom screen is 320x240, arrange 90x90 sprites in triangle
            // Top sprite: centered at x=115, y=20
            // Bottom left: x=25, y=130
            // Bottom right: x=205, y=130
            if (touch.px >= 115 && touch.px < 205 && 
                touch.py >= 20 && touch.py < 110) {
                // Top sprite - battler_1 (Bulbasaur)
                Battler* bulbasaur = new Battler(1, &Monster::MONSTERS[1]);
                player->give(bulbasaur);
                starterChosen = true;
            }
            else if (touch.px >= 25 && touch.px < 115 && 
                     touch.py >= 130 && touch.py < 220) {
                // Bottom left - battler_4 (Charmander)
                Battler* charmander = new Battler(1, &Monster::MONSTERS[4]);
                player->give(charmander);
                starterChosen = true;
            }
            else if (touch.px >= 205 && touch.px < 295 && 
                     touch.py >= 130 && touch.py < 220) {
                // Bottom right - battler_7 (Squirtle)
                Battler* squirtle = new Battler(1, &Monster::MONSTERS[7]);
                player->give(squirtle);
                starterChosen = true;
            }
        }
    }
    //debugCheckpoint(player->team[0]->nickname.c_str());
}

int main(int argc, char **argv) {
    //consoleDebugInit(DebugDevice_NOCASH);
    initGraphics();
    initializeFont();
    if(!fatInitDefault())
    {
        debugCheckpoint("SKINNY!");
        return 1;
    }
    if (!nitroFSInit(NULL))
    {
        debugCheckpoint("NOTRO!");
        return 1;
    }
    // Initialize sprite map data
    initializeSprites();
    buildGameData();
    //debugCheckpoint(("His name is "+Monster::MONSTERS[0].name).c_str());
    Player *player = nullptr;
    std::ifstream saveFile("fat:/save.txt");
    if (saveFile.is_open()) {
        std::stringstream buffer;
        buffer << saveFile.rdbuf();
        startFromSave(buffer, player);
        saveFile.close();
    } else {
        startFromScratch(player);
    }
    g_gui->playGame();
    return 0;
}
