#include "TmLearnsets.h"
#include "LevelUpMove.h"
#include "MartItem.h"
#include "Utils.h"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>

void TmLearnsets::buildTmLearnsets() {
    const u8 count = 152;
    const size_t buffer_size = 576; // Optimized for max line length (541) rounded up
    
    // Open file directly (nitrofs should be auto-initialized by build system)
    FILE* file = fopen("nitro:/TmLearnsets.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    // Allocate buffer on heap (not stack)
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Read exactly 152 lines using optimized for loop
    for (u8 i = 0; i < count; i++) {
        if (fgets(buffer, buffer_size, file)) {
            // Remove newline character if present
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            // Add moves to monster
            Monster::MONSTERS[i].addMoves(buffer);
        } else {
            // Failed to read line, break early
            break;
        }
    }
    
    // Clean up
    free(buffer);
    fclose(file);
}
