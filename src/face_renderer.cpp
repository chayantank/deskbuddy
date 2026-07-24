// ============================================================================
// DeskBuddy — Face Renderer Implementation
// ============================================================================
#include "face_renderer.h"
#include "storage.h"
#include "easing.h"
#include <math.h>

// Default eye positions (centered on 128x64 screen)
#define LEFT_EYE_X    40.0f
#define RIGHT_EYE_X   88.0f
#define EYE_Y         26.0f
#define MOUTH_X       64.0f
#define MOUTH_Y       52.0f

void FaceRenderer::begin(DisplayManager* dm, Storage* storage) {
    _dm = dm;
    _storage = storage;
    if (_storage) {
        _happiness = _storage->getHappiness();
    } else {
        _happiness = 75;
    }
    
    _lastActivityTime = millis();
    _nextBlinkTime = millis() + random(BLINK_MIN_INTERVAL_MS, BLINK_MAX_INTERVAL_MS);
    _nextLookTime = millis() + random(1000, IDLE_LOOK_INTERVAL_MS);
    
    // Initialize base neutral params so eye dimensions are never zero!
    _setExpressionParams(Expression::NEUTRAL, _leftEye, _rightEye, _mouth, _extras);
    _targetLeftEye = _leftEye;
    _targetRightEye = _rightEye;
    _targetMouth = _mouth;
    _targetExtras = _extras;
    _transProgress = 1.0f;
    
    updateMood();
}

void FaceRenderer::setHappiness(uint8_t val) {
    if (val > 100) val = 100;
    _happiness = val;
    if (_storage) _storage->setHappiness(_happiness);
    updateMood();
}

void FaceRenderer::addHappiness(int delta) {
    int newH = (int)_happiness + delta;
    if (newH > 100) newH = 100;
    if (newH < 0) newH = 0;
    _happiness = (uint8_t)newH;
    if (_storage) _storage->setHappiness(_happiness);
    updateMood();
}

void FaceRenderer::updateMood() {
    if (_happiness >= 80) {
        // High happiness -> Happy / Love / Excited
        if (_targetExpr != Expression::LOVE && _targetExpr != Expression::PARTY && _targetExpr != Expression::EXCITED) {
            setExpression(Expression::HAPPY, 300);
        }
    } else if (_happiness >= 40) {
        // Neutral / Chill
        if (_targetExpr != Expression::CHILL && _targetExpr != Expression::PLAYFUL) {
            setExpression(Expression::NEUTRAL, 300);
        }
    } else {
        // Low happiness -> Sad / Skeptical / Pouty
        if (_targetExpr != Expression::ANGRY) {
            setExpression(Expression::SAD, 300);
        }
    }
}

const char* FaceRenderer::getExpressionName(Expression expr) const {
    switch (expr) {
        case Expression::NEUTRAL:   return "Neutral";
        case Expression::HAPPY:     return "Happy";
        case Expression::LOVE:      return "Love";
        case Expression::SLEEPY:    return "Sleepy";
        case Expression::SLEEPING:  return "Sleeping";
        case Expression::SURPRISED: return "Surprised";
        case Expression::ANGRY:     return "Angry";
        case Expression::SAD:       return "Sad";
        case Expression::CONFUSED:  return "Confused";
        case Expression::EXCITED:   return "Excited";
        case Expression::WINK:      return "Wink";
        case Expression::DIZZY:     return "Dizzy";
        case Expression::COOL:      return "Cool";
        case Expression::BLUSHING:  return "Blushing";
        case Expression::SKEPTICAL: return "Skeptical";
        case Expression::STARRY:    return "Starry";
        case Expression::PLAYFUL:   return "Playful";
        case Expression::CHILL:     return "Chill";
        case Expression::PARTY:     return "Party";
        default: return "Unknown";
    }
}

void FaceRenderer::setExpression(Expression expr, uint16_t transitionMs) {
    if (expr == _targetExpr && _transProgress >= 1.0f) return;
    
    _targetExpr = expr;
    _transDuration = transitionMs;
    _transStartTime = millis();
    _transProgress = 0.0f;
    
    // Snapshot current state as "from"
    // Target params set for interpolation
    _setExpressionParams(expr, _targetLeftEye, _targetRightEye, _targetMouth, _targetExtras);
}

void FaceRenderer::cycleNextExpression() {
    int next = ((int)_targetExpr + 1) % (int)Expression::COUNT;
    setExpression((Expression)next, 250);
    _toastStartTime = millis();
    addHappiness(5);
}

