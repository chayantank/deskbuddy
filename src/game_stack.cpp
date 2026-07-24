// ============================================================================
// DeskBuddy — Stack Tower Implementation (1-Tap Arcade)
// ============================================================================
#include "game_stack.h"

GameStack::GameStack(DisplayManager* dm, Storage* storage, FaceRenderer* face)
    : _dm(dm), _storage(storage), _face(face) {
}

void GameStack::onEnter() {
    if (_storage) {
        _highScore = _storage->getStackHighScore();
    }
    _resetGame();
}

void GameStack::_resetGame() {
    _score = 0;
    _state = State::READY;
    _currW = 44.0f;
    _currX = (128.0f - _currW) / 2.0f;
    _speed = 1.6f;
    _dir = 1.0f;
    
    // Base platform layer
    _stack[0] = { (128.0f - _currW) / 2.0f, _currW };
    _stackHeight = 1;
}

void GameStack::handleTouch(TouchEvent event) {
    if (event == TouchEvent::TAP) {
        if (_state == State::READY) {
            _state = State::PLAYING;
        } else if (_state == State::PLAYING) {
            _placeBlock();
        } else if (_state == State::GAME_OVER) {
            _resetGame();
        }
    }
}

void GameStack::_placeBlock() {
    Block prev = _stack[_stackHeight - 1];
    
    float diff = _currX - prev.x;
    float overlapW = prev.w - abs(diff);
    
    if (overlapW <= 1.0f) {
        // Complete miss — Game Over!
        _state = State::GAME_OVER;
        if (_score > _highScore) {
            _highScore = _score;
            if (_storage) _storage->setStackHighScore(_highScore);
        }
        if (_face) _face->onGameOver();
        return;
    }
    
    // Trim block to overlap width
    float newX = (_currX > prev.x) ? _currX : prev.x;
    _currW = overlapW;
    
    // Shift stack down if full
    if (_stackHeight >= MAX_STACK) {
        for (int i = 0; i < MAX_STACK - 1; i++) {
            _stack[i] = _stack[i + 1];
        }
        _stack[MAX_STACK - 1] = { newX, _currW };
    } else {
        _stack[_stackHeight] = { newX, _currW };
        _stackHeight++;
    }
    
    _score++;
    _speed = min(_speed + 0.1f, 4.0f);
    _currX = 0;
    _dir = 1.0f;
}

void GameStack::update() {
    if (_state != State::PLAYING) return;
    
    _currX += _dir * _speed;
    if (_currX <= 0) {
        _currX = 0;
        _dir = 1.0f;
    } else if (_currX + _currW >= SCREEN_W) {
        _currX = SCREEN_W - _currW;
        _dir = -1.0f;
    }
}

void GameStack::render() {
    if (!_dm) return;
    Adafruit_SSD1306& d = _dm->display();
    
    int blockH = 6;
    int baseY = 56;
    
    // Draw stacked blocks
    for (int i = 0; i < _stackHeight; i++) {
        int y = baseY - i * (blockH + 1);
        int x = (int)_stack[i].x;
        int w = (int)_stack[i].w;
        d.fillRoundRect(x, y, w, blockH, 1, SSD1306_WHITE);
    }
    
    // Draw active moving block
    if (_state == State::PLAYING) {
        int currY = baseY - _stackHeight * (blockH + 1);
        d.drawRoundRect((int)_currX, currY, (int)_currW, blockH, 1, SSD1306_WHITE);
        d.fillRoundRect((int)_currX + 1, currY + 1, (int)_currW - 2, blockH - 2, 1, SSD1306_WHITE);
    }
    
    // Score
    d.setFont();
    d.setTextSize(1);
    d.setCursor(2, 2);
    d.print(_score);
    
    if (_state == State::READY) {
        _dm->drawCenteredText("STACK TOWER", 14, 1);
        _dm->drawCenteredText("Tap to Drop", 32, 1);
        _dm->drawCenteredText("Dbl-Tap: Exit", 46, 1);
    } else if (_state == State::GAME_OVER) {
        _dm->drawCenteredText("GAME OVER", 14, 1);
        char buf[24];
        snprintf(buf, sizeof(buf), "Blocks: %d  Hi: %d", _score, _highScore);
        _dm->drawCenteredText(buf, 30, 1);
        _dm->drawCenteredText("Tap to Retry", 44, 1);
        _dm->drawCenteredText("Dbl-Tap: Exit", 54, 1);
    }
}
