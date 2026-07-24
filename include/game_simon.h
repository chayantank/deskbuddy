#pragma once
// ============================================================================
// DeskBuddy — Simon Says Game
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "storage.h"
#include "face_renderer.h"
#include "touch_input.h"

class GameSimon {
public:
    GameSimon(DisplayManager* dm, Storage* storage, FaceRenderer* face);
    
    void onEnter();
    void update();
    void render();
    void handleTouch(TouchEvent event);
    
private:
    DisplayManager* _dm;
    Storage* _storage;
    FaceRenderer* _face;
    
    int _state = 0; // 0 = start, 1 = showing sequence, 2 = player turn, 3 = game over
    
    int _score = 0;
    int _hiScore = 0;
    
    static const int MAX_SEQ = 50;
    uint8_t _sequence[MAX_SEQ];
    int _seqLength = 0;
    
    // Playback state
    int _playIndex = 0;
    unsigned long _lastPlayTime = 0;
    bool _isShowingFlash = false;
    
    // Player input state
    int _playerIndex = 0;
    int _currentTaps = 0; // Number of taps for this input
    unsigned long _lastTapTime = 0;
    
    void _addStep();
    void _drawQuadrant(int quad, bool active);
};
