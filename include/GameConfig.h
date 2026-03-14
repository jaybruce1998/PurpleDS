#pragma once

namespace GameConfig {
    // DS screen dimensions
    static constexpr int TOP_SCREEN_WIDTH = 256;
    static constexpr int TOP_SCREEN_HEIGHT = 192;
    static constexpr int BOTTOM_SCREEN_WIDTH = 256;
    static constexpr int BOTTOM_SCREEN_HEIGHT = 192;
    
    // For compatibility with original code
    static constexpr int WINDOW_HEIGHT = TOP_SCREEN_HEIGHT + BOTTOM_SCREEN_HEIGHT;
    static constexpr int WORLD_HEIGHT = TOP_SCREEN_HEIGHT;
    static constexpr int UI_HEIGHT = BOTTOM_SCREEN_HEIGHT;
    
    // Tile size (scaled for DS)
    static constexpr int TILE_SIZE = 16;  // Original 16x16, keep same for DS
    
    // Menu dimensions (scaled for DS)
    static constexpr int MENU_BOX_HEIGHT = 80;   // Scaled for DS screen
    static constexpr int MENU_BOX_MARGIN = 20;   // Scaled for DS screen
    static constexpr int MENU_OPTION_HEIGHT = 20; // Scaled for DS screen
    static constexpr int MENU_OPTION_START_OFFSET = 20;
    static constexpr int MENU_OPTION_COUNT = 4;
    static constexpr int LONG_LIST_START_Y = 40; // Scaled for DS screen
}
