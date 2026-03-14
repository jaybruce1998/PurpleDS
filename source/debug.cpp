// Debug utilities for DS ROM
#include "debug.h"
#include "Gui.h"
#include "font_data.hpp"
//#include <stdio.h>//printf, fprintf

// External framebuffer
extern u16 bottom_fb[256*192];

// External functions
void copyBuffers();

void debugCheckpoint(const char *stepName) {
    // Show and wait for A button with proper input handling
    bool waiting = true;
    //drawTile(0, 0, 0); INIT THE GRAYSCALE ARRAY DUMMY!!
    g_gui->fillRectangle(bottom_fb, WHITE, 0, 0, 256, 192);
    drawText(stepName, 0, 0, BLACK, bottom_fb);
    //fprintf(stderr, "%s\n", stepName);
    //fflush(stderr);
    copyBuffers();
    while (waiting) {
        
        scanKeys();
        u32 kDown = keysDown();
        if(kDown & KEY_A) {
            waiting = false;
        }
        
        // Wait for VBlank
        swiWaitForVBlank();
    }
}
