#pragma once
// ============================================================================
// DeskBuddy — Dino Runner Game
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "storage.h"
#include "face_renderer.h"
#include "touch_input.h"

class GameDino {
public:
    GameDino(DisplayManager* dm, Storage* storage, FaceRenderer* face);
    
    void onEnter();
    void update();
    void render();
    void handleTouch(TouchEvent event);
    
    bool isGameOver() const { return _state == 2; }
    
private:
    DisplayManager* _dm;
    Storage* _storage;
    FaceRenderer* _face;
    
    int _state = 0; // 0 = start, 1 = playing, 2 = game over
    
    int _score = 0;
    int _hiScore = 0;
    
    // Dino state
    float _dinoY = 0;
    float _dinoVY = 0;
    bool _isJumping = false;
    unsigned long _animTimer = 0;
    bool _animFrame = false;
    
    // Obstacles (cacti and birds)
    struct Obstacle {
        float x;
        int type; // 0 = cactus, 1 = bird
        float y;
        bool active;
    };
    
    static const int MAX_OBSTACLES = 3;
    Obstacle _obs[MAX_OBSTACLES];
    float _speed = 2.0f;
    
    void _spawnObstacle(int index);
    bool _checkCollision();
    void _gameOver();
};
