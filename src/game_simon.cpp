// ============================================================================
// DeskBuddy — Simon Says Game Implementation
// ============================================================================
#include "game_simon.h"

// Quadrants:
// 1 = Top-Left      (1 tap)
// 2 = Top-Right     (2 taps)
// 3 = Bottom-Left   (3 taps)
// 4 = Bottom-Right  (4 taps)

GameSimon::GameSimon(DisplayManager* dm, Storage* storage, FaceRenderer* face)
    : _dm(dm), _storage(storage), _face(face) {
}

void GameSimon::onEnter() {
    _state = 0;
    _hiScore = _storage ? _storage->getSimonHighScore() : 0;
    if (_face) _face->setExpression(Expression::PLAYFUL, 200);
}

void GameSimon::_addStep() {
    if (_seqLength < MAX_SEQ) {
        _sequence[_seqLength] = random(1, 5);
        _seqLength++;
    }
}

void GameSimon::update() {
    unsigned long now = millis();
    
    if (_state == 1) {
        // Showing sequence
        int delayTime = max(200, 600 - _seqLength * 15); // Speeds up as game progresses
        
        if (now - _lastPlayTime > delayTime) {
            _lastPlayTime = now;
            
            if (_isShowingFlash) {
                // Turn off flash, wait before next
                _isShowingFlash = false;
                
                if (_playIndex >= _seqLength) {
                    // Sequence done, player's turn
                    _state = 2;
                    _playerIndex = 0;
                    _currentTaps = 0;
                }
            } else {
                // Show next flash
                if (_playIndex < _seqLength) {
                    _isShowingFlash = true;
                    _playIndex++;
                }
            }
        }
    } else if (_state == 2) {
        // Player turn - wait for tap sequence to finish
        if (_currentTaps > 0 && now - _lastTapTime > 500) {
            // Tapping stopped, check input
            if (_currentTaps == _sequence[_playerIndex]) {
                // Correct!
                _playerIndex++;
                _currentTaps = 0;
                
                if (_playerIndex >= _seqLength) {
                    // Level complete
                    _score++;
                    _addStep();
                    _state = 1;
                    _playIndex = 0;
                    _isShowingFlash = false;
                    _lastPlayTime = now + 500; // Pause before next sequence
                }
            } else {
                // Wrong!
                _state = 3;
                if (_score > _hiScore) {
                    _hiScore = _score;
                    if (_storage) _storage->setSimonHighScore(_hiScore);
                }
                if (_face) _face->onGameOver();
            }
        }
    }
}

void GameSimon::handleTouch(TouchEvent event) {
    unsigned long now = millis();
    
    if (_state == 0) {
        if (event == TouchEvent::TAP) {
            _state = 1;
            _score = 0;
            _seqLength = 0;
            _addStep();
            _playIndex = 0;
            _isShowingFlash = false;
            _lastPlayTime = now;
        }
    } else if (_state == 2) {
        if (event == TouchEvent::TAP) {
            _currentTaps++;
            if (_currentTaps > 4) _currentTaps = 4; // Max 4 taps
            _lastTapTime = now;
        }
    } else if (_state == 3) {
        if (event == TouchEvent::TAP) {
            _state = 0;
            if (_face) _face->setExpression(Expression::PLAYFUL, 200);
        }
    }
}

void GameSimon::_drawQuadrant(int quad, bool active) {
    Adafruit_SSD1306& d = _dm->display();
    
    int hw = SCREEN_W / 2;
    int hh = SCREEN_H / 2;
    
    int x = (quad == 1 || quad == 3) ? 2 : hw + 2;
    int y = (quad == 1 || quad == 2) ? 2 : hh + 2;
    int w = hw - 4;
    int h = hh - 4;
    
    if (active) {
        d.fillRoundRect(x, y, w, h, 4, SSD1306_WHITE);
        // Draw tap count in inverse color
        d.setTextColor(SSD1306_BLACK);
        d.setCursor(x + w/2 - 2, y + h/2 - 4);
        d.print(quad);
        d.setTextColor(SSD1306_WHITE);
    } else {
        d.drawRoundRect(x, y, w, h, 4, SSD1306_WHITE);
        d.setCursor(x + w/2 - 2, y + h/2 - 4);
        d.print(quad);
    }
}

void GameSimon::render() {
    Adafruit_SSD1306& d = _dm->display();
    d.setTextSize(1);
    
    // Draw quadrants
    if (_state == 1) {
        int activeQuad = _isShowingFlash ? _sequence[_playIndex - 1] : 0;
        _drawQuadrant(1, activeQuad == 1);
        _drawQuadrant(2, activeQuad == 2);
        _drawQuadrant(3, activeQuad == 3);
        _drawQuadrant(4, activeQuad == 4);
    } else if (_state == 2) {
        // Show which quadrant player is currently selecting based on tap count
        _drawQuadrant(1, _currentTaps == 1);
        _drawQuadrant(2, _currentTaps == 2);
        _drawQuadrant(3, _currentTaps == 3);
        _drawQuadrant(4, _currentTaps == 4);
        
        // Show progress dots
        for (int i = 0; i < _seqLength; i++) {
            if (i < _playerIndex) {
                d.drawPixel(SCREEN_W/2 - (_seqLength*2) + i*4, SCREEN_H/2, SSD1306_WHITE);
            }
        }
    } else {
        _drawQuadrant(1, false);
        _drawQuadrant(2, false);
        _drawQuadrant(3, false);
        _drawQuadrant(4, false);
    }
    
    // Overlays
    if (_state == 0) {
        d.fillRoundRect(24, 20, 80, 24, 4, SSD1306_BLACK);
        d.drawRoundRect(24, 20, 80, 24, 4, SSD1306_WHITE);
        _dm->drawCenteredText("SIMON SAYS", 24, 1);
        _dm->drawCenteredText("Tap to play", 34, 1);
        
        char buf[16];
        snprintf(buf, sizeof(buf), "HI: %d", _hiScore);
        _dm->drawCenteredText(buf, 2, 1);
    } else if (_state == 3) {
        d.fillRoundRect(24, 20, 80, 24, 4, SSD1306_BLACK);
        d.drawRoundRect(24, 20, 80, 24, 4, SSD1306_WHITE);
        
        char buf[16];
        snprintf(buf, sizeof(buf), "SCORE: %d", _score);
        _dm->drawCenteredText(buf, 24, 1);
        _dm->drawCenteredText("Tap to reset", 34, 1);
    }
}
