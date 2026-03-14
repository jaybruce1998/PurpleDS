#include "MartItem.h"
#include "PokeMap.h"
#include "Utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

void MartItem::buildMartItems() {
    const int count = 33;
    const size_t buffer_size = 512; // Buffer for mart item data lines
    
    // Open file directly (nitrofs should be auto-initialized by build system)
    FILE* file = fopen("nitro:/MartItem.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    // Allocate buffer on heap (not stack)
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Read exactly 33 lines using optimized for loop
    for (int i = 0; i < count; i++) {
        if (fgets(buffer, buffer_size, file)) {
            // Remove newline character if present
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '\n') {
                buffer[len - 1] = '\0';
            }
            
            // Process mart item line - split by comma
            std::vector<std::string> a = utils::split(buffer, ',');
            if (a.size() > 0) {
                PokeMap::POKEMAPS[a[0]]->addMartItems(a);
            }
        } else {
            // Failed to read line, break early
            break;
        }
    }
    
    // Clean up
    free(buffer);
    fclose(file);
}

MartItem::MartItem(const std::string &s) {
    if (s.rfind("Move", 0) == 0) {
        size_t i = s.find_last_of(' ');
        move = Move::getMove(s.substr(5, i - 5));
        price = std::stoi(s.substr(i + 1));
    } else if (s.rfind("Pokem", 0) == 0) {
        size_t i = s.find_last_of(' ');
        mon = Monster::MONSTER_MAP[s.substr(8, i - 8)];
        price = std::stoi(s.substr(i + 1));
    } else {
        item = Item::getItem(s);
        price = item->price;
    }
}

std::string MartItem::toString() const {
    std::string n = move ? move->name : (item ? item->name : (mon ? mon->name : ""));
    return n + "   $" + std::to_string(price);
}
