// main_ds_clean.cpp - Clean DS map viewer
#include <nds.h>
#include <map_definitions.h>
#include <tiles_data.h>
#include <battlers_data.h>
#include <transparency_color.h>
#include <sprites_data.hpp>
#include <font_data.hpp>
#include <vector>
#include <string>
#include <algorithm>
#include <cstring>

static u16 top_buffer[256*192];
static u16 bottom_buffer[256*192];

// Map viewer state
static int currentMapIndex = 0;
static int mapScrollX = 0;
static int mapScrollY = 0;
static std::vector<std::string> mapNames;

// Battler viewer state
static int dexNum = 0;

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
void copyBuffers(u16* top_fb, u16* bottom_fb) {
    memcpy(top_fb, top_buffer, 256*192*2);
    memcpy(bottom_fb, bottom_buffer, 256*192*2);
}

// Draw a battler to bottom_buffer with transparency
void drawBattler(int battlerIndex, int screenX, int screenY) {
    // Handle Missingno (index 0) as special case
    if (dexNum==0) {
        // Generate random Missingno pixels
        for (int y = 0; y < 60; y++) {
            for (int x = 0; x < 60; x++) {
                int pos = (screenY + y) * 256 + (screenX + x);
                u16 pixel = rand() | BIT(15);
                bottom_buffer[pos] = pixel;
            }
        }
        return;
    }
    
    // Regular battler - get from array
    const u16* battler = BATTLER_DATA[dexNum-1];
    
    // Draw 60x60 battler
    for (int y = 0; y < 60; y++) {
        for (int x = 0; x < 60; x++) {
            int pos = (screenY + y) * 256 + (screenX + x);
            u16 pixel = battler[y * 60 + x];
            
            if (pixel != TRANSPARENCY_RGB15) {
                bottom_buffer[pos] = pixel;
            }
        }
    }
}

// Draw a tile to bottom_buffer
void drawTile(int tileId, int screenX, int screenY) {
    const u8* data=TILE_DATA[tileId];
    for (int y = std::max(0, -screenY), startX=std::max(0, -screenX), x, sy, sx; y < 16; y++) {
        sy=screenY+y;
        if(sy>191)
            return;
        sy*=256;
        for (x = startX; x < 16; x++) {
            sx=screenX+x;
            if(sx>255)
                break;
            top_buffer[sy + sx] = grayscaleLUT[data[y * 16 + x]];
        }
    }
}

// Draw a sprite to top_buffer with transparency
void drawSprite(const std::string& spriteName, int screenX, int screenY) {
    // Check if sprite exists
    if (SPRITE_DATA.find(spriteName) == SPRITE_DATA.end()) {
        return; // Sprite not found
    }
    
    const u16* sprite = SPRITE_DATA[spriteName];
    
    // Draw 16x16 sprite with transparency
    for (int y = 0; y < 16; y++) {
        for (int x = 0; x < 16; x++) {
            if (screenX + x < 256 && screenY + y < 192) {
                int pos = (screenY + y) * 256 + (screenX + x);
                u16 pixel = sprite[y * 16 + x];
                
                // Skip transparent pixels (cyan color)
                if (pixel != TRANSPARENCY_RGB15) {
                    top_buffer[pos] = pixel;
                }
            }
        }
    }
}

// Initialize map viewer
void initializeMapViewer() {
    // Get all map names
    for (const auto& pair : g_allMaps) {
        mapNames.push_back(pair.first);
    }
    
    if (mapNames.empty()) {
        mapNames.push_back("No Maps Available");
    }
}

