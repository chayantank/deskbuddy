#pragma once
// ============================================================================
// DeskBuddy — Reaction Time Game
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"
#include "storage.h"
#include "face_renderer.h"
#include "touch_input.h"

class GameReaction {
public:
    GameReaction(DisplayManager* dm, Storage* storage, FaceRenderer* face);
    
    void onEnter();
    void update();
    void render();
    void handleTouch(TouchEvent event);
    
private:
    DisplayManager* _dm;
    Storage* _storage;
    FaceRenderer* _face;
    
    int _state = 0; // 0=start, 1=waiting(red), 2=ready(green), 3=result, 4=early jump
    
    int _bestTime = 9999;
    int _lastTime = 0;
    
    unsigned long _stateStartTime = 0;
    unsigned long _targetTime = 0;
};