void FaceRenderer::_setExpressionParams(Expression expr, EyeParams& le, EyeParams& re, MouthParams& m, FaceExtras& ex) {
    // Reset all special flags
    le = {LEFT_EYE_X, EYE_Y, 18, 16, 0, 0, 5, 1.0f, 0, 0.3f, false, false, false, false};
    re = {RIGHT_EYE_X, EYE_Y, 18, 16, 0, 0, 5, 1.0f, 0, 0.3f, false, false, false, false};
    m = {MOUTH_X, MOUTH_Y, 20, 0.2f, 0, false, false, false};
    ex = {false, false, false, false, false, 0};
    
    switch (expr) {
        case Expression::NEUTRAL:
            // Defaults are already neutral
            break;
            
        case Expression::HAPPY:
            le.height = 12; le.squint = 0.4f; le.openness = 0.7f;
            re.height = 12; re.squint = 0.4f; re.openness = 0.7f;
            m.curve = 0.8f; m.width = 26;
            break;
            
        case Expression::LOVE:
            le.isHeart = true; le.width = 20; le.height = 18;
            re.isHeart = true; re.width = 20; re.height = 18;
            m.curve = 1.0f; m.width = 24;
            break;
            
        case Expression::SLEEPY:
            le.openness = 0.35f; le.y = EYE_Y + 3;
            re.openness = 0.35f; re.y = EYE_Y + 3;
            m.curve = 0.0f; m.openness = 0.6f; m.width = 12; // yawn
            break;
            
        case Expression::SLEEPING:
            le.openness = 0.0f;
            re.openness = 0.0f;
            m.curve = 0.0f; m.width = 8;
            ex.showZzz = true;
            break;
            
        case Expression::SURPRISED:
            le.width = 22; le.height = 22; le.pupilSize = 3;
            re.width = 22; re.height = 22; re.pupilSize = 3;
            m.curve = 0.0f; m.openness = 0.9f; m.width = 14;
            break;
            
        case Expression::ANGRY:
            le.squint = 0.5f; le.y = EYE_Y - 2; le.pupilSize = 4;
            re.squint = 0.5f; re.y = EYE_Y - 2; re.pupilSize = 4;
            // Angled eyebrows will be drawn by extra rendering
            m.curve = -0.6f; m.isZigzag = true; m.width = 22;
            break;
            
        case Expression::SAD:
            le.y = EYE_Y + 2; le.height = 14;
            re.y = EYE_Y + 2; re.height = 14;
            m.curve = -0.8f; m.width = 20;
            ex.showTear = true;
            break;
            
        case Expression::CONFUSED:
            le.width = 16; le.height = 16;
            re.width = 20; re.height = 12;
            le.pupilX = -0.3f;
            re.pupilX = 0.3f;
            m.isWavy = true; m.width = 22;
            break;
            
        case Expression::EXCITED:
            le.width = 20; le.height = 20; le.pupilSize = 6;
            re.width = 20; re.height = 20; re.pupilSize = 6;
            le.y = EYE_Y - 2; re.y = EYE_Y - 2;
            m.curve = 1.0f; m.openness = 0.5f; m.width = 28;
            ex.showExclaim = true;
            break;
            
        case Expression::WINK:
            le.openness = 1.0f;
            re.openness = 0.0f; // winking
            m.curve = 0.6f; m.width = 22;
            break;
            
        case Expression::DIZZY:
            le.isSpiral = true; le.width = 18; le.height = 18;
            re.isSpiral = true; re.width = 18; re.height = 18;
            m.isWavy = true; m.width = 18;
            break;
            
        case Expression::COOL:
            le.isCool = true; le.width = 22; le.height = 10; le.y = EYE_Y - 1;
            re.isCool = true; re.width = 22; re.height = 10; re.y = EYE_Y - 1;
            m.curve = 0.4f; m.width = 16; // smirk
            break;
            
        case Expression::BLUSHING:
            le.openness = 0.8f; le.squint = 0.2f;
            re.openness = 0.8f; re.squint = 0.2f;
            m.curve = 0.5f; m.width = 16;
            ex.showBlush = true; ex.blushIntensity = 1.0f;
            break;
            
        case Expression::SKEPTICAL:
            le.y = EYE_Y - 3; le.height = 12; le.squint = 0.3f; // raised
            re.y = EYE_Y + 1; re.height = 14; re.squint = 0.6f; // squinted
            m.curve = 0.1f; m.width = 14;
            // mouth slightly to the side
            m.x = MOUTH_X + 5;
            break;
            
        case Expression::STARRY:
            le.isStar = true; le.width = 20; le.height = 20;
            re.isStar = true; re.width = 20; re.height = 20;
            m.curve = 1.0f; m.openness = 0.3f; m.width = 30;
            break;
            
        case Expression::PLAYFUL:
            le.openness = 0.7f; le.squint = 0.3f;
            re.openness = 0.0f; // wink
            m.curve = 0.7f; m.width = 22; m.hasTongue = true;
            break;
            
        case Expression::CHILL:
            le.openness = 0.5f; le.squint = 0.2f; le.height = 12;
            re.openness = 0.5f; re.squint = 0.2f; re.height = 12;
            m.curve = 0.3f; m.width = 16;
            break;
            
        case Expression::PARTY:
            le.isStar = true; le.width = 22; le.height = 22;
            re.isStar = true; re.width = 22; re.height = 22;
            m.curve = 1.0f; m.openness = 0.8f; m.width = 28; m.hasTongue = true;
            ex.showExclaim = true;
            break;
            
        default:
            break;
    }
}

