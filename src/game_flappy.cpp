// ============================================================================
// DeskBuddy — Flappy Buddy Implementation (1-Tap Arcade)
// ============================================================================
#include "game_flappy.h"

GameFlappy::GameFlappy(DisplayManager* dm, Storage* storage, FaceRenderer* face) 
    : _dm(dm), _storage(storage), _face(face) {
}

void GameFlappy::onEnter() {
    if (_storage) {
        _highScore = _storage->getFlappyHighScore();
    }
    _resetGame();
}

void GameFlappy::_resetGame() {
    _birdY = 32.0f;
    _birdVel = 0.0f;
    _score = 0;
    _state = State::READY;
    
    _spawnPipe(0, 128 + 20);
    _spawnPipe(1, 128 + 20 + 75);
}

void GameFlappy::_spawnPipe(int idx, float x) {
    _pipes[idx].x = x;
    _pipes[idx].gapH = 26; // vertical gap size
    _pipes[idx].gapY = random(10, 64 - 10 - _pipes[idx].gapH);
    _pipes[idx].passed = false;
}

void GameFlappy::handleTouch(TouchEvent event) {
    if (event == TouchEvent::TAP) {
        if (_state == State::READY) {
            _state = State::PLAYING;
            _birdVel = JUMP_IMPULSE;
        } else if (_state == State::PLAYING) {
            _birdVel = JUMP_IMPULSE;
        } else if (_state == State::GAME_OVER) {
            _resetGame();
        }
    }
}

void GameFlappy::update() {
    if (_state != State::PLAYING) return;
    
    // Physics
    _birdVel += GRAVITY;
    _birdY += _birdVel;
    
    // Floor/Ceiling collision
    if (_birdY < 4.0f) {
        _birdY = 4.0f;
        _birdVel = 0;
    }
    if (_birdY > 58.0f) {
        // Died hitting ground
        _state = State::GAME_OVER;
        if (_score > _highScore) {
            _highScore = _score;
            if (_storage) _storage->setFlappyHighScore(_highScore);
        }
        if (_face) _face->onGameOver();
        return;
    }
    
    // Move pipes
    for (int i = 0; i < 2; i++) {
        _pipes[i].x -= 1.8f;
        
        // Pass pipe
        if (!_pipes[i].passed && _pipes[i].x < 24) {
            _pipes[i].passed = true;
            _score++;
        }
        
        // Recycle pipe
        if (_pipes[i].x < -16) {
            int other = (i + 1) % 2;
            _spawnPipe(i, _pipes[other].x + 75);
        }
        
        // Collision check (Bird width = 8, height = 8 centered at X=24)
        float bx = 24.0f;
        if (bx + 4 > _pipes[i].x && bx - 4 < _pipes[i].x + 14) {
            if (_birdY - 4 < _pipes[i].gapY || _birdY + 4 > _pipes[i].gapY + _pipes[i].gapH) {
                // Collided!
                _state = State::GAME_OVER;
                if (_score > _highScore) {
                    _highScore = _score;
                    if (_storage) _storage->setFlappyHighScore(_highScore);
                }
                if (_face) _face->onGameOver();
                return;
            }
        }
    }
}

void GameFlappy::render() {
    if (!_dm) return;
    Adafruit_SSD1306& d = _dm->display();
    
    // Draw bird (cute small pet head / circle with beak)
    int bx = 24;
    int by = (int)_birdY;
    d.fillCircle(bx, by, 4, SSD1306_WHITE);
    d.drawPixel(bx + 5, by, SSD1306_WHITE); // beak
    d.drawPixel(bx - 1, by - 1, SSD1306_BLACK); // eye
    
    // Draw pipes
    for (int i = 0; i < 2; i++) {
        int px = (int)_pipes[i].x;
        if (px + 14 < 0 || px > SCREEN_W) continue;
        
        // Top pipe
        d.fillRect(px, 0, 14, _pipes[i].gapY, SSD1306_WHITE);
        d.fillRect(px - 1, _pipes[i].gapY - 3, 16, 3, SSD1306_WHITE);
        
        // Bottom pipe
        int botY = _pipes[i].gapY + _pipes[i].gapH;
        d.fillRect(px, botY, 14, SCREEN_H - botY, SSD1306_WHITE);
        d.fillRect(px - 1, botY, 16, 3, SSD1306_WHITE);
    }
    
    // Score
    d.setFont();
    d.setTextSize(1);
    d.setCursor(2, 2);
    d.print(_score);
    
    if (_state == State::READY) {
        _dm->drawCenteredText("FLAPPY BUDDY", 16, 1);
        _dm->drawCenteredText("Tap to Flap", 36, 1);
        _dm->drawCenteredText("Dbl-Tap: Exit", 54, 1);
    } else if (_state == State::GAME_OVER) {
        _dm->drawCenteredText("GAME OVER", 14, 1);
        char buf[24];
        snprintf(buf, sizeof(buf), "Score: %d  Hi: %d", _score, _highScore);
        _dm->drawCenteredText(buf, 32, 1);
        _dm->drawCenteredText("Tap to Retry", 46, 1);
        _dm->drawCenteredText("Dbl-Tap: Exit", 56, 1);
    }
}
