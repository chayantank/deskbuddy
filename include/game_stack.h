#pragma once
// ============================================================================
// DeskBuddy — Stack Tower Game (1-Tap Precision Arcade)
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "storage.h"
#include "face_renderer.h"
#include "touch_input.h"

class GameStack {
public:
    GameStack(DisplayManager* dm, Storage* storage, FaceRenderer* face);
    
    void onEnter();
    void update();
    void render();
    void handleTouch(TouchEvent event);

private:
    DisplayManager* _dm = nullptr;
    Storage* _storage = nullptr;
    FaceRenderer* _face = nullptr;
    
    enum class State { READY, PLAYING, GAME_OVER };
    State _state = State::READY;
    
    // Tower stack blocks
    struct Block {
        float x;
        float w;
    };
    
    static const int MAX_STACK = 8;
    Block _stack[MAX_STACK]; // visible stack layers
    int _stackHeight = 0;
    
    // Active moving block
    float _currX = 0;
    float _currW = 40;
    float _dir = 1.0f;
    float _speed = 1.5f;
    
    int _score = 0;
    int _highScore = 0;
    
    void _resetGame();
    void _placeBlock();
};