void FaceRenderer::update() {
    unsigned long now = millis();
    
    // Update expression transition
    if (_transProgress < 1.0f) {
        float elapsed = (float)(now - _transStartTime);
        _transProgress = Easing::clamp(elapsed / _transDuration, 0.0f, 1.0f);
        _interpolateParams(Easing::easeInOutCubic(_transProgress));
        if (_transProgress >= 1.0f) {
            _currentExpr = _targetExpr;
        }
    }
    
    // Update subsystems
    _updateBlink();
    _updateIdleLook();
    _updateIdleTimeout();
    
    if (_extras.showZzz) {
        _updateZzz();
    }
    
    if (_leftEye.isSpiral || _rightEye.isSpiral) {
        _spiralAngle += 0.15f;
        if (_spiralAngle > 2 * M_PI) _spiralAngle -= 2 * M_PI;
    }
    
    // Handle touch reaction timeout
    if (_touchReacting && (now - _touchReactTime > 2000)) {
        _touchReacting = false;
        if (_currentExpr == Expression::LOVE || _currentExpr == Expression::BLUSHING ||
            _currentExpr == Expression::SURPRISED) {
            setExpression(Expression::HAPPY, 500);
            // Will naturally return to neutral via idle
        }
    }
}

void FaceRenderer::_interpolateParams(float t) {
    auto lerpF = [](float a, float b, float t) { return a + (b - a) * t; };
    
    // Interpolate left eye
    _leftEye.x = lerpF(_leftEye.x, _targetLeftEye.x, t);
    _leftEye.y = lerpF(_leftEye.y, _targetLeftEye.y, t);
    _leftEye.width = lerpF(_leftEye.width, _targetLeftEye.width, t);
    _leftEye.height = lerpF(_leftEye.height, _targetLeftEye.height, t);
    _leftEye.pupilSize = lerpF(_leftEye.pupilSize, _targetLeftEye.pupilSize, t);
    _leftEye.openness = lerpF(_leftEye.openness, _targetLeftEye.openness, t);
    _leftEye.squint = lerpF(_leftEye.squint, _targetLeftEye.squint, t);
    
    // Interpolate right eye
    _rightEye.x = lerpF(_rightEye.x, _targetRightEye.x, t);
    _rightEye.y = lerpF(_rightEye.y, _targetRightEye.y, t);
    _rightEye.width = lerpF(_rightEye.width, _targetRightEye.width, t);
    _rightEye.height = lerpF(_rightEye.height, _targetRightEye.height, t);
    _rightEye.pupilSize = lerpF(_rightEye.pupilSize, _targetRightEye.pupilSize, t);
    _rightEye.openness = lerpF(_rightEye.openness, _targetRightEye.openness, t);
    _rightEye.squint = lerpF(_rightEye.squint, _targetRightEye.squint, t);
    
    // Snap special flags at midpoint
    if (t > 0.5f) {
        _leftEye.isHeart = _targetLeftEye.isHeart;
        _leftEye.isStar = _targetLeftEye.isStar;
        _leftEye.isSpiral = _targetLeftEye.isSpiral;
        _leftEye.isCool = _targetLeftEye.isCool;
        _rightEye.isHeart = _targetRightEye.isHeart;
        _rightEye.isStar = _targetRightEye.isStar;
        _rightEye.isSpiral = _targetRightEye.isSpiral;
        _rightEye.isCool = _targetRightEye.isCool;
        _mouth.isWavy = _targetMouth.isWavy;
        _mouth.isZigzag = _targetMouth.isZigzag;
        _mouth.hasTongue = _targetMouth.hasTongue;
        _extras = _targetExtras;
    }
    
    // Interpolate mouth
    _mouth.x = lerpF(_mouth.x, _targetMouth.x, t);
    _mouth.y = lerpF(_mouth.y, _targetMouth.y, t);
    _mouth.width = lerpF(_mouth.width, _targetMouth.width, t);
    _mouth.curve = lerpF(_mouth.curve, _targetMouth.curve, t);
    _mouth.openness = lerpF(_mouth.openness, _targetMouth.openness, t);
}

