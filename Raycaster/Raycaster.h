#pragma once

#include <string>
#include <vector>
#include "Vector2.h"

struct RayHit {
    bool hit = false;
    float distance = 0.0f;
};

RayHit castRay(
    const std::vector<std::string>& map,
    Vector2 origin,
    Vector2 direction,
    int tileSize
);