#pragma once
// ============================================================================
// DeskBuddy — Games Menu
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "storage.h"
#include "face_renderer.h"
#include "touch_input.h"

// Forward declarations
class GameDino;
class GamePong;
class GameSimon;
class GameReaction;

enum class GameID {
    NONE = -1,
    DINO = 0,
    PONG = 1,
    SIMON = 2,
    REACTION = 3,
    COUNT = 4
};

class AppGames {
public:
    AppGames();
    void begin(DisplayManager* dm, Storage* storage, FaceRenderer* face);
    ~AppGames();
    
    void onEnter();
    void onExit();
    void update();
    void render();
    void handleTouch(TouchEvent event);

private:
    DisplayManager* _dm = nullptr;
    Storage* _storage = nullptr;
    FaceRenderer* _face = nullptr;
    
    GameID _selectedGame = GameID::NONE;
    int _menuIndex = 0;
    
    // Game instances (dynamically allocated to save RAM when not in games)
    GameDino* _dino = nullptr;
    GamePong* _pong = nullptr;
    GameSimon* _simon = nullptr;
    GameReaction* _reaction = nullptr;
    
    void _renderMenu();
    void _startGame(GameID id);
    void _exitCurrentGame();
};
