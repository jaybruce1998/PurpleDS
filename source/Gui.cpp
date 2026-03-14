#include "Gui.h"
#include "BattleState.h"
#include "Blocker.h"
#include "FlyLocation.h"
#include "GameConfig.h"
#include "Item.h"
#include "MartItem.h"
#include "Monster.h"
#include "Move.h"
#include "Npc.h"
#include "PokeMap.h"
#include "Player.h"
#include "Trainer.h"
#include "Warp.h"
#include "WorldObject.h"
#include "Utils.h"
#include "Player.h"
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <sstream>
#include <nds.h>
#include "sprites_data.hpp"
#include "battlers_data.h"
#include "tiles_data_compressed.h"
#include "tile_decompress.h"
#include "font_data.hpp"
#include "transparency_color.h"

static Item *FAKE_ID = nullptr;
static Item *LIFT_KEY = nullptr;
static Item *AMULET_COIN = nullptr;
static Move *CUT = nullptr;
static Move *FLY = nullptr;
static Move *SURF = nullptr;
static Move *STRENGTH = nullptr;

#define NUM_STEP_FRAMES 4
#define BUMP_STEP_FRAMES (NUM_STEP_FRAMES * 2)
#define END std::string::npos

static const std::vector<int> HIDEOUT_Y = {0, 18, 18, 0, 14};
static const std::vector<int> SILPH_X = {0, 20, 20, 20, 20, 20, 18, 18, 18, 18, 12, 13};
static const std::vector<int> CATCH_RATES = {
    1,  45, 45, 45, 45, 45, 45, 45, 45, 45, 255, 120, 45, 255, 120, 45, 255, 120, 45, 255, 127, 255, 90, 255, 90,
    190, 75, 255, 90, 235, 120, 45, 235, 120, 45, 150, 25, 190, 75, 170, 50, 255, 90, 255, 120, 45, 190, 75, 190, 75,
    255, 50, 255, 90, 190, 75, 190, 75, 190, 75, 255, 120, 45, 200, 100, 50, 180, 90, 45, 255, 120, 45, 190, 60, 255, 120,
    45, 190, 60, 190, 75, 190, 60, 45, 190, 45, 190, 75, 190, 75, 190, 60, 190, 90, 45, 45, 190, 75, 225, 60, 190, 60, 90,
    45, 190, 75, 45, 45, 45, 190, 60, 120, 60, 30, 45, 45, 225, 75, 225, 60, 225, 60, 45, 45, 45, 45, 45, 45, 45, 255, 45,
    45, 35, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 25, 3,  3,  3,  45, 27, 9,  3,  2};

bool Gui::canMap = true;
bool Gui::rightClicked = false;
bool Gui::inMenu = false;
bool Gui::flying = false;
bool Gui::inside = false;
bool Gui::surfing = false;
bool Gui::spacebar = false;

int Gui::clickedChoice = -1;
std::string Gui::currentLoc;
Gui* g_gui = nullptr;

u16 top_fb[256*192];
u16 bottom_fb[256*192];
u16* top_buffer;
u16* bottom_buffer;

