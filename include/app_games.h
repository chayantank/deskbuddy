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
class GameFlappy;
class GameStack;
class GameReaction;

enum class GameID {
    NONE = -1,
    DINO = 0,
    FLAPPY = 1,
    STACK = 2,
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
    float _smoothScrollOffset = 0.0f;
    
    // Game instances (dynamically allocated to save RAM)
    GameDino* _dino = nullptr;
    GameFlappy* _flappy = nullptr;
    GameStack* _stack = nullptr;
    GameReaction* _reaction = nullptr;
    
    void _renderMenu();
    void _startGame(GameID id);
    void _exitCurrentGame();
};