// Draw map viewer and battler to temp buffers
void drawMapViewer() {
    // Clear temp buffers
    for (int i = 0; i < 256*192; i++) {
        top_buffer[i] = RGB15(0, 0, 0) | BIT(15);    // Black
        bottom_buffer[i] = RGB15(0, 0, 0) | BIT(15);    // Black
    }
    
    // Draw current battler centered on bottom screen
    int battlerX = (256 - 60) / 2;  // Center horizontally
    int battlerY = (192 - 60) / 2;  // Center vertically
    drawBattler(dexNum, battlerX, battlerY);
    
    // Get current map
    const std::string& mapName = mapNames[currentMapIndex];
    const auto& mapData = g_allMaps[mapName];
    
    // Draw map tiles on bottom screen with scrolling
    int mapHeight = mapData.size();
    int mapWidth = mapData[0].size();
    
    // Calculate visible area (16x12 tiles visible)
    int startY = mapScrollY / 16;  // Convert pixel scroll to tile coordinates
    int startX = mapScrollX / 16;
    int endY = std::min(startY + 13, mapHeight);
    int endX = std::min(startX + 17, mapWidth);
    
    // Calculate pixel offset for smooth scrolling
    int pixelOffsetY = mapScrollY % 16;
    int pixelOffsetX = mapScrollX % 16;
    
    // Draw tiles using baked-in tile data
    for (int y = std::max(0, startY), sx=std::max(0, startX), screenY; y < endY; y++) {
        screenY = (y - startY) * 16 - pixelOffsetY;
        for (int x = sx; x < endX; x++) {
            int tileId = mapData[y][x];
            int screenX = (x - startX) * 16 - pixelOffsetX;
            
            // Draw tile to top_buffer
            drawTile(tileId, screenX, screenY);
        }
    }
    
    // Draw "Hello, World!" text in green on bottom screen
    drawText("Hello, World!", 10, 10, RGB15(0, 31, 0)|BIT(15), bottom_buffer);
    
    // Draw RED_0 sprite centered on top screen (over tiles)
    int spriteX = (256 - 16) / 2;  // Center horizontally (16x16 sprite)
    int spriteY = (192 - 16) / 2;  // Center vertically (16x16 sprite)
    drawSprite("RED_0", spriteX, spriteY);
}

// Handle map viewer and battler input
void handleMapViewerInput() {
    scanKeys();
    u32 kDown = keysDown();
    u32 kHeld = keysHeld();
    
    // X button - next battler
    if (kDown & KEY_X) {
        dexNum = (dexNum + 1) % 153;
    }
    
    // Y button - previous battler  
    if (kDown & KEY_Y) {
        dexNum = (dexNum + 152) % 153;
    }
    
    // L/R buttons to change maps
    if (kDown & KEY_L) {
        currentMapIndex = (currentMapIndex - 1 + mapNames.size()) % mapNames.size();
        mapScrollX = 0;
        mapScrollY = 0;
    }
    if (kDown & KEY_R) {
        currentMapIndex = (currentMapIndex + 1) % mapNames.size();
        mapScrollX = 0;
        mapScrollY = 0;
    }
    
    if (kHeld & KEY_UP) {
        mapScrollY--;
    }
    if (kHeld & KEY_DOWN) {
        mapScrollY++;
    }
    if (kHeld & KEY_LEFT) {
        mapScrollX--;
    }
    if (kHeld & KEY_RIGHT) {
        mapScrollX++;
    }
}

int main(int argc, char **argv) {
    // Use Sudoku.c VRAM setup
    videoSetMode(MODE_5_2D);
    vramSetBankA(VRAM_A_MAIN_BG);
    videoSetModeSub(MODE_5_2D);
    vramSetBankC(VRAM_C_SUB_BG);
    
    int top_bg = bgInit(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    int bottom_bg = bgInitSub(2, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
    u16* top_fb = (u16*)bgGetGfxPtr(top_bg);
    u16* bottom_fb = (u16*)bgGetGfxPtr(bottom_bg);
    // 1) Call initializeAllMaps
    initializeAllMaps();
    
    // Initialize grayscale LUT
    initGrayscaleLUT();
    
    // Initialize map viewer
    initializeMapViewer();
    
    // Initialize sprites
    initializeSprites();
    
    // Initialize font
    initializeFont();
    
    // Main loop - simple buffer approach
    while (true) {
        // Handle input
        handleMapViewerInput();
        
        // Draw everything to temp buffers
        drawMapViewer();
        
        swiWaitForVBlank();
        copyBuffers(top_fb, bottom_fb);
    }
    
    return 0;
}