// Screen dimensions
#define WIDTH 256
#define HEIGHT 192
void initGraphics() {
    videoSetMode(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    videoSetModeSub(MODE_5_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    
    int top_bg = bgInit(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    int bottom_bg = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    top_buffer = (u16*)bgGetGfxPtr(top_bg);
    bottom_buffer = (u16*)bgGetGfxPtr(bottom_bg);
    initGrayscaleLUT();
}

// Precomputed RGB15 grayscale lookup table (256 values)
static u16 grayscaleLUT[256];

// Initialize grayscale LUT
void initGrayscaleLUT() {
    for (int i = 0; i < 256; i++) {
        // Convert 8-bit grayscale to 5-bit per channel RGB15
        u8 gray5 = i >> 3; // Convert 8-bit to 5-bit
        grayscaleLUT[i] = (gray5) | (gray5 << 5) | (gray5 << 10) | BIT(15);
    }
}

// Copy temp buffer to actual framebuffer
void copyBuffers() {
    memcpy(top_buffer, top_fb, 256*192*2);
    memcpy(bottom_buffer, bottom_fb, 256*192*2);
}

// Fill a rectangle with a solid color (DS version)
void Gui::fillRectangle(u16* fb, u16 color, int x, int y, int width, int height) {
    // Clamp coordinates to framebuffer bounds
    int startX = std::max(0, x);
    int startY = std::max(0, y);
    int endX = std::min(256, x + width);
    int endY = std::min(192, y + height);
    
    for (int py = startY; py < endY; py++) {
        for (int px = startX; px < endX; px++) {
            fb[py * 256 + px] = color;
        }
    }
}

// Set a single pixel
void setPixel(u16* fb, int x, int y, u8 r, u8 g, u8 b, int width) {
    u16 color = RGB15(r>>3, g>>3, b>>3) | BIT(15);
    fb[y * width + x] = color;
}

// Draw a battler to the frame buffer with transparency
void drawBattler(u16* fb, int battlerIndex, int screenX, int screenY, bool player) {
    // Handle Missingno (index 0) as special case
    if (battlerIndex==0) {
        // Generate random Missingno pixels
        for (int y = 0; y < 60; y++) {
            for (int x = 0; x < 60; x++) {
                int pos = (screenY + y) * 256 + (screenX + x);
                u16 pixel = rand() | BIT(15);
                fb[pos] = pixel;
            }
        }
        return;
    }
    
    // Regular battler - get from array
    const u8* battler = BATTLER_DATA[battlerIndex-1];
    
    // Draw 60x60 battler
    if(player)
    {
        //draw from right to left (horizontal flip)
        for (int y = 0; y < 60; y++) {
            for (int x = 0; x < 60; x++) {
                int pos = (screenY + y) * 256 + (screenX + x);
                u16 pixel = battlerColors[battler[y * 60 + (59 - x)]];
                
                if (pixel != TRANSPARENCY_RGB15) {
                    fb[pos] = pixel;
                }
            }
        }
    }
    else
        for (int y = 0; y < 60; y++) {
            for (int x = 0; x < 60; x++) {
                int pos = (screenY + y) * 256 + (screenX + x);
                u16 pixel = battlerColors[battler[y * 60 + x]];
                
                if (pixel != TRANSPARENCY_RGB15) {
                    fb[pos] = pixel;
                }
            }
        }
}

// Draw a tile to bottom_fb
void drawTile(int tileId, int screenX, int screenY) {
    const u8* compressed_data = TILE_DATA_COMPRESSED[tileId];
    for (int y = std::max(0, -screenY), startX=std::max(0, -screenX), x, sy, sx; y < 16; y++) {
        sy=screenY+y;
        if(sy>191)
            return;
        sy*=256;
        for (x = startX; x < 16; x++) {
            sx=screenX+x;
            if(sx>255)
                break;
            // Get pixel from compressed data
            u8 pixel_value = get_pixel(compressed_data, x, y);
            top_fb[sy + sx] = grayscaleLUT[pixel_value];
        }
    }
}

// Draw a sprite to top_fb with transparency
void drawSprite(const u8* sprite, int screenX, int screenY) {
    // Draw 16x16 sprite with transparency
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            if (screenX + x < 256 && screenY + y < 192) {
                int pos = (screenY + y) * 256 + (screenX + x);
                u16 pixel = spriteColors[sprite[y * 16 + x]];
                
                // Skip transparent pixels (cyan color)
                if (pixel != TRANSPARENCY_RGB15) {
                    top_fb[pos] = pixel;
                }
            }
        }
    }
}

void Gui::stopMovement() {
    pressedKeys = 0;  // Clear all direction bits
    heldDirection = Direction::NONE;
    stepPhase = StepPhase::NONE;
    phaseFrame = 0;
    offsetX = 0;
    offsetY = 0;
}

Gui::Gui(Player *player) : player(player) {
    battling = false;
    autoBattle = false;
    currentMenu = 0; // Start with Inventory
    choosingFromLongList = false;
    longArrHeader = "";
    setup();
}

Gui::Gui(Player *player, const std::string &saveInfo)
    : player(player) {
    battling = false;
    autoBattle = false;
    currentMenu = 0; // Start with Inventory
    choosingFromLongList = false;
    longArrHeader = "";
    applySaveInfo(saveInfo);
    setup();
}

void Gui::applySaveInfo(const std::string &saveInfo) {
    if (saveInfo.empty()) {
        return;
    }
    std::vector<std::string> parts = utils::split(saveInfo, ',');
    if (parts.size() < 10) {
        return;
    }
    mapName = parts[0];
    try {
        playerX = std::stoi(parts[1]);
        playerY = std::stoi(parts[2]);
        facing = static_cast<Gui::Direction>(std::stoi(parts[3]));
        repelSteps = std::stoi(parts[4]);
    } catch (...) {
    }
    if (parts.size() >= 6) {
        auto it = PokeMap::POKEMAPS.find(parts[5]);
        if (it != PokeMap::POKEMAPS.end()) {
            lastHeal = it->second;
        }
    }
    if (player) {
        if (parts.size() >= 7) {
            player->ballin = (parts[6] == "1");
        }
        if (parts.size() >= 8) {
            try {
                player->numCaught = std::stoi(parts[7]);
            } catch (...) {
            }
        }
        if (parts.size() >= 9) {
            try {
                player->money = std::stoi(parts[8]);
            } catch (...) {
            }
        }
        if (parts.size() >= 10) {
            player->name = parts[9];
        }
    }
}

void Gui::setup() {
    
    if (!FAKE_ID) {
        FAKE_ID = Item::getItem("Fake ID");
        LIFT_KEY = Item::getItem("Lift Key");
        AMULET_COIN = Item::getItem("Amulet Coin");
        CUT = Move::getMove("Cut");
        FLY = Move::getMove("Fly");
        SURF = Move::getMove("Surf");
        STRENGTH = Move::getMove("Strength");
    }
    pm = PokeMap::POKEMAPS[mapName];
    if (!lastHeal) {
        lastHeal = PokeMap::POKEMAPS["RedsHouse1F"];
    }
    
    // Skip SDL texture loading - assets are already compiled as C code
    // loadTileImages(); // REMOVED - tiles are in all_tiles_data.c
    // loadPlayerSprites(); // REMOVED - sprites are in all_sprites_complete.cpp
    loadMap(pm);
    
    // Use existing C asset data for frames
    danceFrames.resize(10, nullptr);
    for (int i = 0; i < 10; i++) {
        auto it = SPRITE_DATA.find("COOLTRAINER_F_" + std::to_string(i));
        if (it != SPRITE_DATA.end()) {
            // Convert sprite data to SDL_Texture format for compatibility
            // For now, set to nullptr since we're not using SDL textures
            danceFrames[i] = nullptr;
        }
    }
    
    // oakFrames - use OAK sprites from ALL_SPRITES_MAP  
    oakFrames.resize(10, nullptr);
    for (int i = 0; i < 10; i++) {
        auto it = SPRITE_DATA.find("OAK_" + std::to_string(i));
        if (it != SPRITE_DATA.end()) {
            oakFrames[i] = nullptr; // Set to nullptr for 3DS compatibility
        }
    }
    
    // monsterFrames - use MONSTER sprites from ALL_SPRITES_MAP
    monsterFrames.resize(3, nullptr);
    for (int i = 0; i < 3; i++) {
        auto it = SPRITE_DATA.find("MONSTER_" + std::to_string(i));
        if (it != SPRITE_DATA.end()) {
            monsterFrames[i] = nullptr; // Set to nullptr for 3DS compatibility
        }
    }

    // Skip SDL-specific code since renderer is nullptr for 3DS
    mattNpc = PokeMap::POKEMAPS["Route24"]->npcs[0 * PokeMap::POKEMAPS["Route24"]->getGridCols() + 19];
    oakNpc = PokeMap::POKEMAPS["FuchsiaCity"]->npcs[6 * PokeMap::POKEMAPS["FuchsiaCity"]->getGridCols() + 13];
    monsterTrainer = nullptr;
    auto it = PokeMap::POKEMAPS.find("FuchsiaCity");
    if (it != PokeMap::POKEMAPS.end() && it->second) {
        PokeMap *fm = it->second;
        int monsterIndex = 5 * fm->getGridCols() + 13;
        monsterTrainer = fm->trainers[monsterIndex];
    }

    if (player) {
        for (auto &kv : PokeMap::POKEMAPS) {
            PokeMap *m = kv.second;
            if (!m) {
                continue;
            }
            int totalSize = m->getGridRows() * m->getGridCols();
            for (int i = 0; i < totalSize; i++) {
                Blocker *b = dynamic_cast<Blocker *>(m->npcs[i]);
                if (!b) {
                    continue;
                }
                bool missingItem = false;
                if (b->item != nullptr) {
                    if (b->item->name == "Master Ball") {
                        missingItem = player->ballin;
                    } else {
                        missingItem = !player->hasItem(b->item);
                    }
                }
                bool missingMove = (b->move != nullptr && !player->hasMove(b->move));
                bool missingBadge = (b->numBadges >= 0 && (b->numBadges >= 9 ||
                                                         !player->leadersBeaten[b->numBadges]));

                if (!missingItem && !missingMove && !missingBadge) {
                    m->npcs[i] = nullptr;
                    delete b;
                }
            }
        }
    }
}
void Gui::renderBattle() {
    if (battling) {
        auto drawFilledRainbowStar = [&](int cx, int cy, int outerRadius, int points) {
            if (points < 2) return;
            int innerRadius = std::max(1, outerRadius * 2 / 5);
            
            // Draw rainbow colored lines from center to star points
            for (int i = 0; i < points * 2; i++) {
                double angle = (static_cast<double>(i) * M_PI / static_cast<double>(points)) - (M_PI / 2.0);
                int r = (i % 2 == 0) ? outerRadius : innerRadius;
                int x = cx + static_cast<int>(std::round(std::cos(angle) * r));
                int y = cy + static_cast<int>(std::round(std::sin(angle) * r));
                
                // Calculate rainbow color
                float t = static_cast<float>(i) / static_cast<float>(points * 2);
                float h = 360.0f * t;
                float s = 1.0f;
                float v = 1.0f;
                
                // HSV to RGB conversion
                h = std::fmod(h, 360.0f);
                if (h < 0) h += 360.0f;
                float c = v * s;
                float x_val = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
                float m = v - c;
                float r_val = 0.0f, g_val = 0.0f, b_val = 0.0f;
                
                if (h < 60.0f) { r_val = c; g_val = x_val; }
                else if (h < 120.0f) { r_val = x_val; g_val = c; }
                else if (h < 180.0f) { g_val = c; b_val = x_val; }
                else if (h < 240.0f) { g_val = x_val; b_val = c; }
                else if (h < 300.0f) { r_val = x_val; b_val = c; }
                else { r_val = c; b_val = x_val; }
                
                // Draw line from center to point
                for (int j = 0; j < r; j++) {
                    float t_line = static_cast<float>(j) / static_cast<float>(r);
                    int px = cx + static_cast<int>((x - cx) * t_line);
                    int py = cy + static_cast<int>((y - cy) * t_line);
                    
                    u8 red = static_cast<u8>(std::round((r_val + m) * 255.0f));
                    u8 green = static_cast<u8>(std::round((g_val + m) * 255.0f));
                    u8 blue = static_cast<u8>(std::round((b_val + m) * 255.0f));
                    
                    setPixel(top_fb, px, py, red, green, blue, WIDTH);
                }
            }
        };
        
        auto drawBattlerPanel = [&](int x, int y, Battler *b) {
            fillRectangle(top_fb, WHITE, x+10, y+8, 100, 56);
            // drawRectOutline(top_fb, x, y, y + 124, x + 150, 0, 0, 0, WIDTH);
            // Draw nickname (rainbow if shiny)
            if (b->shiny) {
                //TODO: verify
                drawRainbowText(top_fb, b->nickname, x + 10, y + 8);
            } else {
                drawText(b->nickname.c_str(), x + 10, y + 8, BLACK, top_fb);
            }
            
            // Draw level
            std::string levelText = "Level " + std::to_string(b->level);
            drawText(levelText.c_str(), x + 10, y + 16, BLACK, top_fb);
            
            // Draw status if not empty
            if (!b->status.empty()) {
                drawText(b->status.c_str(), x + 10, y + 24, BLACK, top_fb);
            }
            
            // Draw HP
            std::string hpText = "HP: " + std::to_string(b->hp) + "/" + std::to_string(b->mhp);
            drawText(hpText.c_str(), x + 10, y + 40, BLACK, top_fb);
            
            // Draw HP bar
            int percent = b->mhp <= 0 ? 0 : static_cast<int>((b->hp / static_cast<float>(b->mhp)) * 100.0f);
            percent = std::max(0, std::min(100, percent));
            
            // Bar background - using fillRectangle with working y, x, height, width pattern
            fillRectangle(top_fb, RGB15(4, 4, 4) | BIT(15), x + 10, y + 48, 100, 16);
            
            // Bar foreground (color based on HP percentage)
            u8 barR, barG, barB;
            if (percent <= 33) { barR = 200; barG = 0; barB = 0; }
            else if (percent <= 66) { barR = 200; barG = 200; barB = 0; }
            else { barR = 0; barG = 200; barB = 0; }
            fillRectangle(top_fb, RGB15(barR, barG, barB) | BIT(15), x + 10, y + 48, 100 * percent / 100, 16);
        };
        
        // Draw battler panels
        drawBattlerPanel(0, 20, playerState->monster);
        drawBattlerPanel(136, 20, enemyState->monster);
        
        // Draw battler sprites
        int pX = 32;
        int eX = 154;
        int y = 100;
        int pdn=playerState->monster->dexNum;
        int edn=enemyState->monster->dexNum;
        // Player battler
        drawBattler(top_fb, pdn, pX, y, true);
        if (playerState->monster->shiny) {
            drawFilledRainbowStar(pX + 32, y + 32, 44, 5);
            drawFilledRainbowStar(pX + 96, y + 32, 36, 5);
        }
        
        // Enemy battler
        drawBattler(top_fb, edn, eX, y, false);
        if (enemyState->monster->shiny) {
            drawFilledRainbowStar(eX + 96, y + 32, 44, 5);
            drawFilledRainbowStar(eX + 160, y + 32, 36, 5);
        }
    }
}
void Gui::print(const std::string &s) {
    // Clear bottom screen
    fillRectangle(bottom_fb, WHITE, 0, 0, 256, 192);
    drawText(s.c_str(), 0, 0, BLACK, bottom_fb);
    renderBattle();
    swiWaitForVBlank();
    copyBuffers();
    
    while(true) {
        // Listen for A, B, X, Y, L button, R button, or tap anywhere on screen, break from the loop
        scanKeys();
        u32 kDown = keysDown();
        
        if (kDown & (KEY_A | KEY_B | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_TOUCH)) {
            break;
        }
        
        // Vsync
        swiWaitForVBlank();
        
        // If battling && autobattling && frames%3==0, break
        static u8 localFrames = 0;
        if (battling && autoBattle && (localFrames++ % 3 == 0)) {
            break;
        }
    }
}

std::string Gui::promptText(const std::string &prompt) {
    std::string input = "";
    char lastChar = 'A';
    bool uppercase = true;
    
    while (true) {
        fillRectangle(top_fb, WHITE, 0, 0, 256, 192);
        fillRectangle(bottom_fb, WHITE, 0, 0, 256, 192);
        
        // Build top screen text: prompt + instructions + input
        std::string topText = prompt + "\n\nSELECT: Shift case\nSTART: Confirm\nB: Backspace\nA: Repeat last\n\nInput: " + input;
        
        // Draw all top screen text at once
        drawText(topText.c_str(), 10, 10, BLACK, top_fb);
        
        // Build keyboard string for bottom screen with formatting
        const char* keyboard = uppercase ? 
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()_+{}|:<>?[]\\;'\",./" :
            "abcdefghijklmnopqrstuvwxyz1234567890-=~`";
        
        // Format keyboard with spaces and newlines every 10 characters
        std::string keyboardText = "";
        for (size_t i = 0; i < strlen(keyboard); i++) {
            keyboardText += keyboard[i];
            keyboardText += " "; // Add space between characters
            if ((i + 1) % 10 == 0) {
                keyboardText += "\n"; // Newline every 10 characters
            }
        }
        
        // Draw keyboard on bottom screen
        drawText(keyboardText.c_str(), 10, 20, BLACK, bottom_fb);
        // Swap buffers

        swiWaitForVBlank();
        copyBuffers();
        
        // Handle input
        scanKeys();
        u32 kDown = keysDown();
        
        if (kDown & KEY_START) {
            break;  // Return current input
        }
        else if (kDown & KEY_SELECT) {
            uppercase = !uppercase;  // Toggle case
        }
        else if (kDown & KEY_B) {
            if (!input.empty()) {
                input.pop_back();  // Backspace
                lastChar = input.empty() ? 'A' : input.back();
            }
        }
        else if (kDown & KEY_A) {
            if (input.length() < 10) {
                input += lastChar;  // Repeat last character or A if none
            }
        }
        else if (kDown & KEY_TOUCH) {
            touchPosition touch;
            touchRead(&touch);
            
            // Calculate which key was touched
            // Each character takes 16 pixels (8 for char + 8 for space)
            // 10 characters per line = 160 pixels total
            // Line height is 10 pixels (8 glyph + 2 spacing)
            int charsPerLine = 10;
            int charWidth = 16; // 8 for character + 8 for space
            int charHeight = 10; // 8 glyph height + 2 line spacing
            
            int line = (touch.py - 20) / charHeight;
            int col = touch.px / charWidth;
            size_t keyIndex = line * charsPerLine + col;
            
            if (keyIndex < strlen(keyboard) && input.length() < 10) {
                input += keyboard[keyIndex];
                lastChar = keyboard[keyIndex];
            }
        }
    }
    
    return input;
}

u8 Gui::promptNumber(const std::string &prompt) {
    std::string input = "";
    char lastChar = '0';
    
    while (true) {
        fillRectangle(top_fb, WHITE, 0, 0, 256, 192);
        fillRectangle(bottom_fb, WHITE, 0, 0, 256, 192);
        
        // Build top screen text: prompt + instructions + input
        std::string topText = prompt + "\n\nSTART: Confirm\nB: Backspace\nA: Repeat last\n\nInput: " + input;
        
        // Draw all top screen text at once
        drawText(topText.c_str(), 10, 10, BLACK, top_fb);
        
        // Build number pad for bottom screen with formatting
        const char* numpad = "1234567890";
        
        // Format numpad with spaces and newlines every 5 characters
        std::string numpadText = "";
        for (size_t i = 0; i < strlen(numpad); i++) {
            numpadText += numpad[i];
            numpadText += " "; // Add space between characters
            if ((i + 1) % 5 == 0) {
                numpadText += "\n"; // Newline every 5 characters
            }
        }
        
        // Draw numpad on bottom screen
        drawText(numpadText.c_str(), 10, 20, BLACK, bottom_fb);
        
        // Swap buffers

        swiWaitForVBlank();
        copyBuffers();
        
        // Handle input
        scanKeys();
        u32 kDown = keysDown();
        
        if (kDown & KEY_START) {
            break;  // Return current input
        }
        else if (kDown & KEY_B) {
            if (!input.empty()) {
                input.pop_back();  // Backspace
                lastChar = input.empty() ? '0' : input.back();
            }
        }
        else if (kDown & KEY_A) {
            if (input.length() < 10) {
                input += lastChar;  // Repeat last character or 0 if none
            }
        }
        else if (kDown & KEY_TOUCH) {
            touchPosition touch;
            touchRead(&touch);
            
            // Calculate which key was touched
            // Each character takes 16 pixels (8 for char + 8 for space)
            // 5 characters per line = 80 pixels total
            // Line height is 10 pixels (8 glyph + 2 spacing)
            int charsPerLine = 5;
            int charWidth = 16; // 8 for character + 8 for space
            int charHeight = 10; // 8 glyph height + 2 line spacing
            
            int line = (touch.py - 20) / charHeight;
            int col = touch.px / charWidth;
            size_t keyIndex = line * charsPerLine + col;
            
            if (keyIndex < strlen(numpad) && input.length() < 10) {
                input += numpad[keyIndex];
                lastChar = numpad[keyIndex];
            }
        }
    }
    if(input=="")
    {
        print("Keep those obscenities to yourself, boy!");
        return 0;
    }
    int n=std::stoi(input);
    if(n>255)
    {
        print("Keep those obscenities to yourself, boy!");
        return 0;
    }
    return static_cast<u8>(n);
}

u8 Gui::waitForChoice(u8 maxChoice) {
    touchPosition touch;
    u32 kDown;
    renderBattle();
    displayLongArr();
    while (true) {
        swiWaitForVBlank();
        // Scan DS input
        scanKeys();
        kDown = keysDown();
        
        if (kDown & KEY_A) {
            return 0;
        }
        
        if (kDown & KEY_B) {
            return maxChoice;
        }
        
        if (kDown & KEY_TOUCH) {
            touchRead(&touch);
            u8 choice = touch.py / 8;
            // Validate choice is within range
            if (choice < maxChoice) {
                return choice;
            }
        }
    }
}

void Gui::setBattleStates(BattleState *a, BattleState *b) {
    if(playerState)
        delete playerState;
    playerState = a;
    if(enemyState)
        delete enemyState;
    enemyState = b;
    battling = true;
}

void Gui::loadMap(PokeMap *map) {
    if (!map) {
        return;
    }
    pm = map;
    connections[0] = pm->north;
    connOffsets[0] = pm->nOff;
    connections[1] = pm->south;
    connOffsets[1] = pm->sOff;
    connections[2] = pm->west;
    connOffsets[2] = pm->wOff;
    connections[3] = pm->east;
    connOffsets[3] = pm->eOff;
}

void Gui::closeMenus() {
    inMenu = false;
    longArrHeader.clear();
    choosingFromLongList = false;
}

void Gui::displayLongArr()
{
    if(longArrHeader!="")
    {
        fillRectangle(top_fb, WHITE, 8, 180, 238, 16);
        drawText(longArrHeader.c_str(), 16, 184, BLACK, top_fb);
    }
    fillRectangle(bottom_fb, WHITE, 0, 0, 256, 192);
    for(u8 i=0; i<20&&longArr[i]!=""; i++)
        drawText(longArr[i].c_str(), 0, i*8, BLACK, bottom_fb);
    copyBuffers();
}

void Gui::checkBattler(u16 py)
{
    u8 i = py/8, pp, mpp, kd;
    if (i >= 6 || player->team[i] == nullptr) {
        return;
    }
    Move* m;
    Battler* chosenMon=player->team[i];
    chosenMon->setInformation();
    displayLongArr();
    while(true)
    {
        swiWaitForVBlank();
        scanKeys();
        kd=keysDown();
        if(kd&KEY_B)
            return;
        if(kd&KEY_UP)
        {
            if(!chosenMon->moves[1])
                return;
            m=chosenMon->moves[0];
            pp=chosenMon->pp[0];
            mpp=chosenMon->mpp[0];
            longArr[4]=longArr[13];
            for(i=1; i<4&&chosenMon->moves[i]; i++)
            {
                chosenMon->moves[i-1]=chosenMon->moves[i];
                chosenMon->pp[i-1]=chosenMon->pp[i];
                chosenMon->mpp[i-1]=chosenMon->mpp[i];
                longArr[12+i]=longArr[13+i];
            }
            i--;
            chosenMon->moves[i]=m;
            chosenMon->pp[i]=pp;
            chosenMon->mpp[i]=mpp;
            longArr[13+i]=longArr[4];
            longArr[4]=" ";
            displayLongArr();
        }
        else if(kd&KEY_DOWN)
        {
            if(!chosenMon->moves[1])
                return;
            m=chosenMon->moves[0];
            pp=chosenMon->pp[0];
            mpp=chosenMon->mpp[0];
            longArr[4]=longArr[13];
            chosenMon->moves[0]=chosenMon->moves[1];
            chosenMon->pp[0]=chosenMon->pp[1];
            chosenMon->mpp[0]=chosenMon->mpp[1];
            longArr[13]=longArr[14];
            chosenMon->moves[1]=m;
            chosenMon->pp[1]=pp;
            chosenMon->mpp[1]=mpp;
            longArr[14]=longArr[4];
            longArr[4]=" ";
            displayLongArr();
        }
    }
}

void Gui::setPartyStrings(Battler** party) {
    longArrHeader="Party";
    for (int i = 0; i < 6; i++) {
        if (party[i] == nullptr) {
            longArr[i] = "";
            return;
        } else {
            longArr[i] = party[i]->toString();
        }
    }
    longArr[6]="";
}

void Gui::setBuySellStrings()
{
    longArrHeader="What do you want to do?";
    longArr[0]="Buy";
    longArr[1]="Sell";
    longArr[2]="";
    displayLongArr();
}

void Gui::buy()
{
    touchPosition touch;
    u32 kd;
    u8 n;
    longArrHeader="Money: $" + std::to_string(player->money);
    for(size_t i = 0; i < martItems->size(); i++)
        longArr[i] = (*martItems)[i].toString();
    displayLongArr();
    while(true)
    {
        swiWaitForVBlank();
        scanKeys();
        kd = keysDown();
        if(kd&KEY_B)
            return;
        if(kd & KEY_TOUCH)
        {
            touchRead(&touch);
            kd=touch.py/8;
            if(kd>=martItems->size())
                continue;
            const MartItem &m = (*martItems)[kd];
            if (player->money < m.price) {
                print("You can't afford that!");
                displayLongArr();
                continue;
            }
            if (m.move) {
                if (player->hasMove(m.move)) {
                    print("You already have that TM!");
                } else {
                    player->money -= m.price;
                    player->give(m.move);
                    print("Thank you!");
                }
            }
            else if (m.mon) {
                player->money -= m.price;
                player->give(new Battler(1, m.mon));
                print("Thank you!");
            }
            else {
                n = this->promptNumber("How many?");
                kd = m.price * n;
                if (n==0||player->money < kd) {
                    print("You can't afford that many!");
                    displayLongArr();
                    continue;
                }
                player->money -= kd;
                player->give(m.item, n);
                print("Thank you!");
            }
            longArrHeader="Money: $" + std::to_string(player->money);
            displayLongArr();
        }
    }
}

void Gui::sell()
{
    touchPosition touch;
    u32 kd;
    u8 i;
    Item* it;
    setInventoryStrings();
    displayLongArr();
    while(true)
    {
        swiWaitForVBlank();
        scanKeys();
        kd = keysDown();
        if(kd&KEY_B)
            return;
        if(kd & KEY_TOUCH)
        {
            touchRead(&touch);
            i=touch.py/8;
            if(i<20)
            {
                it=Item::ITEMS[psi*20+i];
                if(it->quantity==0)
                    continue;
                if(it->price<0)
                {
                    print("I REALLY do not want that!");
                    displayLongArr();
                    continue;
                }
                i = this->promptNumber("How many?");
                if(i==0)
                    continue;
                player->sell(it, std::min(static_cast<int>(i), static_cast<int>(it->quantity)));
                print("Thank you!");
                setInventoryStrings();
                displayLongArr();
            }
        }
    }
}

void Gui::buySell()
{
    touchPosition touch;
    u32 kd;
    psi=0;
    setBuySellStrings();
    while(true)
    {
        swiWaitForVBlank();
        scanKeys();
        kd = keysDown();
        if(kd&KEY_B)
            return;
        if(kd & KEY_TOUCH)
        {
            touchRead(&touch);
            kd=touch.py/8;
            if(kd==0)
            {
                buy();
                setBuySellStrings();
            }
            else if(kd==1)
            {
                sell();
                setBuySellStrings();
            }
        }
    }
}

u8 Gui::moveChoice(Battler* b)
{
    touchPosition touch;
    u32 kd;
    u8 i;
    for (i = 0; i < 4; i++) {
        longArr[i] = b->moves[i] == nullptr ? "" : b->moves[i]->name + " (" +
            std::to_string(b->pp[i])+"/"+std::to_string(b->mpp[i]) + ")";
    }
    displayLongArr();
    while(true)
    {
        swiWaitForVBlank();
        scanKeys();
        kd = keysDown();
        if(kd&KEY_A)
            return 0;
        if(kd&KEY_B)
            return 4;
        if(kd & KEY_TOUCH)
        {
            touchRead(&touch);
            i=touch.py/8;
            if(i<4&&b->moves[i])
                return i;
        }
    }
}

bool Gui::useMoveItem(Item* it, std::string n, Battler* chosenMon)
{
    u8 i=moveChoice(chosenMon);
    if(i==4)
        return false;
    std::string name=chosenMon->moves[i]->name;
    if (n == "Ether") {
        if (chosenMon->pp[i] == chosenMon->mpp[i]) {
            print("DO NOT WASTE THAT!");
            return false;
        } else {
            chosenMon->pp[i] = std::min(static_cast<int>(chosenMon->mpp[i]), chosenMon->pp[i] + 10);
            print(name + "'s PP was restored by 10!");
            return player->use(it);
        }
    }
    if (n == "Max Ether") {
        if (chosenMon->pp[i] == chosenMon->mpp[i]) {
            print("DO NOT WASTE THAT!");
            return false;
        } else {
            chosenMon->pp[i] = chosenMon->mpp[i];
            print(name + "'s PP was fully restored!");
            return player->use(it);
        }
    }
    if (n == "PP Up") {
        chosenMon->mpp[i] = static_cast<int>(chosenMon->mpp[i] * 1.2);
        print(name + "'s PP was permanently increased!");
        return player->use(it);
    }
    print((n+"?").c_str());
    return false;
}

bool Gui::usePartyItem(Item* it, std::string n)
{
    touchPosition touch;
    u32 kd;
    u8 i;
    Battler* b;
    setPartyStrings(player->team);
    displayLongArr();
    while(true)
    {
        swiWaitForVBlank();
        scanKeys();
        kd = keysDown();
        if(kd&KEY_A)
        {
            b=player->team[0];
            break;
        }
        if(kd&KEY_B)
            return false;
        if(kd & KEY_TOUCH)
        {
            touchRead(&touch);
            i=touch.py/8;
            if(i>5)
                continue;
            b=player->team[i];
            if(b)
                break;
        }
    }
    if(it->flags&16)
        return useMoveItem(it, n, b);
    if (n == "Antidote") {
        return healStatus(it, b, "POISONED");
    }
    if (n == "Awakening") {
        return healStatus(it, b, "SLEEPING");
    }
    if (n == "Burn Heal") {
        return healStatus(it, b, "BURNED");
    }
    if (n == "Ice Heal") {
        return healStatus(it, b, "FROZEN");
    }
    if (n == "Paralyze Heal") {
        return healStatus(it, b, "PARALYZED");
    }
    if (n == "Full Heal") {
        if (b->status.empty()) {
            print("Quit messing around!");
            return false;
        } else {
            if (playerState && playerState->monster == b) {
                playerState->poisonDamage = 0;
                playerState->sTurns=0;
            }
            b->status.clear();
            print(b->nickname + "'s status was healed!");
            return player->use(it);
        }
    }
    if (n == "Full Restore") {
        if (b->hp == 0) {
            print("No, you'll need something stronger to heal this one...");
            return false;
        } else if (b->hp == b->mhp && b->status.empty()) {
            print("Quit messing around!");
            return false;
        } else {
            b->hp = b->mhp;
            b->status.clear();
            print(b->nickname + " was fully restored!");
            return player->use(it);
        }
    }
    if (n == "Elixir") {
        return useElixir(it, b, 10);
    }
    if (n == "Max Elixir") {
        return useElixir(it, b, 99);
    }
    if (n == "Potion") {
        return healHp(it, b, 20);
    }
    if (n == "Fresh Water") {
        return healHp(it, b, 50);
    }
    if (n == "Super Potion") {
        return healHp(it, b, 50);
    }
    if (n == "Soda Pop") {
        return healHp(it, b, 60);
    }
    if (n == "Hyper Potion") {
        return healHp(it, b, 200);
    }
    if (n == "Lemonade") {
        return healHp(it, b, 80);
    }
    if (n == "Max Potion") {
        return healHp(it, b, 999);
    }
    if (n == "Revive") {
        return useRevive(it, b, b->mhp / 2);
    }
    if (n == "Max Revive") {
        return useRevive(it, b, b->mhp);
    }
    if (n.rfind("Stone")!=END) {
        if(b->useStone(n))
        {
            player->registerBattler(b);
            return player->use(it);
        }
        else
            return false;
    }
    if (n == "Calcium") {
        b->spatkXp += 10;
        b->refreshStats();
        print(b->nickname + " gained special attack experience!");
        return player->use(it);
    }
    if (n == "Carbos") {
        b->spdXp += 10;
        b->refreshStats();
        print(b->nickname + " gained speed experience!");
        return player->use(it);
    }
    if (n == "HP Up") {
        b->hpXp += 10;
        b->refreshStats();
        print(b->nickname + " gained HP experience!");
        return player->use(it);
    }
    if (n == "Iron") {
        b->defXp += 10;
        b->refreshStats();
        print(b->nickname + " gained defense experience!");
        return player->use(it);
    }
    if (n == "Protein") {
        b->atkXp += 10;
        b->refreshStats();
        print(b->nickname + " gained attack experience!");
        return player->use(it);
    }
    if (n == "Zinc") {
        b->spdefXp += 10;
        b->refreshStats();
        print(b->nickname + " gained special defense experience!");
        return player->use(it);
    }
    if(n=="Rare Candy")
    {
	    b->gainXp(player, b->moves, b->mxp-b->xpNeeded(b->level), 0, 0, 0, 0, 0, 0);
        return player->use(it);
    }
    print((n+"?").c_str());
    return false;
}

bool Gui::useItem(u16 py, u8 flag)
{
    if(py>159)
        return false;
    Item* it=Item::ITEMS[psi * 20 + py/8];
    if(it->quantity==0)
        return false;
    if((it->flags&flag)!=flag)
    {
        print("This cannot be used now.");
        return false;
    }
    std::string n=it->name;
    if(it->flags&8)
        return usePartyItem(it, n);
    if(n == "Repel") {
        return useRepel(it, 101);
    }
    if(n == "Super Repel") {
        return useRepel(it, 201);
    }
    if(n == "Max Repel") {
        return useRepel(it, 251);
    }
    if(n == "Old Rod" || n == "Good Rod" || n == "Super Rod") {
        u8 nx = playerX;
        u8 ny = playerY;
        switch (facing) {
            case Direction::NONE:
                return false;
            case Direction::SOUTH:
                ny++;
                break;
            case Direction::NORTH:
                ny--;
                break;
            case Direction::WEST:
                nx--;
                break;
            case Direction::EAST:
                nx++;
                break;
        }
        if (ny >= pm->getGridRows() || nx >= pm->getGridCols()) {
            print("What exactly do you expect to find in the void?");
            return false;
        }
        if (pm->getTypeValue(ny, nx) != 4) {
            print("Maybe I should fish in the water...");
            return false;
        }
        if(wildMon)
            delete wildMon;
        wildMon = pm->getRandomEncounter(n);
        if (wildMon == nullptr) {
            print("Huh, no fish... I should try somewhere else!");
            return false;
        }
        int result = BattleState::wildBattle(player->team, wildMon);
        if (result < 0) {
            blackout();
        } else {
            player->money += result * (player->hasItem(AMULET_COIN) ? 2 : 1);
        }
        return true;
    }
    if(n == "Escape Rope") {
        Trainer::addEliteFour();
        loadMap(lastHeal);
        playerX = lastHeal->healX;
        playerY = lastHeal->healY;
        print("You climbed back to the last heal spot!");
        return player->use(it);
    }
    if (n == "X Accuracy") {
        if(playerState->accStage==6)
        {
            print(playerState->monster->nickname+" already has max accuracy!");
            return false;
        }
        playerState->accStage++;
        print(playerState->monster->nickname + "'s accuracy rose by 1 stage!");
        return player->use(it);
        
    }
    if (n == "X Attack") {
        if(playerState->atkStage==6)
        {
            print(playerState->monster->nickname+" already has max attack!");
            return false;
        }
        playerState->atkStage++;
        print(playerState->monster->nickname + "'s attack rose by 1 stage!");
        return player->use(it);
    }
    if (n == "X Defend") {
        if(playerState->defStage==6)
        {
            print(playerState->monster->nickname+" already has max defense!");
            return false;
        }
        playerState->defStage++;
        print(playerState->monster->nickname + "'s defense rose by 1 stage!");
        return player->use(it);
    }
    if (n == "X Special Attack") {
        if(playerState->spatkStage==6)
        {
            print(playerState->monster->nickname+" already has max special attack!");
            return false;
        }
        playerState->spatkStage++;
        print(playerState->monster->nickname + "'s special attack rose by 1 stage!");
        return player->use(it);
    }
    if (n == "X Special Defend") {
        if(playerState->spdefStage==6)
        {
            print(playerState->monster->nickname+" already has max special defense!");
            return false;
        }
        playerState->spdefStage++;
        print(playerState->monster->nickname + "'s special defense rose by 1 stage!");
        return player->use(it);
    }
    if (n == "X Speed") {
        if(playerState->spdStage==6)
        {
            print(playerState->monster->nickname+" already has max speed!");
            return false;
        }
        playerState->spdStage++;
        print(playerState->monster->nickname + "'s speed rose by 1 stage!");
        return player->use(it);
    }
    if (n == "Dire Hit") {
        playerState->critMul *= 4;
        print(playerState->monster->nickname + " is now 4 times more likely to get a critical hit!");
        return player->use(it);
    }
    if (n == "Guard Spec.") {
        if (playerState->canLower) {
            playerState->canLower = false;
            print(playerState->monster->nickname + "'s stats can no longer be lowered!");
            return player->use(it);
        }
        print(playerState->monster->nickname+"'s stats are already protected.");
        return false;
    }
    if (n == "Poke Doll") {
        battling=false;
        return player->use(it);
    }
    if (n == "PokeBall") {
        return catchMon(it, n, 1.0);
    }
    if (n == "Great Ball") {
        return catchMon(it, n, 1.5);
    }
    if (n == "Ultra Ball") {
        return catchMon(it, n, 2.0);
    }
    if (n == "Master Ball") {
        return catchMon(it, n, 100000);
    }
    print((n+"?").c_str());
    return false;
}

bool Gui::useBattleItem(u8 flag)
{
    touchPosition touch;
    u32 kd;
    psi=0;
    setInventoryStrings();
    displayLongArr();
    while(true)
    {
        swiWaitForVBlank();
        scanKeys();
        kd = keysDown();
        
        if(kd & (KEY_B|KEY_X))
            return false;
        
        if(kd&(KEY_L|KEY_LEFT))
        {
            psi=std::max(psi-1, 0);
            setInventoryStrings();
            displayLongArr();
        }
        if(kd&(KEY_R|KEY_RIGHT))
        {
            psi++;
            setInventoryStrings();
            displayLongArr();
        }
        if(kd & KEY_TOUCH)
        {
            touchRead(&touch);
            if(useItem(touch.py, flag))
                return true;
            setInventoryStrings();
            displayLongArr();
        }
    }
}

void Gui::teach(u16 py, Move* tm)
{
    u8 i=py/8;
    if(i>5||!player->team[i])
        return;
    Battler* chosenMon=player->team[i];
    const std::unordered_set<Move*>& learnable = Monster::MONSTERS[chosenMon->dexNum].learnable;
    if(learnable.find(tm) == learnable.end())
    {
        print(chosenMon->nickname + " cannot learn " + tm->name + ".");
        return;
    }
    for (i = 0; i < 4; i++) {
        if (!chosenMon->moves[i]) {
            chosenMon->moves[i] = tm;
            chosenMon->pp[i] = tm->pp;
            chosenMon->mpp[i] = tm->pp;
            print(chosenMon->nickname + " learned " + tm->name + "!");
            return;
        }
        if (chosenMon->moves[i] == tm) {
            print(chosenMon->nickname + " already knows " + tm->name + "!");
            return;
        }
    }
    chosenMon->learn(chosenMon->moves, tm);
}

void Gui::useTm(u16 py)
{
    u8 idx = psi * 20 + py/8;
    if (idx >= 58 || !player->tmHms[idx]) {
        return;
    }
    Move* tm = player->tmHms[idx];
    touchPosition touch;
    u32 kd;
    setPartyStrings(player->team);
    displayLongArr();
    while(true)
    {
        swiWaitForVBlank();
        scanKeys();
        kd = keysDown();
        if(kd&KEY_A)
        {
            teach(0, tm);
            setPartyStrings(player->team);
            displayLongArr();
        }
        if(kd&KEY_B)
            return;
        if(kd & KEY_TOUCH)
        {
            touchRead(&touch);
            teach(touch.py, tm);
            setPartyStrings(player->team);
            displayLongArr();
        }
    }
}

void Gui::setInventoryStrings()
{
    if(psi==4)
    {
        psi=3;
        return;
    }
    u8 s=psi*20, n;
    longArrHeader = "Items";
    if(Item::ITEMS[s]->quantity==0)
    {
        if(psi>0)
            psi--;
        return;
    }
    for(u8 i=0; i<20; i++)
    {
        n=s+i;
        if(Item::ITEMS[n]->quantity==0)
        {
            longArr[i]="";
            return;
        }
        longArr[i]=Item::ITEMS[n]->toString();
    }
}

void Gui::setTMStrings()
{
    if(psi==3)
    {
        psi=2;
        return;
    }
    u8 s=psi*20, n;
    longArrHeader = "TMs";
    if(!player->tmHms[s])
    {
        if(psi>0)
        psi--;
        return;
    }
    for(u8 i=0; i<20; i++)
    {
        n=s+i;
        if(n==58||!player->tmHms[n])
        {
            longArr[i]="";
            return;
        }
        longArr[i]=player->tmHms[n]->name;
    }
}

void Gui::setPokedexStrings()
{
    if(psi==8)
    {
        psi=7;
        return;
    }
    u8 s=psi*20, d;
    longArrHeader = "Pokedex (Caught: " + std::to_string(player->numCaught) + ")";
    for (u8 i = 0; i < 20; i++) {
        d=s+i;
        std::string n = std::to_string(d);
        std::string num = "#"+std::string(3 - static_cast<int>(n.size()), '0') + n+" ";
        if(d==152)
        {
            longArr[i]="";
            return;
        }
        if((d==0||d==151)&&!player->pokedex[d])
            longArr[i]=" ";
        else if(player->pokedex[d])
            longArr[i]=num+Monster::MONSTERS[d].name;
        else
            longArr[i]=num+"?";
    }
}

void Gui::setMenuStrings()
{
    longArr[0]="";
    switch(currentMenu)
    {
        case 0: // Inventory
            setInventoryStrings();
            break;
        case 1: // TMs
            setTMStrings();
            break;
        case 2: // Pokemon
            setPartyStrings(player->team);
            break;
        case 3: // Pokedex
            setPokedexStrings();
            break;
    }
    displayLongArr();
}

void Gui::itemUpDown(u32 kd)
{
    if(Item::ITEMS[psi*20+1]->quantity==0)
        return;
    Item* i=Item::ITEMS[psi*20];
    if(kd & KEY_UP)
    {
        kd=i->quantity;
        i->quantity=0;
        player->removeItem(i);
        player->give(i, kd);
        setInventoryStrings();
        displayLongArr();
        return;
    }
    if(kd & KEY_DOWN)
    {
        Item::ITEMS[psi*20]=Item::ITEMS[psi*20+1];
        Item::ITEMS[psi*20+1]=i;
        setInventoryStrings();
        displayLongArr();
    }
}

void Gui::tmUpDown(u32 kd)
{
    if(!player->tmHms[psi*20+1])
        return;
    Move* m=player->tmHms[psi*20];
    if(kd & KEY_UP)
    {
        for(kd=psi*20+1; kd<57; kd++)
        {
            if(!player->tmHms[kd])
                break;
            player->tmHms[kd-1]=player->tmHms[kd];
        }
        player->tmHms[kd-1]=m;
        setTMStrings();
        displayLongArr();
        return;
    }
    if(kd & KEY_DOWN)
    {
        player->tmHms[psi*20]=player->tmHms[psi*20+1];
        player->tmHms[psi*20+1]=m;
        setTMStrings();
        displayLongArr();
    }
}

void Gui::partyUpDown(u32 kd)
{
    if(!player->team[1])
        return;
    Battler* b=player->team[0];
    if(kd & KEY_UP)
    {
        for(kd=1; kd<6; kd++)
        {
            if(!player->team[kd])
                break;
            player->team[kd-1]=player->team[kd];
        }
        player->team[kd-1]=b;
        setPartyStrings(player->team);
        displayLongArr();
        return;
    }
    if(kd & KEY_DOWN)
    {
        player->team[0]=player->team[1];
        player->team[1]=b;
        setPartyStrings(player->team);
        displayLongArr();
    }
}

void Gui::deposit()
{
    touchPosition touch;
    u32 kd;
    setPartyStrings(player->team);
    displayLongArr();
    while(true)
    {
        swiWaitForVBlank();
        scanKeys();
        kd = keysDown();
        if(kd & KEY_B)
            return;
        partyUpDown(kd);
        if(kd & KEY_TOUCH)
        {
            if(!player->team[1])
            {
                print("You'll never survive in this world without pokemon...");
                return;
            }
            touchRead(&touch);
            kd=touch.py/8;
            if(kd>5||!player->team[kd])
                continue;
            player->pc.push_back(player->team[kd]);
            for(kd++; kd<6&&player->team[kd]; kd++)
                player->team[kd-1]=player->team[kd];
            player->team[kd-1]=nullptr;
            setPartyStrings(player->team);
        }
    }
}

void Gui::setPcStrings()
{
    longArrHeader="Box "+std::to_string(psi+1);
    for(u8 i=0, bi=psi*20; i<20; i++)
    {
        if(i+bi>=player->pc.size())
        {
            longArr[i]="";
            break;
        }
        longArr[i]=player->pc[i+bi]->toString();
    }
    displayLongArr();
}

void Gui::withdraw()
{
    touchPosition touch;
    Battler* b;
    u32 kd;
    u8 i;
    for(i=1; i<6; i++)
        if(!player->team[i])
            break;
    setPcStrings();
    while(true)
    {
        swiWaitForVBlank();
        scanKeys();
        kd = keysDown();
        if(kd & KEY_B)
            return;
        if(kd & (KEY_L|KEY_LEFT))
        {
            if(psi>0)
            {
                psi--;
                setPcStrings();
            }
        }
        if(kd & (KEY_R|KEY_RIGHT))
        {
            if(player->pc.size() > static_cast<size_t>((psi + 1) * 20))
            {
                psi++;
                setPcStrings();
            }
        }
        if(kd & KEY_TOUCH)
        {
            if(i==6)
            {
                print("Your party is already full!");
                return;
            }
            touchRead(&touch);
            kd=touch.py/8+psi*20;
            if(kd >= player->pc.size())
                continue;
            player->team[i++]=player->pc[kd];
            player->pc.erase(player->pc.begin()+kd);
            setPcStrings();
        }
        if(kd & KEY_UP)
        {
            kd=psi*20;
            if(player->pc.size()<kd+1)
                continue;
            b=player->pc[kd];
            player->pc.erase(player->pc.begin()+kd);
            player->pc.emplace_back(b);
            setPcStrings();
        }
        else if(kd & KEY_DOWN)
        {
            kd=psi*20;
            if(player->pc.size()<kd+2)
                continue;
            b=player->pc[kd];
            player->pc[kd]=player->pc[kd+1];
            player->pc[kd+1]=b;
            setPcStrings();
        }
    }
}

void Gui::setDepWith()
{
    longArrHeader="PC";
    longArr[0]="Deposit";
    longArr[1]="Withdraw";
    longArr[2]="";
    displayLongArr();
}

void Gui::depositWithdraw()
{
    touchPosition touch;
    u32 kd;
    psi=0;
    setDepWith();
    while(true)
    {
        swiWaitForVBlank();
        scanKeys();
        kd = keysDown();
        
        if(kd & KEY_B)
            return;
        if(kd & KEY_TOUCH)
        {
            touchRead(&touch);
            switch(touch.py/8)
            {
                case 0:
                    deposit();
                    break;
                case 1:
                    withdraw();
                    break;
            }
            setDepWith();
        }
    }
}

void Gui::openMenu()
{
    touchPosition touch;
    u32 kd;
    psi=0;
    setMenuStrings();
    while(true)
    {
        swiWaitForVBlank();
        scanKeys();
        kd = keysDown();
        
        if(kd & (KEY_B|KEY_X))
            return;
        
        if(kd&KEY_L)
        {
            psi=0;
            currentMenu=(currentMenu+3)%4;
            setMenuStrings();
        }
        if(kd&KEY_R)
        {
            psi=0;
            currentMenu=(currentMenu+1)%4;
            setMenuStrings();
        }

        switch(currentMenu)
        {
            case 0:
                itemUpDown(kd);
                break;
            case 1:
                tmUpDown(kd);
                break;
            case 2:
                partyUpDown(kd);
        }

        if(kd&KEY_LEFT)
        {
            psi=std::max(psi-1, 0);
            setMenuStrings();
        }
        else if(kd&KEY_RIGHT)
        {
            psi++;
            setMenuStrings();
        }
        if(kd & KEY_TOUCH)
        {
            touchRead(&touch);
            switch(currentMenu)
            {
                case 0:
                    useItem(touch.py, 4);
                    setInventoryStrings();
                    break;
                case 1:
                    useTm(touch.py);
                    setTMStrings();
                    break;
                case 2:
                    checkBattler(touch.py);
                    setPartyStrings(player->team);
                    break;
                case 3:
                    return;
            }
            displayLongArr();
        }
    }
}

void Gui::clickMouse(int mouseX, int mouseY, bool rightClick) {
    // Handle flying map touch input
    if (flying && canMap && !battling && !choosingFromLongList && !blackjackActive) {
        if (rightClick) {
            flying = false;
            inMenu = false;
            inside = false;
            currentLoc.clear();
            return;
        }
        
        // Map is centered: mapX = 44, mapY = 4, map size = 231x231, cell size = 21x21
        int mapX = (320 - 231) / 2;  // 44
        int mapY = (240 - 231) / 2;  // 4
        int cellW = 21;  // 231 / 11 = 21 pixels per cell
        int cellH = 21;  // 231 / 11 = 21 pixels per cell
        
        // Check if click is within map bounds
        if (mouseX >= mapX && mouseX < mapX + 231 && mouseY >= mapY && mouseY < mapY + 231) {
            // Convert to map coordinates
            int mapLocalX = mouseX - mapX;
            int mapLocalY = mouseY - mapY;
            
            // Convert to grid coordinates (11x11 grid)
            int j = mapLocalX / cellW;
            int i = mapLocalY / cellH;
            
            if (i >= 0 && i < 11 && j >= 0 && j < 11) {
                std::string n = FlyLocation::NAME_MAP[i][j];
                if (n.empty()) {
                    return;
                }
                std::string p = printable(n);
                if (inside || FlyLocation::isRed(i, j)) {
                    currentLoc = p;
                    return;
                }
                auto it = FlyLocation::FLY_LOCATIONS.find(n);
                if (it == FlyLocation::FLY_LOCATIONS.end() || !it->second || !it->second->visited) {
                    return;
                }
                FlyLocation *f = it->second;
                loadMap(f->dest);
                playerX = f->x;
                playerY = f->y;
                offsetX = 0;
                offsetY = 0;
                inMenu = false;
                flying = false;
                return;
            }
        }
    }

    if (blackjackShowingResults) {
        longArr[0] = "";
        choosingFromLongList = false;
        blackjackShowingResults = false;
        inMenu = false;
        closeMenus();
        clickedChoice = 0;
        return;
    }

    if (blackjackActive) {
        if (blackjackFinished) {
            return;
        }

        static std::mt19937 s_blackjackRng(std::random_device{}());

        auto cardValue = [](int c) {
            int r = c % 13;
            if (r == 0) {
                return 11;
            }
            if (r >= 9) {
                return 10;
            }
            return r + 1;
        };
        auto handValue = [&](const std::vector<int> &hand) {
            int sum = 0;
            int aces = 0;
            for (int c : hand) {
                int v = cardValue(c);
                sum += v;
                if (c % 13 == 0) {
                    aces++;
                }
            }
            while (sum > 21 && aces-- > 0) {
                sum -= 10;
            }
            return sum;
        };
        auto isSoft = [&](const std::vector<int> &hand) {
            int sum = 0;
            int aces = 0;
            for (int c : hand) {
                sum += cardValue(c);
                if (c % 13 == 0) {
                    aces++;
                }
            }
            return aces > 0 && sum <= 21;
        };
        auto dealerShouldHit = [&](const std::vector<int> &hand) {
            int v = handValue(hand);
            return v < 17 || (v == 17 && isSoft(hand));
        };
        auto drawCard = [&]() {
            if (blackjackDeck.empty()) {
                blackjackDeck.clear();
                blackjackDeck.reserve(52);
                for (int i = 0; i < 52; i++) {
                    blackjackDeck.push_back(i);
                }
                std::shuffle(blackjackDeck.begin(), blackjackDeck.end(), s_blackjackRng);
            }
            int c = blackjackDeck.back();
            blackjackDeck.pop_back();
            return c;
        };

        if (!rightClick) {
            blackjackPlayer.push_back(drawCard());
            if (handValue(blackjackPlayer) > 21) {
                blackjackFinished = true;
                blackjackPlayerWon = false;
            }
        } else {
            blackjackHideFirst = false;
            while (dealerShouldHit(blackjackDealer)) {
                blackjackDealer.push_back(drawCard());
            }
            int p = handValue(blackjackPlayer);
            int d = handValue(blackjackDealer);
            if (p > 21) {
                blackjackFinished = true;
                blackjackPlayerWon = false;
            } else if (d > 21) {
                blackjackFinished = true;
                blackjackPlayerWon = true;
            } else if (p > d) {
                blackjackFinished = true;
                blackjackPlayerWon = true;
            } else if (p < d) {
                blackjackFinished = true;
                blackjackPlayerWon = false;
            } else {
                blackjackTieFrames = 60;
            }
        }

        if (blackjackFinished) {
            blackjackHideFirst = false;

            auto fmtCard = [](int c) {
                static const char *ranks[] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
                static const char *suits[] = {"H", "D", "C", "S"};
                int r = c % 13;
                int s = c / 13;
                std::string out = ranks[r];
                out += suits[s];
                return out;
            };
            auto fmtHand = [&](const std::vector<int> &hand) {
                std::string out;
                for (size_t i = 0; i < hand.size(); i++) {
                    if (!out.empty()) {
                        out += " ";
                    }
                    out += fmtCard(hand[i]);
                }
                return out;
            };

            int p = handValue(blackjackPlayer);
            int d = handValue(blackjackDealer);

            std::string result;
            result += blackjackPlayerWon ? "YOU WIN!\n\n" : "YOU LOSE\n\n";
            result += "Dealer: " + fmtHand(blackjackDealer) + " (" + std::to_string(d) + ")\n";
            result += "Player: " + fmtHand(blackjackPlayer) + " (" + std::to_string(p) + ")\n\n";

            std::string moneyLine;

            if (player) {
                if (blackjackPlayerWon) {
                    player->money += blackjackBet;
                    moneyLine = "You won $" + std::to_string(blackjackBet) + "!";
                } else {
                    player->money -= blackjackBet;
                    moneyLine = "You lost $" + std::to_string(blackjackBet) + "!";
                }
                bet = std::max(1, blackjackBet * 2);
            }

            longArr[0]=blackjackPlayerWon ? "YOU WIN!" : "YOU LOSE";
            longArr[1] = " ";
            longArr[2] = "Dealer:";
            longArr[3] = fmtHand(blackjackDealer) + " (" + std::to_string(d) + ")";
            longArr[4] = " ";
            longArr[5] = "Player:";
            longArr[6] = fmtHand(blackjackPlayer) + " (" + std::to_string(p) + ")";
            longArr[7] = " ";
            longArr[8] = moneyLine;
            longArr[9] = "";
            psi = 0;
            choosingFromLongList = true;
            blackjackShowingResults = true;
            blackjackActive = false;
            blackjackBet = 0;
        }
        return;
    }

    if (rightClick) {
        longArr[0] = "";
        choosingFromLongList = false;
        inMenu = false;
        closeMenus();
        rightClicked = true;
        clickedChoice = 0;
        return;
    }
    if (flying) {
        if (rightClick) {
            flying = false;
            inMenu = false;
            inside = false;
            currentLoc.clear();
            return;
        }
        if (mouseY < GameConfig::WORLD_HEIGHT) {
            return;
        }
        int mapX = mouseX;
        int mapY = mouseY - GameConfig::WORLD_HEIGHT;
        int cellW = GameConfig::BOTTOM_SCREEN_WIDTH / 11;
        int cellH = GameConfig::UI_HEIGHT / 11;
        int j = mapX / std::max(1, cellW);
        int i = mapY / std::max(1, cellH);
        if (i < 0 || j < 0 || i >= 11 || j >= 11) {
            return;
        }
        std::string n = FlyLocation::NAME_MAP[i][j];
        if (n.empty()) {
            return;
        }
        std::string p = printable(n);
        if (inside || FlyLocation::isRed(i, j)) {
            currentLoc = p;
            return;
        }
        auto it = FlyLocation::FLY_LOCATIONS.find(n);
        if (it == FlyLocation::FLY_LOCATIONS.end() || !it->second || !it->second->visited) {
            return;
        }
        FlyLocation *f = it->second;
        loadMap(f->dest);
        playerX = f->x;
        playerY = f->y;
        offsetX = 0;
        offsetY = 0;
        inMenu = false;
        flying = false;
        return;
    }
}

void Gui::playGame()
{
    while (true) {
        handleInput();
        update();
        render();
    }
}

void Gui::pressDirection(Direction d, bool pressed)
{
    if(pressed)
        heldDirection=d;
    else if(heldDirection==d)
        heldDirection=Direction::NONE;
}

void Gui::handleInput()
{
    scanKeys();
    u32 kDown = keysDown();
    u32 kHeld = keysHeld();
    pressDirection(Direction::WEST, kHeld & KEY_LEFT);
    pressDirection(Direction::EAST, kHeld & KEY_RIGHT);
    pressDirection(Direction::NORTH, kHeld & KEY_UP);
    pressDirection(Direction::SOUTH, kHeld & KEY_DOWN);
    
    if (kDown & KEY_A) {
        // Toggle auto-battle instead of Enter/Return
        autoBattle = !autoBattle;
    }
    if (kDown & KEY_START) {
        // Save game (mapped to what used to be Enter key)
        save();
    }
    if (kDown & KEY_X) {
        // X button opens inventory (mapped to SDLK_i)
        openMenu();
    }
    if (kDown & KEY_L) {
        // L button switches to previous menu
        currentMenu = (currentMenu+3) % 4; // Previous menu
        openMenu();
    }
    if (kDown & KEY_R) {
        // R button switches to next menu
        currentMenu = (currentMenu+1) % 4; // Previous menu
        openMenu();
    }
    if (kDown & KEY_Y) {
        // Y button toggles flying mode
        startFlying();
    }
}

void Gui::update() {
    if (inMenu) {
        return;
    }
    static std::mt19937 s_blackjackRng(std::random_device{}());

    frames++;
    if (frames % 6 == 0) {
        if (spacebar) {
            clickMouse(0, 0, false);
        }
        if (frames % 30 == 0) {
            if (mattNpc && !danceFrames.empty()) {
                mattNpc->bi = danceFrames[utils::randInt(0, static_cast<int>(danceFrames.size()) - 1)];
            }
        }
        if (monsterTrainer != nullptr) {
            if (oakNpc && !oakFrames.empty()) {
                oakNpc->bi = oakFrames[utils::randInt(0, static_cast<int>(oakFrames.size()) - 1)];
            }
            if (!monsterFrames.empty()) {
                if (frames == 24) {
                    monsterTrainer->bi = monsterFrames[0];
                } else if (frames == 48 && monsterFrames.size() > 2) {
                    monsterTrainer->bi = monsterFrames[2];
                } else if (monsterFrames.size() > 1) {
                    frames %= 60;
                    monsterTrainer->bi = monsterFrames[1];
                }
            }
        }
    }

    if (blackjackActive) {
        if (blackjackTieFrames > 0) {
            blackjackTieFrames--;
            if (blackjackTieFrames == 0) {
                blackjackPlayer.clear();
                blackjackDealer.clear();
                blackjackHideFirst = true;
                blackjackFinished = false;
                blackjackPlayerWon = false;

                if (blackjackDeck.size() < 10) {
                    blackjackDeck.clear();
                    blackjackDeck.reserve(52);
                    for (int i = 0; i < 52; i++) {
                        blackjackDeck.push_back(i);
                    }
                    std::shuffle(blackjackDeck.begin(), blackjackDeck.end(), s_blackjackRng);
                }
                auto drawCard = [&]() {
                    int c = blackjackDeck.back();
                    blackjackDeck.pop_back();
                    return c;
                };
                blackjackPlayer.push_back(drawCard());
                blackjackPlayer.push_back(drawCard());
                blackjackDealer.push_back(drawCard());
                blackjackDealer.push_back(drawCard());
            }
        }
        return;
    }

    if (stepPhase == StepPhase::NONE && heldDirection != Direction::NONE) {
        facing = heldDirection;
        currentStepFrames = canMove(heldDirection) ? NUM_STEP_FRAMES : BUMP_STEP_FRAMES;
        stepPhase = StepPhase::MOVING;
        phaseFrame = 0;
    }
    if (stepPhase != StepPhase::NONE) {
        advanceStep();
    }
}

void Gui::render() {
    fillRectangle(top_fb, BLACK, 0, 0, 256, 192);
    
    int viewW = GameConfig::TOP_SCREEN_WIDTH;
    int viewH = GameConfig::TOP_SCREEN_HEIGHT;
    int camX = static_cast<int>(playerX * GameConfig::TILE_SIZE + offsetX - viewW / 2 + GameConfig::TILE_SIZE / 2);
    int camY = static_cast<int>(playerY * GameConfig::TILE_SIZE + offsetY - viewH / 2 + GameConfig::TILE_SIZE / 2);

    int startX = std::max(0, camX / GameConfig::TILE_SIZE - 1);
    int startY = std::max(0, camY / GameConfig::TILE_SIZE - 1);
    int endX = std::min(pm->getGridCols(), startX + viewW / GameConfig::TILE_SIZE + 3);
    int endY = std::min(pm->getGridRows(), startY + viewH / GameConfig::TILE_SIZE + 3);
    
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            int tileId = pm->getGridValue(y, x);
            int screenX = x * GameConfig::TILE_SIZE - camX;
            int screenY = y * GameConfig::TILE_SIZE - camY;
            
            drawTile(tileId, screenX, screenY);
            int index = y * pm->getGridCols() + x;
            Trainer *t = pm->trainers[index];
            if (t) {
                drawSprite(t->bi, screenX, screenY);
            } else if (pm->wob[index]) {
                drawSprite(pm->wob[index]->bi, screenX, screenY);
            } else if (pm->npcs[index]) {
                drawSprite(pm->npcs[index]->bi, screenX, screenY);
            }
            
        }
    }

    for (int i = 0; i < 4; i++) {
        if (connections[i]) {
            drawConnection(i, connections[i], connOffsets[i], camX, camY);
        }
    }

    int drawX = viewW / 2 - GameConfig::TILE_SIZE / 2;
    int drawY = viewH / 2 - GameConfig::TILE_SIZE / 2;
    
    // Draw player sprite using RED sprites
    int currentFrame = getCurrentFrame();
    
    if (surfing) {
        // Use SEEL sprite frames when surfing
        switch (currentFrame) {
            case 0: drawSprite(SEEL_0_data, drawX, drawY); break;
            case 1: drawSprite(SEEL_1_data, drawX, drawY); break;
            case 2: drawSprite(SEEL_2_data, drawX, drawY); break;
            case 3: drawSprite(SEEL_3_data, drawX, drawY); break;
            case 4: drawSprite(SEEL_4_data, drawX, drawY); break;
            case 5: drawSprite(SEEL_5_data, drawX, drawY); break;
            case 6: drawSprite(SEEL_6_data, drawX, drawY); break;
            case 7: drawSprite(SEEL_7_data, drawX, drawY); break;
            case 8: drawSprite(SEEL_8_data, drawX, drawY); break;
            case 9: drawSprite(SEEL_9_data, drawX, drawY); break;
            default: print("No seel");
        }
    } else {
        // Use appropriate RED sprite based on current frame
        switch (currentFrame) {
            case 0: drawSprite(RED_0_data, drawX, drawY); break;
            case 1: drawSprite(RED_1_data, drawX, drawY); break;
            case 2: drawSprite(RED_2_data, drawX, drawY); break;
            case 3: drawSprite(RED_3_data, drawX, drawY); break;
            case 4: drawSprite(RED_4_data, drawX, drawY); break;
            case 5: drawSprite(RED_5_data, drawX, drawY); break;
            case 6: drawSprite(RED_6_data, drawX, drawY); break;
            case 7: drawSprite(RED_7_data, drawX, drawY); break;
            case 8: drawSprite(RED_8_data, drawX, drawY); break;
            case 9: drawSprite(RED_9_data, drawX, drawY); break;
            default: print("No red");
        }
    }

    // Set bottom screen color based on auto-battle status
    fillRectangle(bottom_fb, RGB15(8, 8, 8) | BIT(15), 0, 0, 256, 192); // Dark gray
    if (autoBattle) {
        fillRectangle(bottom_fb, RGB15(0, 31, 0) | BIT(15), 0, 0, 20, 20); // Green square for auto-battling
    }

    /*bool canDrawMap = !showingText && !battling && !choosingFromLongList && !blackjackActive &&!inMenu;
    if (canDrawMap||flying) {
        //TODO: draw map on bottom screen with filled squares

        if (flying) {
            // Draw red overlay for flyable locations
            for (int i = 0; i < 11; i++) {
                for (int j = 0; j < 11; j++) {
                    if (FlyLocation::isRed(i, j)) {
                        fillRectangle(bottom_fb, RGB15(0, 0, 31) | BIT(15), 4+i*21, 44+j*21, 21, 21);
                    }
                }
            }
        }

        // Draw player sprite on map
        int currentFrame = getCurrentFrame();
        
        // Find and highlight current position on map
        if (!inside) {
            for (int i = 0; i < 11; i++) {
                for (int j = 0; j < 11; j++) {
                    if (pm->name == FlyLocation::NAME_MAP[i][j]) {
                        // Update player position on map
                        int playerMapX = 44+j*21;
                        int playerMapY = 4+i*21;
                        
                        // Redraw player sprite at correct position
                        if (surfing) {
                            switch(currentFrame) {
                                case 0: drawSprite(SEEL_0_data, playerMapX, playerMapY); break;
                                case 1: drawSprite(SEEL_1_data, playerMapX, playerMapY); break;
                                case 2: drawSprite(SEEL_2_data, playerMapX, playerMapY); break;
                                case 3: drawSprite(SEEL_3_data, playerMapX, playerMapY); break;
                                case 4: drawSprite(SEEL_4_data, playerMapX, playerMapY); break;
                                case 5: drawSprite(SEEL_5_data, playerMapX, playerMapY); break;
                                case 6: drawSprite(SEEL_6_data, playerMapX, playerMapY); break;
                                case 7: drawSprite(SEEL_7_data, playerMapX, playerMapY); break;
                                case 8: drawSprite(SEEL_8_data, playerMapX, playerMapY); break;
                                case 9: drawSprite(SEEL_9_data, playerMapX, playerMapY); break;
                            }
                        } else {
                            switch (currentFrame) {
                                case 0: drawSprite(RED_0_data, playerMapX, playerMapY); break;
                                case 1: drawSprite(RED_1_data, playerMapX, playerMapY); break;
                                case 2: drawSprite(RED_2_data, playerMapX, playerMapY); break;
                                case 3: drawSprite(RED_3_data, playerMapX, playerMapY); break;
                                case 4: drawSprite(RED_4_data, playerMapX, playerMapY); break;
                                case 5: drawSprite(RED_5_data, playerMapX, playerMapY); break;
                                case 6: drawSprite(RED_6_data, playerMapX, playerMapY); break;
                                case 7: drawSprite(RED_7_data, playerMapX, playerMapY); break;
                                case 8: drawSprite(RED_8_data, playerMapX, playerMapY); break;
                                case 9: drawSprite(RED_9_data, playerMapX, playerMapY); break;
                            }
                        }
                        break;
                    }
                }
            }
        }

        if (flying && !currentLoc.empty()) {
            drawRainbowText(bottom_fb, currentLoc, 10, 10);
        }
    }*/

    if (blackjackActive) {
        auto cardValue = [](int c) {
            int r = c % 13;
            if (r == 0) {
                return 11;
            }
            if (r >= 9) {
                return 10;
            }
            return r + 1;
        };

        auto handValue = [&](const std::vector<int> &hand) {
            int sum = 0;
            int aces = 0;
            for (int c : hand) {
                sum += cardValue(c);
                if (c % 13 == 0) {
                    aces++;
                }
            }
            while (sum > 21 && aces-- > 0) {
                sum -= 10;
            }
            return sum;
        };

        auto fmtCard = [](int c) {
            static const char *ranks[] = {"A", "2", "3", "4", "5", "6", "7", "8", "9", "10", "J", "Q", "K"};
            static const char *suits[] = {"H", "D", "C", "S"};
            int r = c % 13;
            int s = c / 13;
            std::string out = ranks[r];
            out += suits[s];
            return out;
        };

        auto fmtHand = [&](const std::vector<int> &hand, bool hideFirst) {
            std::string out;
            for (size_t i = 0; i < hand.size(); i++) {
                if (!out.empty()) {
                    out += " ";
                }
                if (i == 0 && hideFirst) {
                    out += "??";
                } else {
                    out += fmtCard(hand[i]);
                }
            }
            return out;
        };

        // Draw white background on bottom screen
        fillRectangle(bottom_fb, WHITE, 0, 0, 256, 192);

        // Draw "Blackjack" title on bottom screen
        drawText("Blackjack", (WIDTH - 120) / 2, 30, BLACK, bottom_fb);

        std::vector<std::string> lines;
        lines.push_back("");
        lines.push_back("Dealer:");
        lines.push_back(fmtHand(blackjackDealer, blackjackHideFirst));
        lines.push_back("");
        lines.push_back("Player:");
        lines.push_back(fmtHand(blackjackPlayer, false));
        lines.push_back("Points: " + std::to_string(handValue(blackjackPlayer)));
        lines.push_back("");
        if (blackjackTieFrames > 0) {
            lines.push_back("TIE! DEALING AGAIN");
        } else {
            lines.push_back("Tap: Hit");
            lines.push_back("B button: Stand");
        }

        int x = 60;
        int y = 60;
        int maxLines = 20;
        for (int lineIndex = 0; lineIndex < maxLines; lineIndex++) {
            if (lineIndex >= static_cast<int>(lines.size())) {
                break;
            }
            // Use drawText for bottom screen
            drawText(lines[lineIndex].c_str(), x, y + lineIndex * 12, BLACK, bottom_fb);
        }
    } else if (choosingFromLongList) {
        // Draw dark rectangle background for header on top screen near bottom
        int headerX = 0;
        int headerY = 200;
        int headerWidth = 400;
        int headerHeight = 40;
        fillRectangle(top_fb, WHITE, headerX, headerY, headerWidth, headerHeight);
        auto headerText = [&]() -> std::string {
            if (!longArrHeader.empty()) {
                return longArrHeader;
            }
            if (blackjackShowingResults) {
                return "Blackjack";
            }
            return "Inventory";
        };

        std::string header = headerText();
        
        // Draw header text on top screen (centered in dark rectangle)
        drawText(header.c_str(), headerX, headerY, BLACK, top_fb);

        // Clear bottom screen for menu data
        fillRectangle(bottom_fb, WHITE, 0, 0, 256, 192); // White background

        // Render longArr data on bottom screen every 12 pixels starting at 0,0
        int maxLines = 20;
        int drawStart = 0;
        for (int lineIndex = 0; lineIndex < maxLines; lineIndex++) {
            int i = drawStart + lineIndex;
            drawText(longArr[i].c_str(), 0, lineIndex * 8, BLACK, bottom_fb);
        }
    } else if (flying) {
    } else if (inMenu) {
        if (choosingFromLongList) {
            // Use same rendering logic as main choosingFromLongList section
            // Draw dark rectangle background for header on top screen near bottom
            int headerX = 0;
            int headerY = 200;
            int headerWidth = 400;
            int headerHeight = 40;
            fillRectangle(top_fb, WHITE, headerX, headerY, headerWidth, headerHeight);

            auto headerText = [&]() -> std::string {
                if (!longArrHeader.empty()) {
                    return longArrHeader;
                }
                return "Inventory";
            };

            std::string header = headerText();
            
            // Draw header text on top screen (centered in dark rectangle)
            drawText(header.c_str(), headerX, headerY, BLACK, top_fb);

            // Clear bottom screen for menu data
            fillRectangle(bottom_fb, WHITE, 0, 0, 256, 192); // White background

            // Render longArr data on bottom screen every 12 pixels starting at 0,0
            int maxLines = 20;
            int drawStart = 0;
            for (int lineIndex = 0; lineIndex < maxLines; lineIndex++) {
                int i = drawStart + lineIndex;
                drawText(longArr[i].c_str(), 0, lineIndex * 8, BLACK, bottom_fb);
            }
        }
    }
    
    // Flush buffers and swap screens for 3DS
    swiWaitForVBlank();
    copyBuffers();
}

