// ============================================================================
// DeskBuddy — Dino Runner Game Implementation
// ============================================================================
#include "game_dino.h"
#include "icons.h"

#define GROUND_Y 54
#define DINO_X 20

GameDino::GameDino(DisplayManager* dm, Storage* storage, FaceRenderer* face) 
    : _dm(dm), _storage(storage), _face(face) {
}

void GameDino::onEnter() {
    _state = 0;
    _hiScore = _storage ? _storage->getDinoHighScore() : 0;
    
    if (_face) _face->setExpression(Expression::PLAYFUL, 200);
}

void GameDino::update() {
    if (_state != 1) return; // Only update while playing
    
    unsigned long now = millis();
    
    // Animation
    if (now - _animTimer > (100 / _speed)) {
        _animFrame = !_animFrame;
        _animTimer = now;
        
        // Increase score occasionally
        if (_animFrame) {
            _score++;
            if (_score % 100 == 0) _speed += 0.2f;
        }
    }
    
    // Physics
    if (_isJumping) {
        _dinoVY += 0.4f; // gravity
        _dinoY += _dinoVY;
        
        if (_dinoY >= GROUND_Y) {
            _dinoY = GROUND_Y;
            _isJumping = false;
            _dinoVY = 0;
        }
    } else {
        _dinoY = GROUND_Y;
    }
    
    // Move obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (_obs[i].active) {
            _obs[i].x -= _speed;
            
            if (_obs[i].x < -20) {
                _obs[i].active = false;
            }
        }
    }
    
    // Spawn new obstacles
    bool hasSpace = true;
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (_obs[i].active && _obs[i].x > SCREEN_W - 50) {
            hasSpace = false;
            break;
        }
    }
    
    if (hasSpace && random(100) < 5) { // 5% chance per frame when space is clear
        for (int i = 0; i < MAX_OBSTACLES; i++) {
            if (!_obs[i].active) {
                _spawnObstacle(i);
                break;
            }
        }
    }
    
    // Check collisions
    if (_checkCollision()) {
        _gameOver();
    }
}

void GameDino::_spawnObstacle(int index) {
    _obs[index].active = true;
    _obs[index].x = SCREEN_W;
    
    // Type: 0 = cactus (ground), 1 = bird (air)
    if (_score > 200 && random(10) > 6) {
        _obs[index].type = 1;
        _obs[index].y = GROUND_Y - 20 - random(15); // Birds fly at different heights
    } else {
        _obs[index].type = 0;
        _obs[index].y = GROUND_Y;
    }
}

bool GameDino::_checkCollision() {
    // Simple AABB collision
    // Dino hitbox
    int dx = DINO_X + 4;
    int dy = (int)_dinoY - 14;
    int dw = 8;
    int dh = 14;
    
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (!_obs[i].active) continue;
        
        int ox = (int)_obs[i].x;
        int oy = (int)_obs[i].y;
        int ow, oh;
        
        if (_obs[i].type == 0) { // Cactus
            ow = 8; oh = 16;
            oy -= 16;
            ox += 4; ow -= 4; // tighten hitbox
        } else { // Bird
            ow = 16; oh = 8;
            oy -= 8;
            ox += 2; ow -= 4; dy += 4; dh -= 4; // tighten hitboxes
        }
        
        if (dx < ox + ow && dx + dw > ox &&
            dy < oy + oh && dy + dh > oy) {
            return true;
        }
    }
    return false;
}

void GameDino::_gameOver() {
    _state = 2;
    if (_score > _hiScore) {
        _hiScore = _score;
        if (_storage) _storage->setDinoHighScore(_hiScore);
    }
    if (_face) _face->onGameOver(); // dizzy
}

void GameDino::handleTouch(TouchEvent event) {
    if (event == TouchEvent::TAP) {
        if (_state == 0) {
            // Start game
            _state = 1;
            _score = 0;
            _speed = 3.0f;
            for (int i=0; i<MAX_OBSTACLES; i++) _obs[i].active = false;
            _dinoY = GROUND_Y;
            _isJumping = false;
        } else if (_state == 1) {
            // Jump
            if (!_isJumping) {
                _isJumping = true;
                _dinoVY = -5.5f;
            }
        } else if (_state == 2) {
            // Reset to start screen
            _state = 0;
            if (_face) _face->setExpression(Expression::PLAYFUL, 200);
        }
    }
}

void GameDino::render() {
    Adafruit_SSD1306& d = _dm->display();
    
    // Draw ground
    d.drawLine(0, GROUND_Y, SCREEN_W, GROUND_Y, SSD1306_WHITE);
    
    // Ground texture (simple dots based on time/score to feel like movement)
    int offset = (_score * 2) % 20;
    for (int i = 0; i < SCREEN_W; i += 20) {
        int x = i - offset;
        if (x >= 0 && x < SCREEN_W) {
            d.drawPixel(x, GROUND_Y + 2, SSD1306_WHITE);
        }
    }
    
    // Draw dino
    int dy = (int)_dinoY - 16;
    if (_isJumping || _state != 1) {
        d.drawBitmap(DINO_X, dy, sprite_dino_jump_16, 16, 16, SSD1306_WHITE);
    } else {
        if (_animFrame) {
            d.drawBitmap(DINO_X, dy, sprite_dino_jump_16, 16, 16, SSD1306_WHITE); // Use jump sprite for one run frame
        } else {
            d.drawBitmap(DINO_X, dy, sprite_dino_16, 16, 16, SSD1306_WHITE);
        }
    }
    
    // Draw obstacles
    for (int i = 0; i < MAX_OBSTACLES; i++) {
        if (_obs[i].active) {
            if (_obs[i].type == 0) {
                d.drawBitmap((int)_obs[i].x, (int)_obs[i].y - 16, sprite_cactus_8, 8, 16, SSD1306_WHITE);
            } else {
                // Bird flaps wing
                if (_animFrame) {
                    d.drawBitmap((int)_obs[i].x, (int)_obs[i].y - 8, sprite_bird_16, 16, 8, SSD1306_WHITE);
                } else {
                    d.drawBitmap((int)_obs[i].x, (int)_obs[i].y - 6, sprite_bird_16, 16, 8, SSD1306_WHITE);
                }
            }
        }
    }
    
    // Draw scores
    d.setTextSize(1);
    char buf[16];
    snprintf(buf, sizeof(buf), "%05d", _score);
    d.setCursor(SCREEN_W - 35, 2);
    d.print(buf);
    
    snprintf(buf, sizeof(buf), "HI %05d", _hiScore);
    d.setCursor(SCREEN_W - 90, 2);
    d.print(buf);
    
    // Overlays
    if (_state == 0) {
        d.fillRoundRect(16, 16, 96, 32, 4, SSD1306_BLACK);
        d.drawRoundRect(16, 16, 96, 32, 4, SSD1306_WHITE);
        _dm->drawCenteredText("DINO RUN", 20, 1);
        _dm->drawCenteredText("Tap to Jump", 29, 1);
        _dm->drawCenteredText("Dbl-Tap: Exit", 38, 1);
    } else if (_state == 2) {
        d.fillRoundRect(16, 16, 96, 32, 4, SSD1306_BLACK);
        d.drawRoundRect(16, 16, 96, 32, 4, SSD1306_WHITE);
        _dm->drawCenteredText("GAME OVER", 20, 1);
        _dm->drawCenteredText("Tap to Retry", 29, 1);
        _dm->drawCenteredText("Dbl-Tap: Exit", 38, 1);
    }
}
