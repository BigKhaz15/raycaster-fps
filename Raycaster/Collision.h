#pragma once

#include <SDL.h>
#include <string>
#include <vector>

bool isWallAt(
    const std::vector<std::string>& map,
    float pixelX,
    float pixelY,
    int tileSize
);

bool canOccupy(
    const std::vector<std::string>& map,
    const SDL_FRect& rectangle,
    int tileSize
);

void moveWithCollision(
    SDL_FRect& player,
    const std::vector<std::string>& map,
    float movementX,
    float movementY,
    int tileSize
);