void Gui::drawConnection(int dirIndex, PokeMap *conn, int offset, int camX, int camY) {
    int baseX = 0;
    int baseY = 0;
    switch (dirIndex) {
        case 0:
            baseX = offset * GameConfig::TILE_SIZE;
            baseY = -static_cast<int>(conn->getGridRows()) * GameConfig::TILE_SIZE;
            break;
        case 1:
            baseX = offset * GameConfig::TILE_SIZE;
            baseY = pm->getGridRows() * GameConfig::TILE_SIZE;
            break;
        case 2:
            baseX = -static_cast<int>(conn->getGridCols()) * GameConfig::TILE_SIZE;
            baseY = offset * GameConfig::TILE_SIZE;
            break;
        case 3:
            baseX = pm->getGridCols() * GameConfig::TILE_SIZE;
            baseY = offset * GameConfig::TILE_SIZE;
            break;
    }
    int px = baseX - camX;
    int py = baseY - camY;

    auto floorDiv = [](int a, int b) {
        int q = a / b;
        int r = a % b;
        if (r != 0 && ((r < 0) != (b < 0))) {
            q--;
        }
        return q;
    };

    int tilesW = GameConfig::TOP_SCREEN_WIDTH / GameConfig::TILE_SIZE + 3;
    int tilesH = GameConfig::WORLD_HEIGHT / GameConfig::TILE_SIZE + 3;

    int sx = floorDiv(camX - baseX, GameConfig::TILE_SIZE) - 1;
    int sy = floorDiv(camY - baseY, GameConfig::TILE_SIZE) - 1;
    int ex = std::min(sx + tilesW, static_cast<int>(conn->getGridCols()));
    int ey = std::min(sy + tilesH, static_cast<int>(conn->getGridRows()));
    sx = std::max(sx, 0);
    sy = std::max(sy, 0);
    for (int y = sy; y < ey; y++) {
        for (int x = sx; x < ex; x++) {
            int screenX = x * GameConfig::TILE_SIZE + px;
            int screenY = y * GameConfig::TILE_SIZE + py;
            
            int tileId = conn->getGridValue(y, x);
            drawTile(tileId, screenX, screenY);
            int index = y * conn->getGridCols() + x;
            Trainer *t = conn->trainers[index];
            if (t) {
                drawSprite(t->bi, screenX, screenY);
            } else if (conn->wob[index]) {
                drawSprite(conn->wob[index]->bi, screenX, screenY);
            } else if (conn->npcs[index]) {
                drawSprite(conn->npcs[index]->bi, screenX, screenY);
            }
            
        }
    }
}