void FaceRenderer::_updateBlink() {
    unsigned long now = millis();
    
    // Don't blink if sleeping or eyes already closed by expression
    if (_currentExpr == Expression::SLEEPING) return;
    
    if (_blinkProgress >= 1.0f && !_blinkClosing) {
        // Check if it's time to blink
        if (now >= _nextBlinkTime) {
            _blinkClosing = true;
            _blinkStartTime = now;
            _doDoubleBlink = (random(100) < 15); // 15% chance of double blink
            _doubleBlinkCount = 0;
        }
    }
    
    if (_blinkClosing) {
        float elapsed = (float)(now - _blinkStartTime);
        if (elapsed < BLINK_CLOSE_MS) {
            _blinkProgress = 1.0f - (elapsed / BLINK_CLOSE_MS);
        } else if (elapsed < BLINK_CLOSE_MS + BLINK_OPEN_MS) {
            float openElapsed = elapsed - BLINK_CLOSE_MS;
            _blinkProgress = openElapsed / BLINK_OPEN_MS;
        } else {
            _blinkProgress = 1.0f;
            _blinkClosing = false;
            _doubleBlinkCount++;
            
            if (_doDoubleBlink && _doubleBlinkCount < 2) {
                // Trigger second blink immediately
                _blinkClosing = true;
                _blinkStartTime = now + 50;
            } else {
                _nextBlinkTime = now + random(BLINK_MIN_INTERVAL_MS, BLINK_MAX_INTERVAL_MS);
            }
        }
    }
}

void FaceRenderer::_updateIdleLook() {
    unsigned long now = millis();
    
    // Don't look around if sleeping or in special expression
    if (_currentExpr == Expression::SLEEPING || _currentExpr == Expression::DIZZY) return;
    
    if (now >= _nextLookTime) {
        _targetLookX = (float)random(-100, 101) / 100.0f * 0.5f;
        _targetLookY = (float)random(-50, 51) / 100.0f * 0.3f;
        _lookTransStart = now;
        _nextLookTime = now + random(2000, 5000);
    }
    
    // Smooth transition to target look
    float lookElapsed = (float)(now - _lookTransStart) / 500.0f; // 500ms transition
    lookElapsed = Easing::clamp(lookElapsed, 0.0f, 1.0f);
    float eased = Easing::easeInOutSine(lookElapsed);
    _lookX = Easing::lerp(_lookX, _targetLookX, eased * 0.1f);
    _lookY = Easing::lerp(_lookY, _targetLookY, eased * 0.1f);
}

void FaceRenderer::_updateIdleTimeout() {
    unsigned long now = millis();
    unsigned long idle = now - _lastActivityTime;
    
    if (idle > SLEEP_TIMEOUT_MS && !_isIdleSleeping) {
        _isIdleSleeping = true;
        _isIdleSleepy = true;
        setExpression(Expression::SLEEPING, 1000);
    } else if (idle > SLEEPY_TIMEOUT_MS && !_isIdleSleepy) {
        _isIdleSleepy = true;
        setExpression(Expression::SLEEPY, 500);
    }
}

void FaceRenderer::_updateZzz() {
    _zzzPhase += 0.05f;
    if (_zzzPhase > 2 * M_PI) _zzzPhase -= 2 * M_PI;
}

void FaceRenderer::resetIdleTimer() {
    _lastActivityTime = millis();
    
    if (_isIdleSleeping || _isIdleSleepy) {
        _isIdleSleeping = false;
        _isIdleSleepy = false;
        setExpression(Expression::SURPRISED, 200);
        // Will naturally transition back
    }
}

