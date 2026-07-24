#pragma once
// ============================================================================
// DeskBuddy — Pong (Solo) Game
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "storage.h"
#include "face_renderer.h"
#include "touch_input.h"

class GamePong {
public:
    GamePong(DisplayManager* dm, Storage* storage, FaceRenderer* face);
    
    void onEnter();
    void update();
    void render();
    void handleTouch(TouchEvent event);
    
private:
    DisplayManager* _dm;
    Storage* _storage;
    FaceRenderer* _face;
    
    int _state = 0; // 0 = start, 1 = playing, 2 = game over
    
    int _score = 0;
    int _hiScore = 0;
    
    // Ball state
    float _ballX = 0;
    float _ballY = 0;
    float _ballVX = 0;
    float _ballVY = 0;
    
    // Paddle state (right side)
    float _paddleY = 0;
    int _paddleHeight = 16;
    
    float _speedMultiplier = 1.0f;
    
    void _resetBall();
};
