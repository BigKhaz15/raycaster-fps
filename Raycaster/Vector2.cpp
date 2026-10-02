#include "Vector2.h"
#include <cmath>

float Vector2::length() const {
    return std::sqrt((x * x) + (y * y));
}

Vector2 Vector2::normalized() const {
    float len = length();

    if (len > 0.0f) {
        return Vector2{ x / len, y / len };
    }

    return Vector2{ 0.0f, 0.0f };
}