int Gui::getCurrentFrame() const {
    bool moving = stepPhase == StepPhase::MOVING || (stepPhase == StepPhase::LANDING && phaseFrame < currentStepFrames / 2);
    switch (facing) {
        case Direction::NONE:
            return 1;
        case Direction::SOUTH:
            return moving ? 0 + timesMoved : 1;
        case Direction::NORTH:
            return moving ? 3 + timesMoved : 4;
        case Direction::WEST:
            return moving ? 7 : 6;
        case Direction::EAST:
            return moving ? 9 : 8;
    }
    return 1;
}

bool Gui::canMove(Direction d) {
    if (switchingMaps) {
        return true;
    }
    if (playerX == pm->getGridCols()) {
        print("You beat the game! Now fly out of here and save... and maybe talk to the professor, too!");
        playerX = 1;
        startFlying();
        return true;
    }
    int nx = playerX;
    int ny = playerY;
    switch (d) {
        case Direction::NONE:
            return false;
        case Direction::SOUTH:
            ny++;
            break;
        case Direction::NORTH:
            ny--;
            break;
        case Direction::WEST:
            nx--;
            break;
        case Direction::EAST:
            nx++;
            break;
    }
    if (nx < 0 || ny < 0 || ny >= pm->getGridRows() || nx >= pm->getGridCols()) {
        return false;
    }
    /*Npc *n=pm->npcs[ny][nx];
    if (n) {
        n->interact(player);
        return n->dead;
    }*/
    switch (pm->getTypeValue(playerY, playerX)) {
        case 5:
            return d == Direction::SOUTH;
        case 9:
            return d != Direction::NORTH && pm->getTypeValue(ny, nx) != 0;
        case 10:
            return d == Direction::WEST;
        case 11:
            return d == Direction::EAST;
    }
    switch (pm->getTypeValue(ny, nx)) {
        case 0:
            return false;
        case 3:
            print(printable(pm->name) + "... I think? Man, I wish I could read!");
            return false;
        case 4:
            return player->leadersBeaten[4] && player->hasMove(SURF);
        case 5:
            return d == Direction::SOUTH;
        case 6:
            return player->leadersBeaten[1] && player->hasMove(CUT);
        case 9:
            return d != Direction::SOUTH;
        case 10:
            return d == Direction::WEST;
        case 11:
            return d == Direction::EAST;
        case 19:
            return player->leadersBeaten[3] && player->hasMove(STRENGTH);
        case 20:
            print("Ew, that stinks!");
            if (utils::rand01() < 0.1) {
                if (utils::rand01() < 0.01) {
                    print("Whoa! Who would throw this away?!");
                    player->give(Item::getItem("Rare Candy"));
                } else {
                    print("Oh no, I'll have to wash my hands...");
                    player->give(Item::getItem("...Secret Sauce"));
                }
            }
            return false;
    }
    return true;
}