void FaceRenderer::onTouch() {
    resetIdleTimer();
    addHappiness(5);
    _touchCount++;
    unsigned long now = millis();
    
    if (now - _lastTouchTime < 3000) {
        // Rapid touching -> love!
        if (_touchCount >= 3) {
            setExpression(Expression::LOVE, 200);
            _touchReacting = true;
            _touchReactTime = now;
            _touchCount = 0;
        }
    } else {
        _touchCount = 1;
    }
    _lastTouchTime = now;
    
    if (!_touchReacting) {
        // Quick happy reaction
        setExpression(Expression::HAPPY, 150);
        _touchReacting = true;
        _touchReactTime = now;
    }
}

void FaceRenderer::onLongTouch() {
    resetIdleTimer();
    addHappiness(15);
    setExpression(Expression::BLUSHING, 300);
    _touchReacting = true;
    _touchReactTime = millis();
}

void FaceRenderer::onWifiConnect() {
    setExpression(Expression::EXCITED, 200);
    _touchReacting = true;
    _touchReactTime = millis();
}

void FaceRenderer::onWifiFail() {
    setExpression(Expression::CONFUSED, 300);
    _touchReacting = true;
    _touchReactTime = millis();
}

void FaceRenderer::onGameOver() {
    setExpression(Expression::DIZZY, 200);
    _touchReacting = true;
    _touchReactTime = millis();
}

void FaceRenderer::onGoodWeather() {
    setExpression(Expression::COOL, 300);
    _touchReacting = true;
    _touchReactTime = millis();
}

// ======================== RENDERING ========================

void FaceRenderer::render() {
    if (!_dm) return;
    Adafruit_SSD1306& d = _dm->display();
    
    // Apply blink to eye openness (multiplicative)
    EyeParams leftDraw = _leftEye;
    EyeParams rightDraw = _rightEye;
    
    if (!leftDraw.isHeart && !leftDraw.isStar && !leftDraw.isSpiral) {
        leftDraw.openness *= _blinkProgress;
    }
    // Don't blink the winking eye
    if (_currentExpr != Expression::WINK && _currentExpr != Expression::PLAYFUL) {
        if (!rightDraw.isHeart && !rightDraw.isStar && !rightDraw.isSpiral) {
            rightDraw.openness *= _blinkProgress;
        }
    }
    
    // Apply idle look direction to pupils
    leftDraw.pupilX += _lookX;
    leftDraw.pupilY += _lookY;
    rightDraw.pupilX += _lookX;
    rightDraw.pupilY += _lookY;
    
    // Draw angry eyebrows
    if (_currentExpr == Expression::ANGRY) {
        // Left eyebrow: angled down toward center
        d.drawLine(leftDraw.x - 12, leftDraw.y - 14, leftDraw.x + 12, leftDraw.y - 8, SSD1306_WHITE);
        d.drawLine(leftDraw.x - 12, leftDraw.y - 13, leftDraw.x + 12, leftDraw.y - 7, SSD1306_WHITE);
        // Right eyebrow: mirrored
        d.drawLine(rightDraw.x - 12, rightDraw.y - 8, rightDraw.x + 12, rightDraw.y - 14, SSD1306_WHITE);
        d.drawLine(rightDraw.x - 12, rightDraw.y - 7, rightDraw.x + 12, rightDraw.y - 13, SSD1306_WHITE);
    }
    
    // Skeptical eyebrow
    if (_currentExpr == Expression::SKEPTICAL) {
        // Left eyebrow raised
        d.drawLine(leftDraw.x - 10, leftDraw.y - 14, leftDraw.x + 10, leftDraw.y - 12, SSD1306_WHITE);
        d.drawLine(leftDraw.x - 10, leftDraw.y - 13, leftDraw.x + 10, leftDraw.y - 11, SSD1306_WHITE);
    }
    
    // Draw eyes
    _drawEye(leftDraw);
    _drawEye(rightDraw);
    
    // Draw mouth
    _drawMouth(_mouth);
    
    // Draw extras
    _drawExtras(_extras);
    
    // Floating Expression Toast Banner (when cycled via one-tap!)
    if (millis() - _toastStartTime < 1200) {
        int tw = 80;
        int tx = (SCREEN_W - tw) / 2;
        d.fillRect(tx, 1, tw, 11, SSD1306_BLACK);
        d.drawRoundRect(tx, 1, tw, 11, 3, SSD1306_WHITE);
        _dm->drawCenteredText(getExpressionName(_targetExpr), 3, 1);
    }
}

