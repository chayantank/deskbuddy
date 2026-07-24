// ============================================================================
// DeskBuddy — Touch Input System Implementation
// ============================================================================
#include "touch_input.h"

void TouchInput::begin() {
    pinMode(PIN_TOUCH, INPUT);
    _tapState = TapState::IDLE;
    _touching = false;
    _lastReading = false;
}

unsigned long TouchInput::touchDuration() const {
    if (_touching) {
        return millis() - _touchStart;
    }
    return 0;
}

unsigned long TouchInput::timeSinceLastTouch() const {
    if (_touching) return 0;
    if (_touchEnd == 0) return millis(); // never touched
    return millis() - _touchEnd;
}

TouchEvent TouchInput::_emitEvent(TouchEvent evt) {
    if (_callback && evt != TouchEvent::NONE) {
        _callback(evt);
    }
    return evt;
}

TouchEvent TouchInput::update() {
    bool raw = digitalRead(PIN_TOUCH) == HIGH;
    unsigned long now = millis();
    
    // Debounce
    if (raw != _lastReading) {
        _lastDebounceTime = now;
    }
    _lastReading = raw;
    
    if ((now - _lastDebounceTime) < TOUCH_DEBOUNCE_MS) {
        return TouchEvent::NONE;
    }
    
    bool pressed = raw;
    bool justPressed = pressed && !_touching;
    bool justReleased = !pressed && _touching;
    
    if (justPressed) {
        _touching = true;
        _touchStart = now;
        _longPressFired = false;
        _holdFired = false;
        _deepHoldFired = false;
    }
    
    if (justReleased) {
        _touching = false;
        _touchEnd = now;
    }
    
    // State machine for gesture detection
    switch (_tapState) {
        case TapState::IDLE:
            if (justPressed) {
                _tapState = TapState::FIRST_DOWN;
            }
            break;
            
        case TapState::FIRST_DOWN:
            if (justReleased) {
                unsigned long pressDur = _touchEnd - _touchStart;
                if (pressDur < TOUCH_TAP_MAX_MS) {
                    // Short tap — wait to see if it's a double-tap
                    _firstTapTime = _touchEnd;
                    _tapState = TapState::FIRST_UP_WAITING;
                } else {
                    // Was held too long for a tap, already handled below
                    _tapState = TapState::IDLE;
                }
            } else if (pressed) {
                unsigned long held = now - _touchStart;
                if (held >= TOUCH_DEEP_HOLD_MS && !_deepHoldFired) {
                    _deepHoldFired = true;
                    _tapState = TapState::DEEP_HOLDING;
                    return _emitEvent(TouchEvent::DEEP_HOLD);
                } else if (held >= TOUCH_HOLD_MS && !_holdFired) {
                    _holdFired = true;
                    _tapState = TapState::HOLDING;
                    return _emitEvent(TouchEvent::HOLD);
                } else if (held >= TOUCH_LONG_PRESS_MS && !_longPressFired) {
                    _longPressFired = true;
                    _tapState = TapState::LONG_PRESSING;
                    return _emitEvent(TouchEvent::LONG_PRESS);
                }
            }
            break;
            
        case TapState::FIRST_UP_WAITING:
            if (justPressed) {
                // Second tap started!
                _tapState = TapState::SECOND_DOWN;
            } else if ((now - _firstTapTime) > TOUCH_DOUBLE_GAP_MS) {
                // Timed out waiting for second tap → single tap
                _tapState = TapState::IDLE;
                return _emitEvent(TouchEvent::TAP);
            }
            break;
            
        case TapState::SECOND_DOWN:
            if (justReleased) {
                unsigned long pressDur = _touchEnd - _touchStart;
                if (pressDur < TOUCH_TAP_MAX_MS) {
                    _tapState = TapState::IDLE;
                    return _emitEvent(TouchEvent::DOUBLE_TAP);
                } else {
                    _tapState = TapState::IDLE;
                    return _emitEvent(TouchEvent::TAP); // first tap still counts
                }
            } else if (pressed) {
                unsigned long held = now - _touchStart;
                if (held >= TOUCH_LONG_PRESS_MS) {
                    // Held on second press — treat first as tap, this as long
                    _tapState = TapState::IDLE;
                    return _emitEvent(TouchEvent::TAP);
                }
            }
            break;
            
        case TapState::LONG_PRESSING:
            if (justReleased) {
                _tapState = TapState::IDLE;
            } else if (pressed) {
                unsigned long held = now - _touchStart;
                if (held >= TOUCH_DEEP_HOLD_MS && !_deepHoldFired) {
                    _deepHoldFired = true;
                    _tapState = TapState::DEEP_HOLDING;
                    return _emitEvent(TouchEvent::DEEP_HOLD);
                } else if (held >= TOUCH_HOLD_MS && !_holdFired) {
                    _holdFired = true;
                    _tapState = TapState::HOLDING;
                    return _emitEvent(TouchEvent::HOLD);
                }
            }
            break;
            
        case TapState::HOLDING:
            if (justReleased) {
                _tapState = TapState::IDLE;
            } else if (pressed) {
                unsigned long held = now - _touchStart;
                if (held >= TOUCH_DEEP_HOLD_MS && !_deepHoldFired) {
                    _deepHoldFired = true;
                    _tapState = TapState::DEEP_HOLDING;
                    return _emitEvent(TouchEvent::DEEP_HOLD);
                }
            }
            break;
            
        case TapState::DEEP_HOLDING:
            if (justReleased) {
                _tapState = TapState::IDLE;
            }
            break;
    }
    
    return TouchEvent::NONE;
}