std::string Gui::printable(const std::string &s) {
    std::string out;
    out.reserve(s.size() + 8);
    for (size_t i = 0; i < s.size(); i++) {
        char c = s[i];
        if (i > 0) {
            char prev = s[i - 1];
            if (std::isdigit(static_cast<unsigned char>(c)) && std::isalpha(static_cast<unsigned char>(prev))) {
                out.push_back(' ');
            } else if (std::isupper(static_cast<unsigned char>(c)) &&
                       std::islower(static_cast<unsigned char>(prev))) {
                out.push_back(' ');
            }
        }
        out.push_back(c);
    }
    return out;
}

int Gui::elevate(const std::string &s, bool B, int m) {
    std::string prompt = (pm && pm->name == "ViridianBunglerHouse") ? "..." : "Enter the floor you would like to go to:";
    stopMovement();

    if (B && player && LIFT_KEY && !player->hasItem(LIFT_KEY)) {
        print("Weird, nothing is happening.");
        return 0;
    }

    std::string input = this->promptText(prompt);
    if (input.empty()) {
        return 0;
    }
    if (pm && pm->name == "ViridianBunglerHouse") {
        if (input == "DEATH" && player && player->pokedex[150]) {
            loadMap(PokeMap::POKEMAPS["RocketHideoutB1F"]);
            return 1;
        }
        return 0;
    }
    std::string u = input;
    std::transform(u.begin(), u.end(), u.begin(), ::toupper);
    if (B && !u.empty() && u[0] == 'B') {
        u = u.substr(1);
    }
    if (!u.empty() && u.back() == 'F') {
        u.pop_back();
    }
    int q = 0;
    try {
        q = std::stoi(u);
    } catch (...) {
        print(input + ", huh? Yeah, okay buddy.");
        return 0;
    }
    if (q <= 0 || q > m) {
        print("Uhh, you know there are only " + std::to_string(m) + " floors, right?");
        return 0;
    }
    if (B && q == 3) {
        print("...no.");
        return 0;
    }
    loadMap(PokeMap::POKEMAPS[s + (B ? "B" + std::to_string(q) : std::to_string(q)) + "F"]);
    print("Wee!");
    return q;
}

