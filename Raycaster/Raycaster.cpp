#include "Raycaster.h"
#include <cmath>
#include <limits>

RayHit castRay(
    const std::vector<std::string>& map,
    Vector2 origin,
    Vector2 rayDir,
    int tileSize
) {
    if (rayDir.x == 0.0f && rayDir.y == 0.0f) {
        return {};
    }

    int column = static_cast<int>(std::floor(origin.x / tileSize));
    int row = static_cast<int>(std::floor(origin.y / tileSize));

    // Which way do our column and row indices change?
    int stepX;

    if (rayDir.x < 0.0f) {
        stepX = -1;
    }
    else {
        stepX = 1;
    }

    int stepY;

    if (rayDir.y < 0.0f) {
        stepY = -1;
    }
    else {
        stepY = 1;
    }

    float infinity = std::numeric_limits<float>::infinity();

    // Distance along the ray between vertical grid crossings.
    float deltaX;

    if (rayDir.x == 0.0f) {
        deltaX = infinity;
    }
    else {
        deltaX = tileSize / std::abs(rayDir.x);
    }

    // Distance along the ray between horizontal grid crossings.
    float deltaY;

    if (rayDir.y == 0.0f) {
        deltaY = infinity;
    }
    else {
        deltaY = tileSize / std::abs(rayDir.y);
    }

    // Pixel x coordinate of the first vertical boundary ahead.
    float boundaryX;

    if (stepX > 0) {
        boundaryX = (column + 1) * tileSize;
    }
    else {
        boundaryX = column * tileSize;
    }

    // Pixel y coordinate of the first horizontal boundary ahead.
    float boundaryY;

    if (stepY > 0) {
        boundaryY = (row + 1) * tileSize;
    }
    else {
        boundaryY = row * tileSize;
    }

    // Distance along the ray to the first vertical boundary.
    float sideX;

    if (rayDir.x == 0.0f) {
        sideX = infinity;
    }
    else {
        sideX = (boundaryX - origin.x) / rayDir.x;
    }

    // Distance along the ray to the first horizontal boundary.
    float sideY;

    if (rayDir.y == 0.0f) {
        sideY = infinity;
    }
    else {
        sideY = (boundaryY - origin.y) / rayDir.y;
    }

    float distance = 0.0f;

    while (true) {
        // Stop if the tracked tile is outside the map.
        if (row < 0 || row >= static_cast<int>(map.size())) {
            return {};
        }

        if (column < 0 ||
            column >= static_cast<int>(map[row].size())) {
            return {};
        }

        // Return the distance at which we entered the first wall.
        if (map[row][column] == '1') {
            return RayHit{ true, distance };
        }

        // Track the next tile along the ray's fixed direction.
        if (sideX < sideY) {
            distance = sideX;
            sideX += deltaX;
            column += stepX;
        }
        else {
            distance = sideY;
            sideY += deltaY;
            row += stepY;
        }
    }
}