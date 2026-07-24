#pragma once
// ============================================================================
// DeskBuddy — Easing Functions for Smooth Animations
// ============================================================================
#include <math.h>

namespace Easing {

// Linear (no easing)
inline float linear(float t) {
    return t;
}

// Quadratic ease-in-out
inline float easeInOutQuad(float t) {
    return t < 0.5f ? 2.0f * t * t : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) / 2.0f;
}

// Cubic ease-in-out
inline float easeInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - powf(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

// Ease-out bounce
inline float easeOutBounce(float t) {
    if (t < 1.0f / 2.75f) {
        return 7.5625f * t * t;
    } else if (t < 2.0f / 2.75f) {
        t -= 1.5f / 2.75f;
        return 7.5625f * t * t + 0.75f;
    } else if (t < 2.5f / 2.75f) {
        t -= 2.25f / 2.75f;
        return 7.5625f * t * t + 0.9375f;
    } else {
        t -= 2.625f / 2.75f;
        return 7.5625f * t * t + 0.984375f;
    }
}

// Ease-out elastic
inline float easeOutElastic(float t) {
    if (t == 0.0f || t == 1.0f) return t;
    return powf(2.0f, -10.0f * t) * sinf((t * 10.0f - 0.75f) * (2.0f * M_PI) / 3.0f) + 1.0f;
}

// Ease-in-out sine (very smooth, natural)
inline float easeInOutSine(float t) {
    return -(cosf(M_PI * t) - 1.0f) / 2.0f;
}

// Ease-out back (slight overshoot)
inline float easeOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * powf(t - 1.0f, 3.0f) + c1 * powf(t - 1.0f, 2.0f);
}

// Interpolate between two values with easing
inline float lerp(float from, float to, float t) {
    return from + (to - from) * t;
}

// Interpolate with easing function
inline float lerpEased(float from, float to, float t, float (*easeFn)(float)) {
    return from + (to - from) * easeFn(t);
}

// Clamp value
inline float clamp(float v, float lo, float hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}

// Map value from one range to another
inline float mapRange(float v, float inLo, float inHi, float outLo, float outHi) {
    return outLo + (v - inLo) * (outHi - outLo) / (inHi - inLo);
}

} // namespace Easing