char Gui::randomSpooky() {
    return static_cast<char>(32 + utils::randInt(0, 95));
}

void Gui::printSpooky() {
    std::string s;
    for (int i = 0; i < 10; i++) {
        s.push_back(randomSpooky());
    }
    s += "DEATH";
    for (int i = 0; i < 10; i++) {
        s.push_back(randomSpooky());
    }
    print(s);
}

void Gui::advanceStep() {
    
    float delta = static_cast<float>(GameConfig::TILE_SIZE) / static_cast<float>(currentStepFrames);
    int dx = 0;
    int dy = 0;
    PokeMap *newMap = nullptr;
    switch (facing) {
        case Direction::NONE:
            return;
        case Direction::SOUTH:
            dy = 1;
            if (playerY == pm->getGridRows() - 1 && connections[1] != nullptr) {
                newMap = connections[1];
                playerX -= connOffsets[1];
                playerY = -1;
            }
            break;
        case Direction::NORTH:
            dy = -1;
            if (playerY == 0 && connections[0] != nullptr) {
                newMap = connections[0];
                playerX -= connOffsets[0];
                playerY = static_cast<int>(newMap->getGridRows());
            }
            break;
        case Direction::WEST:
            dx = -1;
            if (playerX == 0 && connections[2] != nullptr) {
                newMap = connections[2];
                playerX = static_cast<int>(newMap->getGridCols());
                playerY -= connOffsets[2];
            }
            break;
        case Direction::EAST:
            dx = 1;
            if (playerX == pm->getGridCols() - 1 && connections[3] != nullptr) {
                newMap = connections[3];
                playerX = -1;
                playerY -= connOffsets[3];
            }
            break;
    }
    if (newMap != nullptr) {
        mapName = newMap->name;
        loadMap(newMap);
        switchingMaps = true;
        currentStepFrames /= 2;
    }
    bool isBump = currentStepFrames == BUMP_STEP_FRAMES;
    int nextX = playerX + dx;
    int nextY = playerY + dy;
    if (isBump) {
        bool b = nextX >= 0 && nextY >= 0 && nextY < pm->getGridRows() && nextX < pm->getGridCols();

        if (b && (pm->getGridValue(nextY, nextX) == 582 || pm->getGridValue(nextY, nextX) == 583)) {
            stopMovement();
            phaseFrame = currentStepFrames;
            print("Honestly, this one doesn't really look like " + player->team[0]->nickname +
                ", although it says it's supposed to be... " + Monster::MONSTERS[utils::randInt(1, 100)].name + "?");
            return;
        }

        if (pm->getTypeValue(playerY, playerX) == 2 || pm->getTypeValue(playerY, playerX) == 18) {
            Warp *w = pm->getWarp(playerY, playerX);
            if (w == nullptr) {
                w = pm->getNearbyWarp(playerY, playerX);
                if (w == nullptr) {
                    inMenu = true;
                    int n = elevate("RocketHideout", true, 4);
                    if (n > 0) {
                        playerY = HIDEOUT_Y[n];
                    } else {
                        playerY--;
                    }
                    inMenu = false;
                    return;
                }
            }
            stepPhase = StepPhase::NONE;
            loadMap(w->pm);
            playerX = w->col;
            playerY = w->row;
            offsetX = 0;
            offsetY = 0;
            currentStepFrames /= 2;
            return;
        }

        if (pm->healX == playerX && pm->healY == playerY) {
            stopMovement();
            player->healTeam();
            phaseFrame = currentStepFrames;
            lastHeal = pm;
            FlyLocation::visit(pm->name);
            print("Your team was fully healed!");
            return;
        }

        if (b) {
            int index = nextY * pm->getGridCols() + nextX;
            WorldObject *wo = pm->wob[index];
            if (wo) {
                stopMovement();
                std::optional<bool> res = wo->stepOn(this);
                if (res.has_value() && res.value()) {
                    pm->stepOn(player, nextX, nextY);
                } else if (res.has_value() && !res.value()) {
                    playerX -= dx;
                    playerY -= dy;
                    offsetX = 0;
                    offsetY = 0;
                }
                phaseFrame = currentStepFrames;
                return;
            }
            int npcIndex = nextY * pm->getGridCols() + nextX;
            if (pm->npcs[npcIndex] != nullptr) {
                stopMovement();
                phaseFrame = currentStepFrames;
                bool oldCanMap = canMap;
                canMap = false;
                pm->npcs[npcIndex]->interact(player);

                if (pm->name == "Daycare") {
                    u32 p = 5000;
                    int d = player->team[0]->dexNum;
                    if (d == 0) {
                        p = 50000;
                    } else if (d > 132) {
                        p = 25000;
                    }
                    if (p > player->money) {
                        print("Wait, you're sooo poor! You need at least $" + std::to_string(p) +
                              " for this child, I guess I'm keeping this one...");
                    } else {
                        player->money -= p;
                        print("You spent $" + std::to_string(p) + "...");
                        player->give(new Battler(player->team[0]));
                    }
                }

                canMap = oldCanMap;
                return;
            }
            if (pm->getGridValue(playerY, playerX) == 122 && pm->name == "GameCorner") {
                stopMovement();
                phaseFrame = currentStepFrames;
                if (player->hasItem(FAKE_ID)) {
                    if (player->money < bet) {
                        print("No! I want to bet $" + std::to_string(bet) + " if I play, but I'm broke!");
                    } else {
                        static std::mt19937 s_blackjackRng(std::random_device{}());

                        blackjackActive = true;
                        blackjackBet = bet;
                        blackjackFinished = false;
                        blackjackTieFrames = 0;
                        blackjackHideFirst = true;

                        blackjackDeck.clear();
                        blackjackDeck.reserve(52);
                        for (int i = 0; i < 52; i++) {
                            blackjackDeck.push_back(i);
                        }
                        std::shuffle(blackjackDeck.begin(), blackjackDeck.end(), s_blackjackRng);
                        auto drawCard = [&]() {
                            int c = blackjackDeck.back();
                            blackjackDeck.pop_back();
                            return c;
                        };

                        blackjackPlayer.clear();
                        blackjackDealer.clear();
                        blackjackPlayer.push_back(drawCard());
                        blackjackPlayer.push_back(drawCard());
                        blackjackDealer.push_back(drawCard());
                        blackjackDealer.push_back(drawCard());

                        auto cardValue = [](int c) {
                            int r = c % 13;
                            if (r == 0) {
                                return 11;
                            }
                            if (r >= 9) {
                                return 10;
                            }
                            return r + 1;
                        };
                        auto handValue = [&](const std::vector<int> &hand) {
                            int sum = 0;
                            int aces = 0;
                            for (int c : hand) {
                                sum += cardValue(c);
                                if (c % 13 == 0) {
                                    aces++;
                                }
                            }
                            while (sum > 21 && aces-- > 0) {
                                sum -= 10;
                            }
                            return sum;
                        };

                        if (handValue(blackjackPlayer) == 21) {
                            blackjackHideFirst = false;
                            blackjackFinished = true;
                            blackjackPlayerWon = true;
                            player->money += blackjackBet;
                            blackjackActive = false;
                            blackjackBet = 0;
                            bet *= 2;
                            print("You won $" + std::to_string(bet / 2) + "!");
                        }
                    }
                } else {
                    print("Crap, it wants me to scan an ID!!");
                }
                return;
            }
            switch (pm->getGridValue(nextY, nextX)) {
                case 143:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Hey baby, come sit on my lap!");
                    print("(Eek, I'd better get out of here!)");
                    heldDirection = Direction::EAST;
                    facing = Direction::EAST;
                    return;
                case 35:
                case 220:
                case 388:
                case 420:
                    stopMovement();
                    depositWithdraw();
                    phaseFrame = currentStepFrames;
                    return;
                case 51:
                case 52:
                case 65:
                case 66:
                case 139:
                case 141:
                case 326:
                case 329:
                case 534:
                case 535:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("No " + player->name + ", you told yourself you'd stop eating plants!");
                    return;
                case 3:
                case 4:
                case 56:
                case 57:
                case 317:
                case 328:
                case 435:
                case 438:
                case 496:
                case 668:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Hot dog, that looks just like " + player->team[0]->nickname + "!");
                    return;
                case 189:
                case 190:
                case 192:
                case 193:
                case 279:
                case 280:
                case 281:
                case 282:
                case 284:
                case 285:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Gah, I can never find what I need! May as well speak to a worker...");
                    return;
                case 2:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Here lies... huh? I wish I could read!");
                    return;
                case 42:
                case 378:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Golly gee, I could stand here all day looking at this view!");
                    return;
                case 44:
                case 45:
                case 61:
                case 161:
                case 350:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Books, huh? Boooring!");
                    return;
                case 9:
                case 13:
                case 14:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Whoa, this one's a beauty! But I'll never afford it.");
                    return;
                case 20:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Aw great, now my hands are covered in oil! I hate getting lubed up.");
                    return;
                case 36:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("How the heck do I work this crazy thing?!");
                    return;
                case 31:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Nope, DEFINITELY not going in there!");
                    return;
                case 41:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Gasp! Who would hang this in their house?!");
                    return;
                case 62:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("A trophy for comedy! How pathetic.");
                    return;
                case 177:
                case 178:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    heldDirection = Direction::SOUTH;
                    print("NO, I HATE SCHOOL!");
                    return;
                case 179:
                case 180:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    heldDirection = Direction::SOUTH;
                    print("NO, I HATE SCHOOL!");
                    return;
                case 185:
                case 188:
                case 333:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Hehehe look at all those squiggles... I wonder what they mean!");
                    return;
                case 194:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Limited time sale on EVERYTHING?! I'd better act quick!");
                    return;
                case 226:
                case 614:
                case 633:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("It'd probably be rude to drink this... slurp!");
                    return;
                case 302:
                case 303:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Well, I suppose a nibble couldn't hurt... CRUNCH!");
                    return;
                case 314:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Wait, is there somebody INSIDE there? I'd better keep my distance...");
                    return;
                case 347:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Blah blah words blah, LAME!");
                    return;
                case 391:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("A bachelor's degree in... computer science? Well that's worthless!");
                    return;
                case 206:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Are those... pokeballs? Maybe I should just snatch one... eh, seems risky!");
                    return;
                case 455:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("This must be what the region looked like thousands of years ago, neat!");
                    return;
                case 464:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("What? It just says *ROCKS*, no duh!!");
                    return;
                case 575:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Hehe, ohhh yeah, that's the good stuff!");
                    return;
                case 627:
                case 629:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("It says here the ship is... sinking?! I'd better get out of here!");
                    return;
                case 625:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Yikes, this thing could tip over at any second!");
                    return;
                case 631:
                case 632:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Is that... slowpoke tail? My my, how SCRUMPTIOUS indeed!");
                    return;
                case 164:
                case 165:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("No time to sleep, I've got pokemon to catch!");
                    return;
                case 195:
                case 379:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Whoa, I don't think I'm old enough to look at this...");
                    return;
                case 196:
                case 390:
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Crud, he hit me with Rat Gambit again! I suck at this game.");
                    return;
                default:
                    break;
            }

            martItems = pm->getMartItems(playerX, playerY);
            if (martItems == nullptr) {
                if (pm->getGridValue(nextY, nextX) == 187) {
                    stopMovement();
                    phaseFrame = currentStepFrames;
                    print("Hm, better not touch that...");
                    return;
                }
            } else {
                stopMovement();
                buySell();
                return;
            }
        }
    } else if (isBump) {
        float t = static_cast<float>(phaseFrame) / static_cast<float>(std::max(1, currentStepFrames - 1));
        float push = (t < 0.5f) ? (t * 2.0f) : ((1.0f - t) * 2.0f);
        float bumpDelta = delta * 0.35f;
        offsetX += dx * bumpDelta * push;
        offsetY += dy * bumpDelta * push;
    } else if (canMove(facing)) {
        offsetX += dx * delta;
        offsetY += dy * delta;
    }
    phaseFrame++;
    if (phaseFrame >= currentStepFrames) {
        
        switchingMaps = false;
        phaseFrame = 0;
        stepPhase = StepPhase::NONE;
        offsetX = 0;
        offsetY = 0;
        timesMoved = timesMoved == 0 ? 2 : 0;
        if (!isBump) {
            int prevType = -1;
            if (playerX >= 0 && playerY >= 0 && playerY < pm->getGridRows() &&
                playerX < pm->getGridCols()) {
                prevType = pm->getTypeValue(playerY, playerX);
            }
            playerX = nextX;
            playerY = nextY;
            if (playerX < 0 || playerY < 0 || playerY >= pm->getGridRows() ||
                playerX >= pm->getGridCols()) {
                return;
            }

            auto startForcedStep = [&](Direction d) {
                if (!canMove(d)) {
                    return;
                }
                facing = d;
                currentStepFrames = NUM_STEP_FRAMES;
                stepPhase = StepPhase::MOVING;
                phaseFrame = 0;
            };

            int curType = pm->getTypeValue(playerY, playerX);
            if (pm && player && curType == 13 && prevType != 13 && pm->name.rfind("PokemonTower", 0) == 0) {
                player->healTeam();
                print("You feel power flow through you. Your pokemon have been healed!");
            }
            if (curType == 4) {
                surfing = true;
            } else if (curType == 1 || curType == 8 || curType == 12) {
                surfing = false;
            }

            if (curType == 5) {
                startForcedStep(Direction::SOUTH);
                return;
            }
            if (curType == 10) {
                startForcedStep(Direction::WEST);
                return;
            }
            if (curType == 11) {
                startForcedStep(Direction::EAST);
                return;
            }

            if (curType == 14) {
                spinning = true;
                startForcedStep(Direction::WEST);
                return;
            }
            if (curType == 15) {
                spinning = true;
                startForcedStep(Direction::EAST);
                return;
            }
            if (curType == 16) {
                spinning = true;
                startForcedStep(Direction::NORTH);
                return;
            }
            if (curType == 17) {
                spinning = true;
                startForcedStep(Direction::SOUTH);
                return;
            }

            if (spinning && pm && pm->getGridValue(playerY, playerX) == 545) {
                spinning = false;
            }

            if (spinning) {
                startForcedStep(facing);
                return;
            }

            if (curType == 7) {
                if (spinning) {
                    spinning = false;
                }
                if (pm->getGridValue(playerY, playerX) == 182) {
                    inMenu = true;
                    elevate("CeladonMart", false, 5);
                    inMenu = false;
                    return;
                }
                if (pm->getGridValue(playerY, playerX) == 697) {
                    inMenu = true;
                    int f = elevate("SilphCo", false, 11);
                    if (f > 0) {
                        playerX = SILPH_X[f];
                    }
                    inMenu = false;
                    return;
                }
                Warp *w = pm->getWarp(playerY, playerX);
                if (w) {
                    PokeMap *from = pm;
                    loadMap(w->pm);
                    playerX = w->col;
                    playerY = w->row;
                    offsetX = 0;
                    offsetY = 0;
                    if (playerX == 0 && playerY == 6 && from && from->name == "RedsHouse2F") {
                        player->ballin = true;
                        Trainer::addEliteFour();
                        print("What? How did I get back here??");
                    }
                    return;
                }
            }
            if (pm->healX == playerX && pm->healY == playerY) {
                if (spinning) {
                    spinning = false;
                }
                player->healTeam();
                lastHeal = pm;
                FlyLocation::visit(pm->name);
                print("Your team was fully healed!");
            }
            int woIndex = playerY * pm->getGridCols() + playerX;
            WorldObject *wo = pm->wob[woIndex];
            if (wo) {
                if (spinning) {
                    spinning = false;
                }
                std::optional<bool> res = wo->stepOn(this);
                if (!res.has_value()) {
                    if(wildMon)
                        delete wildMon;
                    wildMon = new Battler(wo->level, wo->mon);
                    bool oldCanMap = canMap;
                    canMap = false;
                    int result = BattleState::wildBattle(player->team, wildMon);
                    if (result < 0) {
                        blackout();
                    } else {
                        player->money += result * (player->hasItem(AMULET_COIN) ? 2 : 1);
                        pm->stepOn(player, playerX, playerY);
                    }
                    battling = false;
                    canMap = oldCanMap;
                    offsetX = 0;
                    offsetY = 0;
                    return;
                }
                if (res.value()) {
                    pm->stepOn(player, playerX, playerY);
                } else {
                    playerX -= dx;
                    playerY -= dy;
                    offsetX = 0;
                    offsetY = 0;
                }
            }
            int nIndex = playerY * pm->getGridCols() + playerX;
            Npc *n = pm->npcs[nIndex];
            if (n) {
                if (spinning) {
                    spinning = false;
                }
                stopMovement();
                bool oldCanMap = canMap;
                canMap = false;
                n->interact(player);

                if (pm->name == "Daycare") {
                    u32 p = 5000;
                    int d = player->team[0]->dexNum;
                    if (d == 0) {
                        p = 50000;
                    } else if (d > 132) {
                        p = 25000;
                    }
                    if (p > player->money) {
                        print("Wait, you're sooo poor! You need at least $" + std::to_string(p) +
                              " for this child, I guess I'm keeping this one...");
                    } else {
                        player->money -= p;
                        print("You spent $" + std::to_string(p) + "...");
                        player->give(new Battler(player->team[0]));
                    }
                }

                canMap = oldCanMap;

                if (n->dead) {
                    pm->npcs[nIndex] = nullptr;
                } else {
                    playerX -= dx;
                    playerY -= dy;
                }
                offsetX = 0;
                offsetY = 0;
                return;
            }
            int sightIndex = playerY * pm->getGridCols() + playerX;
            if (pm->sight[sightIndex] != 0) {
                if (spinning) {
                    spinning = false;
                }
                Trainer *t = pm->getTrainer(playerX, playerY, pm->sight[sightIndex]);
                if (t) {
                    bool oldCanMap = canMap;
                    canMap = false;
                    print(t->getPhrase(0));
                    battling = true;
                    Battler** trainerTeam = t->createAllBattlers();
                    int result = BattleState::trainerBattle(player->team, t->type, trainerTeam);
                    battling = false;
                    // Clean up trainer team after battle
                    for (int i = 0; i < 6; i++) {
                        if (trainerTeam[i]) {
                            delete trainerTeam[i];
                        }
                    }
                    if (result < 0) {
                        blackout();
                    } else {
                        int r = (result + t->reward) * (player->hasItem(AMULET_COIN) ? 2 : 1);
                        player->money += r;
                        print("You got $" + std::to_string(r) + " for winning!");
                        print(t->getPhrase(1));
                        t->beat(player);
                        Trainer::E4Trainer *e4 = dynamic_cast<Trainer::E4Trainer*>(t);
                        if (e4 && e4->id == 8) {
                            Trainer::addEliteFour();
                        }
                        pm->deleteTrainer(t, pm->sight[sightIndex]);
                    }
                    offsetX = 0;
                    offsetY = 0;
                    canMap = oldCanMap;
                    return;
                }
            }
            if (--repelSteps == 0) {
                print("Your repel ran out!");
            } else {
                repelSteps = std::max(repelSteps, 0);
            }
            int type = pm->getTypeValue(playerY, playerX);
            if (type == 12 || type == 8 || type == 4) {
                if (utils::rand01() < 0.1) {
                    std::string encounterType = type == 12 ? "Tall Grass" : (type == 8 ? "Cave" : "Surfing");
                    bool oldCanMap = canMap;
                    canMap = false;
                    if(wildMon)
                        delete wildMon;
                    wildMon = pm->getRandomEncounter(encounterType);
                    if (wildMon && (repelSteps == 0 || (player->team[0] && wildMon->level >= player->team[0]->level))) {
                        int result = BattleState::wildBattle(player->team, wildMon);
                        if (result < 0) {
                            blackout();
                        } else {
                            player->money += result * (player->hasItem(AMULET_COIN) ? 2 : 1);
                        }
                        battling = false;
                        offsetX = 0;
                        offsetY = 0;
                        canMap = oldCanMap;
                        return;
                    }
                    canMap = oldCanMap;
                }
            }
        }
        offsetX = 0;
        offsetY = 0;
    }
}

