// ============================================================================
// DeskBuddy — Pong Game Implementation
// ============================================================================
#include "game_pong.h"

GamePong::GamePong(DisplayManager* dm, Storage* storage, FaceRenderer* face)
    : _dm(dm), _storage(storage), _face(face) {
}

void GamePong::onEnter() {
    _state = 0;
    _hiScore = _storage ? _storage->getPongHighScore() : 0;
    
    if (_face) _face->setExpression(Expression::PLAYFUL, 200);
}

void GamePong::_resetBall() {
    _ballX = 20;
    _ballY = SCREEN_H / 2.0f;
    
    // Random launch angle
    float angle = (random(-45, 46)) * (M_PI / 180.0f);
    float speed = 2.0f * _speedMultiplier;
    
    _ballVX = cosf(angle) * speed;
    _ballVY = sinf(angle) * speed;
    
    // Ensure it goes right
    if (_ballVX < 0) _ballVX = -_ballVX;
}

void GamePong::update() {
    if (_state != 1) return;
    
    // Move ball
    _ballX += _ballVX;
    _ballY += _ballVY;
    
    // Top/Bottom bounce
    if (_ballY <= 0) {
        _ballY = 0;
        _ballVY = -_ballVY;
    } else if (_ballY >= SCREEN_H - 2) {
        _ballY = SCREEN_H - 2;
        _ballVY = -_ballVY;
    }
    
    // Left bounce (wall)
    if (_ballX <= 0) {
        _ballX = 0;
        _ballVX = -_ballVX;
    }
    
    // Right side (Paddle / Score / Miss)
    if (_ballX >= SCREEN_W - 6) {
        // Check paddle hit
        if (_ballY >= _paddleY - 2 && _ballY <= _paddleY + _paddleHeight + 2) {
            _ballX = SCREEN_W - 6;
            _ballVX = -_ballVX;
            
            // Adjust angle based on where it hit paddle
            float hitPos = (_ballY - _paddleY) / _paddleHeight; // 0.0 to 1.0
            _ballVY = (hitPos - 0.5f) * 4.0f; 
            
            _score++;
            if (_score % 5 == 0) _speedMultiplier += 0.2f;
            
            // Speed up
            float speed = 2.0f * _speedMultiplier;
            float mag = sqrtf(_ballVX*_ballVX + _ballVY*_ballVY);
            _ballVX = (_ballVX / mag) * speed;
            _ballVY = (_ballVY / mag) * speed;
            
        } else if (_ballX > SCREEN_W) {
            // Missed!
            _state = 2; // Game over
            if (_score > _hiScore) {
                _hiScore = _score;
                if (_storage) _storage->setPongHighScore(_hiScore);
            }
            if (_face) _face->onGameOver();
        }
    }
}

void GamePong::handleTouch(TouchEvent event) {
    if (event == TouchEvent::TAP) {
        if (_state == 0) {
            _state = 1;
            _score = 0;
            _speedMultiplier = 1.0f;
            _paddleY = (SCREEN_H - _paddleHeight) / 2.0f;
            _resetBall();
        } else if (_state == 1) {
            // Tap moves paddle to bottom half or top half
            // Or toggles position
            if (_paddleY < SCREEN_H / 2) {
                _paddleY = SCREEN_H - _paddleHeight - 2;
            } else {
                _paddleY = 2;
            }
        } else if (_state == 2) {
            _state = 0;
            if (_face) _face->setExpression(Expression::PLAYFUL, 200);
        }
    }
}

void GamePong::render() {
    Adafruit_SSD1306& d = _dm->display();
    
    // Draw ball
    if (_state == 1 || _state == 2) {
        d.fillRect((int)_ballX, (int)_ballY, 3, 3, SSD1306_WHITE);
    }
    
    // Draw paddle
    if (_state != 0) {
        d.fillRect(SCREEN_W - 4, (int)_paddleY, 3, _paddleHeight, SSD1306_WHITE);
    }
    
    // Center dashed line
    for (int i = 0; i < SCREEN_H; i += 6) {
        d.drawLine(SCREEN_W/2, i, SCREEN_W/2, i+3, SSD1306_WHITE);
    }
    
    // Score
    d.setTextSize(1);
    char buf[8];
    snprintf(buf, sizeof(buf), "%d", _score);
    d.setCursor(SCREEN_W/2 - 20, 4);
    d.print(buf);
    
    snprintf(buf, sizeof(buf), "%d", _hiScore);
    d.setCursor(SCREEN_W/2 + 10, 4);
    d.print(buf);
    
    // Overlays
    if (_state == 0) {
        d.fillRoundRect(24, 20, 80, 24, 4, SSD1306_BLACK);
        d.drawRoundRect(24, 20, 80, 24, 4, SSD1306_WHITE);
        _dm->drawCenteredText("SOLO PONG", 24, 1);
        _dm->drawCenteredText("Tap to play", 34, 1);
    } else if (_state == 2) {
        d.fillRoundRect(24, 20, 80, 24, 4, SSD1306_BLACK);
        d.drawRoundRect(24, 20, 80, 24, 4, SSD1306_WHITE);
        _dm->drawCenteredText("GAME OVER", 24, 1);
        _dm->drawCenteredText("Tap to reset", 34, 1);
    }
}