void FaceRenderer::_drawEye(const EyeParams& eye) {
    Adafruit_SSD1306& d = _dm->display();
    
    if (eye.isHeart) {
        _drawHeartEye(eye.x, eye.y, eye.width * 0.5f);
        return;
    }
    if (eye.isStar) {
        _drawStarEye(eye.x, eye.y, eye.width * 0.5f);
        return;
    }
    if (eye.isSpiral) {
        _drawSpiralEye(eye.x, eye.y, eye.width * 0.5f);
        return;
    }
    if (eye.isCool) {
        _drawCoolEye(eye);
        return;
    }
    
    // Apply breathing subtle pulse
    float breathMod = sinf(_breathPhase) * 0.5f;
    float effectiveHeight = (eye.height + breathMod) * eye.openness;
    
    if (effectiveHeight < 1.5f) {
        // Eye is closed — draw a line
        d.drawLine(
            (int)(eye.x - eye.width / 2), (int)eye.y,
            (int)(eye.x + eye.width / 2), (int)eye.y,
            SSD1306_WHITE
        );
        // Thicken
        d.drawLine(
            (int)(eye.x - eye.width / 2), (int)(eye.y + 1),
            (int)(eye.x + eye.width / 2), (int)(eye.y + 1),
            SSD1306_WHITE
        );
        return;
    }
    
    // Apply squint: reduce top part of eye
    float topReduction = eye.squint * effectiveHeight * 0.4f;
    
    // Draw eye outline as filled ellipse
    float hw = eye.width / 2.0f;
    float hh = effectiveHeight / 2.0f;
    
    int drawX = (int)(eye.x - hw);
    int drawY = (int)(eye.y - hh + topReduction);
    int drawW = (int)eye.width;
    int drawH = (int)(effectiveHeight - topReduction);
    
    // Draw pupil (black circle inside white eye)
    float px = eye.x + eye.pupilX * (hw - eye.pupilSize - 2);
    float py = eye.y + eye.pupilY * (hh - eye.pupilSize - 2);
    
    if (_eyeStyle == EyeStyle::VECTOR_SQUARE) {
        // Vector-style rounded box eyes
        d.fillRoundRect(drawX, drawY, drawW, drawH, 4, SSD1306_WHITE);
        d.fillRoundRect((int)px - (int)eye.pupilSize, (int)py - (int)eye.pupilSize, (int)eye.pupilSize * 2, (int)eye.pupilSize * 2, 2, SSD1306_BLACK);
        // Primary 2x2 Catchlight specular glint
        d.fillRect((int)(px - 2), (int)(py - 2), 2, 2, SSD1306_WHITE);
        // Secondary 1x1 Catchlight glint
        d.drawPixel((int)(px + 1), (int)(py + 1), SSD1306_WHITE);
    }
    else if (_eyeStyle == EyeStyle::NEON_OUTLINE) {
        // Neon outline hollow eyes
        d.drawRoundRect(drawX, drawY, drawW, drawH, 3, SSD1306_WHITE);
        d.drawRoundRect(drawX + 1, drawY + 1, drawW - 2, drawH - 2, 2, SSD1306_WHITE);
        d.fillCircle((int)px, (int)py, (int)eye.pupilSize, SSD1306_WHITE);
    }
    else if (_eyeStyle == EyeStyle::CYBER_DOT) {
        // Cyber matrix grid eyes
        for (int rx = drawX; rx < drawX + drawW; rx += 3) {
            for (int ry = drawY; ry < drawY + drawH; ry += 3) {
                d.drawPixel(rx, ry, SSD1306_WHITE);
            }
        }
    }
    else {
        // CLASSIC_OVAL default
        int r = (int)(eye.roundness * min(hw, hh) * 0.5f);
        d.fillRoundRect(drawX, drawY, drawW, drawH, r > 0 ? r : 3, SSD1306_WHITE);
        d.fillCircle((int)px, (int)py, (int)eye.pupilSize, SSD1306_BLACK);
        // Primary 2x2 Catchlight specular glint
        d.fillRect((int)(px - 2), (int)(py - 2), 2, 2, SSD1306_WHITE);
        // Secondary 1x1 Catchlight glint
        d.drawPixel((int)(px + 1), (int)(py + 1), SSD1306_WHITE);
    }
}

void FaceRenderer::_drawHeartEye(float cx, float cy, float size) {
    Adafruit_SSD1306& d = _dm->display();
    int s = (int)size;
    
    // Draw heart using two circles and a triangle
    int r = s / 2;
    d.fillCircle((int)cx - r/2, (int)cy - r/3, r, SSD1306_WHITE);
    d.fillCircle((int)cx + r/2, (int)cy - r/3, r, SSD1306_WHITE);
    d.fillTriangle(
        (int)cx - s, (int)cy,
        (int)cx + s, (int)cy,
        (int)cx, (int)(cy + s),
        SSD1306_WHITE
    );
}