void Gui::save() {
    if (pm->name == "RocketHideoutB1F" && playerX == 3) {
        printSpooky();
        print("You will not be saved.");
        return;
    }
    if (pm->name == "ChampionsRoom" || pm->name == "LancesRoom" || pm->name == "AgathasRoom" || pm->name == "BrunosRoom" ||
        pm->name == "LoreleisRoom") {
        printSpooky();
        print("You will not be saved.");
        return;
    }
    
    std::ofstream file("fat:/save.txt", std::ios::binary);
    if (!file.is_open()) {
        print("Failed to save!");
        return;
    }
    
    // Write player info line
    std::string line = pm->name + "," + std::to_string(playerX) + "," + std::to_string(playerY) + "," +
                      std::to_string(static_cast<int>(facing)) + "," + std::to_string(repelSteps) + "," + lastHeal->name + "," +
                      (player->ballin ? "1" : "0") + "," + std::to_string(player->numCaught) + "," + std::to_string(player->money) +
                      "," + player->name + "\n";
    file.write(line.c_str(), line.length());
    line.clear();
    // Write PC Pokemon line
    if (!player->pc.empty()) {
        player->pc[0]->append(line);
        for (size_t i = 1; i < player->pc.size(); i++) {
            line.push_back(';');
            player->pc[i]->append(line);
        }
        line.push_back('\n');
        file.write(line.c_str(), line.length());
        line.clear();
    } else {
        file.write("\n", 1);
    }
    // Write party Pokemon line
    if (player->team[0]) {
        player->team[0]->append(line);
        for (size_t i = 1; i < 6; i++) {
            if (!player->team[i]) {
                break;
            }
            line.push_back(';');
            player->team[i]->append(line);
        }
        line.push_back('\n');
        file.write(line.c_str(), line.length());
        line.clear();
    } else {
        file.write("\n", 1);
    }
    // Write boolean arrays
    auto writeBoolLine = [&](const auto &boolArray, int size) {
        for (int i = 0; i < size; i++) {
            line.push_back(boolArray[i] ? '1' : '0');
        }
        line.push_back('\n');
        file.write(line.c_str(), line.length());
        line.clear();
    };
    
    writeBoolLine(player->trainersBeaten, 309);
    writeBoolLine(player->leadersBeaten, 9);
    writeBoolLine(player->gioRivalsBeaten, 9);
    writeBoolLine(player->objectsCollected, 126);
    writeBoolLine(player->pokedex, 152);
    // Write items line
    bool firstItem = true;
    for (u8 i = 0; i < 80; i++) {//print(Item::ITEMS[i]->toString());
        if (Item::ITEMS[i]->quantity > 0) {
            if (!firstItem) {
                line += ";";
            }
            line += Item::ITEMS[i]->name + "," + std::to_string(Item::ITEMS[i]->quantity);
            firstItem = false;
        }
        else
            break;
    }
    line.push_back('\n');
    file.write(line.c_str(), line.length());
    line.clear();
    // Write TMs/HMs line
    for (u8 i = 0; i < 58; i++) {
        if (!player->tmHms[i])
            break;
        line += "," + player->tmHms[i]->name;
    }
    line.push_back('\n');
    file.write(line.c_str(), line.length());
    line.clear();
    // Write fly locations line
    for (size_t i = 1; i < 14; i++) {
        auto it = FlyLocation::FLY_LOCATIONS.find(FlyLocation::INDEX_MEANINGS[i]);
        line.push_back(it != FlyLocation::FLY_LOCATIONS.end() && it->second->visited ? '1' : '0');
    }
    file.write(line.c_str(), line.length());
    line.clear();
    
    file.close();
    print("Saved!");
}

