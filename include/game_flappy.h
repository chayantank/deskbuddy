#pragma once
// ============================================================================
// DeskBuddy — Flappy Buddy Game (1-Tap Arcade)
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "storage.h"
#include "face_renderer.h"
#include "touch_input.h"

class GameFlappy {
public:
    GameFlappy(DisplayManager* dm, Storage* storage, FaceRenderer* face);
    
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
    
    // Bird physics
    float _birdY = 32.0f;
    float _birdVel = 0.0f;
    const float GRAVITY = 0.35f;
    const float JUMP_IMPULSE = -3.8f;
    
    // Pipe obstacles
    struct Pipe {
        float x;
        int gapY;
        int gapH;
        bool passed;
    };
    Pipe _pipes[2];
    
    int _score = 0;
    int _highScore = 0;
    
    void _resetGame();
    void _spawnPipe(int idx, float x);
};