void FaceRenderer::_drawStarEye(float cx, float cy, float size) {
    Adafruit_SSD1306& d = _dm->display();
    
    // Draw 4-pointed star
    int s = (int)size;
    // Vertical spike
    d.fillTriangle((int)cx, (int)(cy - s), (int)(cx - 3), (int)cy, (int)(cx + 3), (int)cy, SSD1306_WHITE);
    d.fillTriangle((int)cx, (int)(cy + s), (int)(cx - 3), (int)cy, (int)(cx + 3), (int)cy, SSD1306_WHITE);
    // Horizontal spike
    d.fillTriangle((int)(cx - s), (int)cy, (int)cx, (int)(cy - 3), (int)cx, (int)(cy + 3), SSD1306_WHITE);
    d.fillTriangle((int)(cx + s), (int)cy, (int)cx, (int)(cy - 3), (int)cx, (int)(cy + 3), SSD1306_WHITE);
    
    // Center circle
    d.fillCircle((int)cx, (int)cy, 3, SSD1306_WHITE);
}

void FaceRenderer::_drawSpiralEye(float cx, float cy, float size) {
    Adafruit_SSD1306& d = _dm->display();
    
    // Draw spiral using points
    for (float a = 0; a < 4 * M_PI; a += 0.15f) {
        float r = (a / (4 * M_PI)) * size;
        float angle = a + _spiralAngle;
        int px = (int)(cx + r * cosf(angle));
        int py = (int)(cy + r * sinf(angle));
        d.drawPixel(px, py, SSD1306_WHITE);
        // Thicken spiral
        d.drawPixel(px + 1, py, SSD1306_WHITE);
        d.drawPixel(px, py + 1, SSD1306_WHITE);
    }
}

void FaceRenderer::_drawCoolEye(const EyeParams& eye) {
    Adafruit_SSD1306& d = _dm->display();
    
    // Sunglasses: filled rectangle with rounded corners
    int hw = (int)(eye.width / 2);
    int hh = (int)(eye.height / 2);
    d.fillRoundRect(
        (int)(eye.x - hw), (int)(eye.y - hh),
        (int)eye.width, (int)eye.height,
        3, SSD1306_WHITE
    );
    
    // Inner dark area (lens)
    d.fillRoundRect(
        (int)(eye.x - hw + 2), (int)(eye.y - hh + 2),
        (int)(eye.width - 4), (int)(eye.height - 4),
        2, SSD1306_BLACK
    );
    
    // Glare line
    d.drawLine(
        (int)(eye.x - hw + 3), (int)(eye.y - hh + 3),
        (int)(eye.x - hw + 7), (int)(eye.y - hh + 3),
        SSD1306_WHITE
    );
    
    // Bridge between sunglasses (drawn once for left eye)
    if (eye.x < 64) {
        d.drawLine((int)(eye.x + hw), (int)eye.y, (int)(eye.x + hw + 8), (int)eye.y, SSD1306_WHITE);
        d.drawLine((int)(eye.x + hw), (int)(eye.y - 1), (int)(eye.x + hw + 8), (int)(eye.y - 1), SSD1306_WHITE);
    }
}