void Gui::startFlying() {
    if (!player || !player->hasMove(FLY) || !player->leadersBeaten[2]) {
        print("You can't fly right now!");
        return;
    }
    flying = true;
    inMenu = true;
    currentLoc = printable(pm->name);
    inside = true;
    for (int i = 0; i < 11; i++) {
        for (int j = 0; j < 11; j++) {
            if (pm->name == FlyLocation::NAME_MAP[i][j]) {
                inside = false;
                return;
            }
        }
    }
}

bool Gui::useRevive(Item* it, Battler *b, int n) {
    if (b->hp > 0) {
        print("Quit messing around!");
        return false;
    } else {
        b->hp += n;
        print(b->nickname + "'s health was restored by " + std::to_string(n) + " points!");
        return player->use(it);
    }
}

bool Gui::healHp(Item* it, Battler *b, int n) {
    if (b->hp == 0) {
        print("No, you'll need something stronger to heal this one...");
        return false;
    } else if (b->hp == b->mhp) {
        print("Quit messing around!");
        return false;
    } else {
        b->hp = std::min(b->hp + n, b->mhp);
        print(b->nickname + "'s health was restored by " + std::to_string(n) + " points!");
        return player->use(it);
    }
}

bool Gui::useElixir(Item* it, Battler *b, int n) {
    bool used = false;
    for (size_t i = 0; i < 4; i++) {
        if (b->moves[i] == nullptr) {
            break;
        }
        if (b->pp[i] < b->mpp[i]) {
            b->pp[i] = std::min(b->pp[i] + n, static_cast<int>(b->mpp[i]));
            used = true;
        }
    }
    if (used) {
        print(b->nickname + "'s PP was restored!");
        return player->use(it);
    } else {
        print("DO NOT WASTE THAT!");
        return false;
    }
}

bool Gui::healStatus(Item* it, Battler *b, const std::string &s) {
    if (b->status == s) {
        if (playerState && playerState->monster == b) {
            playerState->poisonDamage = 0;
        }
        b->status.clear();
        print(b->nickname + "'s status was healed!");
        return player->use(it);
    } else {
        print("That wouldn't do anything!");
        return false;
    }
}

bool Gui::useRepel(Item* it, int steps) {
    repelSteps = std::max(repelSteps, 0) + steps;
    print("You gained " + std::to_string(steps - 1) + " repel steps!");
    return player->use(it);
}

bool Gui::catchMon(Item* it, std::string n, double bm) {
    int thp = 3 * wildMon->mhp;
    int sb = 0;
    if (wildMon->status == "ASLEEP" || wildMon->status == "FROZEN") {
        sb = 10;
    } else if (wildMon->status == "POISONED" || wildMon->status == "BURNED" || wildMon->status == "PARALYZED") {
        sb = 5;
    }
    print("You threw the " + n + "...");
    int rate = CATCH_RATES[wildMon->dexNum];
    int chance = std::max(((thp - 2 * wildMon->hp) * rate * bm) / thp, 1.0) + sb;
    if (utils::randInt(0, 256) <= chance) {
        battling = false;
        print("Oh baby! You caught it!");
        wildMon->nickname = wildMon->name;
        player->give(wildMon);
        wildMon = nullptr;
    } else {
        print("But it broke out!");
    }
    return player->use(it);
}

void Gui::blackout() {
    Trainer::addEliteFour();
    loadMap(lastHeal);
    playerX = lastHeal->healX;
    playerY = lastHeal->healY;
    int n = player->money / 2;
    player->money -= n;
    player->healTeam();
    print("You dropped $" + std::to_string(n) + "!");
}

void Gui::drawRainbowText(u16* framebuffer, const std::string &text, int x, int y) {
    std::string t = text;
    size_t p = 0;
    while ((p = t.find("=", p)) != std::string::npos) {
        t.erase(p, std::string("=").size());
    }
    
    for (unsigned char ch : t) {
        u16 color = rand() | BIT(15);
        drawChar(ch, x, y, color, framebuffer);
        x += 8;
    }
}