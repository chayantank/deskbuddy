// ============================================================================
// DeskBuddy — Reaction Time Game Implementation
// ============================================================================
#include "game_reaction.h"

GameReaction::GameReaction(DisplayManager* dm, Storage* storage, FaceRenderer* face)
    : _dm(dm), _storage(storage), _face(face) {
}

void GameReaction::onEnter() {
    _state = 0;
    _bestTime = _storage ? _storage->getReactionBest() : 9999;
    if (_face) _face->setExpression(Expression::PLAYFUL, 200);
}

void GameReaction::update() {
    unsigned long now = millis();
    
    if (_state == 1) {
        if (now >= _targetTime) {
            _state = 2; // Green!
            _stateStartTime = now;
            if (_face) _face->setExpression(Expression::SURPRISED, 100);
        }
    }
}

void GameReaction::handleTouch(TouchEvent event) {
    if (event != TouchEvent::TAP) return;
    
    unsigned long now = millis();
    
    if (_state == 0 || _state == 3 || _state == 4) {
        // Start game
        _state = 1;
        _stateStartTime = now;
        _targetTime = now + random(2000, 5000); // Wait 2-5 seconds
        if (_face) _face->setExpression(Expression::NEUTRAL, 200);
    } else if (_state == 1) {
        // Tapped too early!
        _state = 4;
        if (_face) _face->setExpression(Expression::ANGRY, 200);
    } else if (_state == 2) {
        // Success! Record time
        _lastTime = now - _stateStartTime;
        _state = 3;
        
        if (_lastTime < _bestTime) {
            _bestTime = _lastTime;
            if (_storage) _storage->setReactionBest(_bestTime);
            if (_face) _face->setExpression(Expression::EXCITED, 200);
        } else {
            if (_face) _face->setExpression(Expression::HAPPY, 200);
        }
    }
}

void GameReaction::render() {
    Adafruit_SSD1306& d = _dm->display();
    
    d.setTextSize(1);
    
    if (_state == 0) {
        _dm->drawCenteredText("REACTION TEST", 4, 1);
        _dm->drawCenteredText("Wait for TAP!", 24, 1);
        _dm->drawCenteredText("Tap: Start", 42, 1);
        _dm->drawCenteredText("Dbl-Tap: Exit", 54, 1);
        
        if (_bestTime < 9999) {
            char buf[16];
            snprintf(buf, sizeof(buf), "Best: %d ms", _bestTime);
            _dm->drawCenteredText(buf, 14, 1);
        }
    } else if (_state == 1) {
        // Waiting
        d.fillRoundRect(14, 10, 100, 44, 10, SSD1306_WHITE);
        d.setTextColor(SSD1306_BLACK);
        d.setTextSize(2);
        _dm->drawCenteredText("WAIT...", 24, 2);
        d.setTextColor(SSD1306_WHITE);
    } else if (_state == 2) {
        // Ready
        // Invert screen for huge visual impact
        d.invertDisplay(true);
        d.fillRoundRect(14, 10, 100, 44, 10, SSD1306_WHITE);
        d.setTextColor(SSD1306_BLACK);
        d.setTextSize(2);
        _dm->drawCenteredText("TAP!!", 24, 2);
        d.setTextColor(SSD1306_WHITE);
    } else if (_state == 3) {
        d.invertDisplay(false); // Make sure it's back to normal
        
        char buf[16];
        snprintf(buf, sizeof(buf), "%d ms", _lastTime);
        d.setTextSize(2);
        _dm->drawCenteredText(buf, 10, 2);
        
        d.setTextSize(1);
        if (_lastTime <= _bestTime) {
            _dm->drawCenteredText("NEW BEST!", 32, 1);
        } else {
            _dm->drawCenteredText("Good job!", 32, 1);
        }
        _dm->drawCenteredText("Tap: Retry | Dbl-Tap: Exit", 52, 1);
        
    } else if (_state == 4) {
        d.invertDisplay(false);
        _dm->drawCenteredText("TOO EARLY!", 16, 2);
        _dm->drawCenteredText("Tap: Retry | Dbl-Tap: Exit", 48, 1);
    }
}
