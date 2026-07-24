#pragma once
// ============================================================================
// DeskBuddy — Face Renderer (Procedural Expressive Face Engine)
// ============================================================================
#include <Arduino.h>
#include "display_manager.h"

enum class EyeStyle {
    CLASSIC_OVAL,
    VECTOR_SQUARE,
    NEON_OUTLINE,
    CYBER_DOT,
    COUNT
};

// Expression types
enum class Expression {
    NEUTRAL,
    HAPPY,
    LOVE,
    SLEEPY,
    SLEEPING,
    SURPRISED,
    ANGRY,
    SAD,
    CONFUSED,
    EXCITED,
    WINK,
    DIZZY,
    COOL,
    BLUSHING,
    SKEPTICAL,
    STARRY,
    PLAYFUL,
    CHILL,
    PARTY,
    COUNT  // total number of expressions
};

// Eye parameters (fully parametric)
struct EyeParams {
    float x, y;           // center position
    float width, height;  // ellipse dimensions
    float pupilX, pupilY; // pupil offset from center (-1 to 1)
    float pupilSize;      // pupil radius
    float openness;       // 0.0 = closed, 1.0 = fully open
    float squint;         // 0.0 = none, 1.0 = full squint
    float roundness;      // corner radius factor
    bool isHeart;         // draw as heart shape
    bool isStar;          // draw as star shape
    bool isSpiral;        // draw as spiral (dizzy)
    bool isCool;          // draw as rectangle (sunglasses)
};

// Mouth parameters
struct MouthParams {
    float x, y;           // center position
    float width;          // mouth width
    float curve;          // -1.0 = frown, 0.0 = flat, 1.0 = smile
    float openness;       // 0.0 = closed line, 1.0 = fully open
    bool isWavy;          // confused wavy mouth
    bool isZigzag;        // angry zigzag mouth
    bool hasTongue;       // tongue sticking out
};

// Face extras
struct FaceExtras {
    bool showBlush;       // blush marks on cheeks
    bool showTear;        // tear drop
    bool showZzz;         // sleeping Z's
    bool showExclaim;     // exclamation mark
    bool showSweat;       // sweat drop
    float blushIntensity; // 0.0 - 1.0
};

class FaceRenderer {
public:
    void begin(DisplayManager* dm);
    void update();         // call every frame
    void render();         // draw current face state
    
    // Expression & Style control
    void setExpression(Expression expr, uint16_t transitionMs = EXPRESSION_TRANSITION_MS);
    Expression getExpression() const { return _targetExpr; }
    const char* getExpressionName(Expression expr) const;
    
    void setEyeStyle(EyeStyle style) { _eyeStyle = style; }
    EyeStyle getEyeStyle() const { return _eyeStyle; }
    
    // External triggers
    void onTouch();        // react to touch
    void onLongTouch();    // react to sustained touch
    void onWifiConnect();  // excited expression
    void onWifiFail();     // confused expression
    void onGameOver();     // dizzy expression
    void onGoodWeather();  // cool expression
    
    // Idle management
    void resetIdleTimer();
    bool isSleeping() const { return _currentExpr == Expression::SLEEPING; }
    
private:
    DisplayManager* _dm = nullptr;
    
    // Current and target state
    EyeStyle _eyeStyle = EyeStyle::CLASSIC_OVAL;
    Expression _currentExpr = Expression::NEUTRAL;
    Expression _targetExpr = Expression::NEUTRAL;
    
    // Animation & Micro-expression state
    float _breathPhase = 0.0f;
    float _transProgress = 1.0f;       // 0.0 to 1.0
    uint16_t _transDuration = 300;
    unsigned long _transStartTime = 0;
    
    // Eye parameters (interpolated)
    EyeParams _leftEye, _rightEye;
    EyeParams _targetLeftEye, _targetRightEye;
    MouthParams _mouth;
    MouthParams _targetMouth;
    FaceExtras _extras;
    FaceExtras _targetExtras;
    
    // Blink system
    unsigned long _nextBlinkTime = 0;
    float _blinkProgress = 1.0f;       // 1.0 = open, 0.0 = closed
    bool _blinkClosing = false;
    unsigned long _blinkStartTime = 0;
    bool _doDoubleBlink = false;
    int _doubleBlinkCount = 0;
    
    // Idle look-around
    unsigned long _nextLookTime = 0;
    float _lookX = 0, _lookY = 0;
    float _targetLookX = 0, _targetLookY = 0;
    unsigned long _lookTransStart = 0;
    
    // Idle timeout
    unsigned long _lastActivityTime = 0;
    bool _isIdleSleepy = false;
    bool _isIdleSleeping = false;
    
    // ZZZ animation
    float _zzzPhase = 0;
    
    // Dizzy spiral angle
    float _spiralAngle = 0;
    
    // Touch reaction
    unsigned long _touchReactTime = 0;
    bool _touchReacting = false;
    int _touchCount = 0;
    unsigned long _lastTouchTime = 0;
    
    // Internal methods
    void _setExpressionParams(Expression expr, EyeParams& le, EyeParams& re, MouthParams& m, FaceExtras& ex);
    void _interpolateParams(float t);
    void _updateBlink();
    void _updateIdleLook();
    void _updateIdleTimeout();
    void _updateZzz();
    
    // Drawing
    void _drawEye(const EyeParams& eye);
    void _drawHeartEye(float cx, float cy, float size);
    void _drawStarEye(float cx, float cy, float size);
    void _drawSpiralEye(float cx, float cy, float size);
    void _drawCoolEye(const EyeParams& eye);
    void _drawMouth(const MouthParams& mouth);
    void _drawExtras(const FaceExtras& extras);
    void _drawBlush(float x, float y, float intensity);
    void _drawTear(float x, float y);
    void _drawZzz(float x, float y);
};