void FaceRenderer::_drawMouth(const MouthParams& mouth) {
    Adafruit_SSD1306& d = _dm->display();
    int mx = (int)mouth.x;
    int my = (int)mouth.y;
    int hw = (int)(mouth.width / 2);
    
    if (mouth.isWavy) {
        // Wavy confused mouth
        for (int i = -hw; i <= hw; i++) {
            float wave = sinf(i * 0.5f) * 3;
            d.drawPixel(mx + i, my + (int)wave, SSD1306_WHITE);
            d.drawPixel(mx + i, my + (int)wave + 1, SSD1306_WHITE);
        }
        return;
    }
    
    if (mouth.isZigzag) {
        // Angry zigzag mouth
        int segW = hw / 3;
        for (int i = 0; i < 6; i++) {
            int x1 = mx - hw + i * segW;
            int x2 = mx - hw + (i + 1) * segW;
            int y1 = my + ((i % 2 == 0) ? -2 : 2);
            int y2 = my + ((i % 2 == 0) ? 2 : -2);
            d.drawLine(x1, y1, x2, y2, SSD1306_WHITE);
            d.drawLine(x1, y1 + 1, x2, y2 + 1, SSD1306_WHITE);
        }
        return;
    }
    
    if (mouth.openness > 0.1f) {
        // Open mouth (ellipse)
        int mouthH = (int)(mouth.openness * 10);
        if (mouth.curve > 0.3f) {
            // Smiling open mouth: D shape
            d.fillRoundRect(mx - hw, my - mouthH/2, hw * 2, mouthH, 4, SSD1306_WHITE);
            // Black top half for D-smile
            d.fillRect(mx - hw, my - mouthH/2 - 1, hw * 2, mouthH/2, SSD1306_BLACK);
            // White line on top
            d.drawLine(mx - hw, my, mx + hw, my, SSD1306_WHITE);
        } else {
            // Circular open mouth (surprise "O")
            d.drawRoundRect(mx - hw/2, my - mouthH/2, hw, mouthH, 3, SSD1306_WHITE);
            d.drawRoundRect(mx - hw/2 + 1, my - mouthH/2 + 1, hw - 2, mouthH - 2, 2, SSD1306_WHITE);
        }
    } else {
        // Closed mouth — curved line
        if (abs(mouth.curve) < 0.05f) {
            // Flat line
            d.drawLine(mx - hw, my, mx + hw, my, SSD1306_WHITE);
            d.drawLine(mx - hw, my + 1, mx + hw, my + 1, SSD1306_WHITE);
        } else {
            // Curved line (bezier approximation with arc)
            int curveH = (int)(mouth.curve * 8);
            for (int i = -hw; i <= hw; i++) {
                float t = (float)i / hw;
                float yOff = curveH * (1.0f - t * t); // parabolic curve
                d.drawPixel(mx + i, my + (int)yOff, SSD1306_WHITE);
                d.drawPixel(mx + i, my + (int)yOff + 1, SSD1306_WHITE);
            }
        }
    }
    
    // Tongue
    if (mouth.hasTongue) {
        d.fillRoundRect(mx - 3, my + 2, 8, 8, 3, SSD1306_WHITE);
        d.drawLine(mx + 1, my + 4, mx + 1, my + 9, SSD1306_BLACK); // tongue line
    }
}

void FaceRenderer::_drawExtras(const FaceExtras& extras) {
    if (extras.showBlush) {
        _drawBlush(LEFT_EYE_X - 2, EYE_Y + 12, extras.blushIntensity);
        _drawBlush(RIGHT_EYE_X + 2, EYE_Y + 12, extras.blushIntensity);
    }
    if (extras.showTear) {
        _drawTear(RIGHT_EYE_X + 10, EYE_Y + 5);
    }
    if (extras.showZzz) {
        _drawZzz(RIGHT_EYE_X + 16, EYE_Y - 15);
    }
    if (extras.showExclaim) {
        Adafruit_SSD1306& d = _dm->display();
        // Exclamation mark above head
        d.fillRect(62, 2, 4, 8, SSD1306_WHITE);
        d.fillRect(62, 12, 4, 3, SSD1306_WHITE);
    }
}

void FaceRenderer::_drawBlush(float x, float y, float intensity) {
    Adafruit_SSD1306& d = _dm->display();
    int count = (int)(intensity * 5);
    // Hash marks (diagonal lines)
    for (int i = 0; i < count; i++) {
        int ox = i * 3 - (count * 3) / 2;
        d.drawLine((int)x + ox, (int)y, (int)x + ox + 2, (int)y + 2, SSD1306_WHITE);
    }
}

void FaceRenderer::_drawTear(float x, float y) {
    Adafruit_SSD1306& d = _dm->display();
    // Teardrop shape
    float phase = (float)(millis() % 2000) / 2000.0f;
    float dropY = y + phase * 15;
    
    d.fillCircle((int)x, (int)dropY + 2, 2, SSD1306_WHITE);
    d.fillTriangle(
        (int)x, (int)dropY - 2,
        (int)x - 2, (int)dropY + 2,
        (int)x + 2, (int)dropY + 2,
        SSD1306_WHITE
    );
}

void FaceRenderer::_drawZzz(float x, float y) {
    Adafruit_SSD1306& d = _dm->display();
    
    // Three floating Z's at different heights
    float baseY = y + sinf(_zzzPhase) * 3;
    
    d.setTextSize(1);
    d.setCursor((int)x, (int)(baseY));
    d.print("z");
    d.setCursor((int)(x + 6), (int)(baseY - 6));
    d.setTextSize(1);
    d.print("Z");
    d.setCursor((int)(x + 12), (int)(baseY - 14));
    d.setTextSize(1);
    d.print("Z");
}
