#include "Item.h"
#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <nds.h>
#define END std::string::npos

Item* Item::ITEMS[80];

Item* Item::getItem(const std::string& name) {
    for (u8 i = 0; i < 80; i++) {
        if (ITEMS[i]->name == name) {
            return ITEMS[i];
        }
    }
    return nullptr;
}

Item::Item(const std::vector<std::string> &a) {
    name = a[0];
    if(name == "Antidote" || name == "Awakening" || name == "Fresh Water"
        || name == "Lemonade" || name == "Soda Pop" || name == "Full Restore"
        || name.rfind("Heal") !=END || name.rfind("Elixir") !=END
        || name.rfind("Potion") !=END || name.rfind("Revive") !=END)
        flags=15;
    else if(name.rfind("Ether") !=END)
        flags=31;
    else if(name.rfind("X", 0) == 0 || name == "Dire Hit" || name == "Guard Spec.")
        flags=3;
    else if(name.rfind("Ball") !=END || name == "Poke Doll")
        flags=2;
    else if(name == "Escape Rope" ||name.rfind("Rod")!=END||name.rfind("Repel")!=END)
        flags=4;
    else if(name == "Calcium" || name == "Carbos" || name == "Iron" || name == "Protein"
        ||name=="HP Up"||name=="Zinc" || name == "Rare Candy"||name.rfind("Stone")!=END)
        flags=12;
    else if(name=="PP Up")
        flags=28;
    price = std::stoi(a[1]);
}

Item::Item(const Item &i) {
    name = i.name;
    price = i.price;
    quantity = i.quantity;
    flags = i.flags;
}

bool Item::equals(const Item &o) const {
    return name == o.name;
}

std::string Item::toString() const {
    return name + " x" + std::to_string(quantity);
}

void Item::buildItems() {
    const u8 count = 80;
    const size_t buffer_size = 256; // Buffer for item data lines
    
    // Open file directly (nitrofs should be auto-initialized by build system)
    FILE* file = fopen("nitro:/Item.txt", "r");
    if (!file) {
        return; // Failed to open file
    }
    
    // Allocate buffer on heap (not stack)
    char* buffer = (char*)malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return; // Failed to allocate memory
    }
    
    // Read exactly 80 lines using optimized for loop
    for (int i = 0; i < count; i++) {
        if (fgets(buffer, buffer_size, file)) {
            // Remove newline character if present
            buffer[strlen(buffer) - 1] = '\0';
            
            // Process item line - split by comma
            std::vector<std::string> a = utils::split(buffer, ',');
            ITEMS[i] = new Item(a);
        } else {
            // Failed to read line, break early
            break;
        }
    }
    
    // Clean up
    free(buffer);
    fclose(file);
}
