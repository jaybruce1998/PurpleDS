#pragma once

#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <nds.h>

// SDL type definitions for compatibility
typedef struct {
    int type;
    union {
        struct {
            int sym;
            int repeat;
        } key;
        struct {
            int button;
            int x;
            int y;
        } button;
    };
} SDL_Event;

// SDL constants
#define SDL_KEYDOWN 1
#define SDL_KEYUP 2
#define SDL_MOUSEBUTTONDOWN 3
#define SDL_MOUSEBUTTONUP 4
#define SDL_BUTTON_LEFT 1
#define SDL_BUTTON_RIGHT 2

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
#define SDLK_x 25
#define SDLK_q 26
#define SDLK_0 27
#define SDLK_9 28
#define SDLK_COMMA 29
#define SDLK_o 30
#define SDLK_e 31
#define SDLK_SPACE 32
#define SDLK_p 33
#define SDLK_l 34
#define SDLK_c 35
#define SDLK_TAB 36
#define SDLK_KP_ENTER 37

#define BLACK RGB15(0, 0, 0) | BIT(15)
#define WHITE RGB15(31, 31, 31) | BIT(15)

class BattleState;
class Item;
class Player;
class PokeMap;
class Battler;
class Trainer;
class Npc;
class WorldObject;
class MartItem;
class Move;

class Gui {
public:
    bool battling;
    static bool canMap;
    bool choosingFromLongList;
    static bool rightClicked;
    static bool inMenu;
    static bool flying;
    static bool inside;
    static bool surfing;
    static bool spacebar;
    u8 currentMenu; // 0=Inventory, 1=TMs, 2=Pokemon, 3=Pokedex
    bool autoBattle;

    static int clickedChoice;
    static std::string currentLoc;
    std::string longArrHeader;
    BattleState* playerState = nullptr;
    BattleState* enemyState = nullptr;

    enum class Direction : u8 {
        NONE = 255,
        SOUTH = 0,
        NORTH = 1,
        WEST = 2,
        EAST = 3
    };

    enum class StepPhase { NONE, MOVING, LANDING };

    void print(const std::string &s);
    std::string promptText(const std::string &prompt);
    u8 promptNumber(const std::string &prompt);
    u8 waitForChoice(u8 maxChoice);
    u8 moveChoice(Battler* b);
    void setBattleStates(BattleState *a, BattleState *b);

    explicit Gui(Player *player);
    Gui(Player *player, const std::string &saveInfo);

    void closeMenus();
    void playGame();
    void fillRectangle(u16* fb, u16 color, int x, int y, int width, int height);

    // Public variables for battle inventory access
    std::string longArr[20];
    u8 psi;

    Player *player;

    void setPartyStrings(Battler** party);
    bool useBattleItem(u8 flag);

private:
    static void advanceText();
    void stopMovement();
    void applySaveInfo(const std::string &saveInfo);
    void setup();
    void loadMap(PokeMap *map);
    void loadTileImages();
    void loadPlayerSprites();
    void clickMouse(int mouseX, int mouseY, bool rightClick);
    void save();
    void startFlying();
    std::string printable(const std::string &s);
    bool canMove(Direction d);
    int elevate(const std::string &s, bool B, int m);
    char randomSpooky();
    void printSpooky();
    void advanceStep();
    void blackout();
    bool useRevive(Item* it, Battler *b, int n);
    bool healHp(Item* it, Battler *b, int n);
    bool useElixir(Item* it, Battler *b, int n);
    bool healStatus(Item* it, Battler *b, const std::string &s);
    bool useRepel(Item* it, int steps);
    bool catchMon(Item* it, std::string n, double bm);
    void drawTextBox(const std::string &text);
    void drawWrappedText(const std::string &text, int x, int y, int maxWidth);
    void drawRainbowText(u16* framebuffer, const std::string &text, int x, int y);
    void drawConnection(int dirIndex, PokeMap *conn, int offset, int camX, int camY);
    int getCurrentFrame() const;
    void renderBattle();
    void displayLongArr();
    void openMenu();
    void setMenuStrings();
    void setInventoryStrings();
    void setTMStrings();
    void setPokedexStrings();
    void checkInventory(u16 py);
    void checkTMs(u16 py);
    void checkPokedex(u16 py);
    void checkBattler(u16 py);
    bool useItem(u16 py, u8 flag);
    bool usePartyItem(Item* it, std::string n);
    bool useMoveItem(Item* it, std::string n, Battler* b);
    void useTm(u16 py);
    void sell();
    void buySell();
    void buy();
    void setBuySellStrings();
    void update();
    void render();
    void handleInput();
    void pressDirection(Direction d, bool pressed);
    void teach(u16 py, Move* tm);
    void deposit();
    void setDepWith();
    void depositWithdraw();
    void withdraw();
    void itemUpDown(u32 kd);
    void tmUpDown(u32 kd);
    void partyUpDown(u32 kd);
    void setPcStrings();

    PokeMap *pm = nullptr;
    PokeMap *lastHeal = nullptr;
    std::string mapName = "RedsHouse2F";
    PokeMap *connections[4]{nullptr, nullptr, nullptr, nullptr};
    int connOffsets[4]{0, 0, 0, 0};

    u8 playerX = 0;
    u8 playerY = 6;
    int repelSteps = 0;
    int boxNum = 0;
    int frames = 0;
    int phaseFrame = 0;
    int currentStepFrames = 8;
    int timesMoved = 0;
    u32 bet = 1;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    Direction facing = Direction::SOUTH;
    Direction heldDirection = Direction::NONE;
    StepPhase stepPhase = StepPhase::NONE;
    bool switchingMaps = false;
    bool spinning = false;
    bool jumping = false;

    bool blackjackActive = false;
    int blackjackBet = 0;
    std::vector<int> blackjackDeck;
    std::vector<int> blackjackPlayer;
    std::vector<int> blackjackDealer;
    bool blackjackHideFirst = true;
    bool blackjackFinished = false;
    bool blackjackPlayerWon = false;
    int blackjackTieFrames = 0;
    bool blackjackShowingResults = false;

    Battler* wildMon = nullptr;
    std::vector<MartItem>* martItems;

    std::vector<const u8*> danceFrames;
    std::vector<const u8*> oakFrames;
    std::vector<const u8*> monsterFrames;
    Npc *mattNpc = nullptr;
    Npc *oakNpc = nullptr;
    Trainer *monsterTrainer = nullptr;

    // Direction keys state as bitmask (more efficient than vector)
    u8 pressedKeys = 0;
};

// External variables for main_ds.cpp
extern u16 top_fb[];
extern u16 bottom_fb[];
extern u16* top_buffer;
extern u16* bottom_buffer;
extern Gui* g_gui;

// Global function declarations for main_ds.cpp
void initGraphics();
void initGrayscaleLUT();
void copyBuffers();
void drawBattler(u16* fb, int battlerIndex, int screenX, int screenY, bool player = false);