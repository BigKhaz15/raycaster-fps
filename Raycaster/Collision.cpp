#include "Collision.h"
#include <algorithm>
#include <cmath>

bool isWallAt(
    const std::vector<std::string>& map,
    float pixelX,
    float pixelY,
    int tileSize
) {
    int column = static_cast<int>(std::floor(pixelX / tileSize));
    int row = static_cast<int>(std::floor(pixelY / tileSize));

    if (row < 0 || row >= static_cast<int>(map.size())) {
        return true;
    }

    if (column < 0 || column >= static_cast<int>(map[row].size())) {
        return true;
    }

    return map[row][column] == '1';
}

bool canOccupy(
    const std::vector<std::string>& map,
    const SDL_FRect& rectangle,
    int tileSize
) {
    int leftColumn = static_cast<int>(
        std::floor(rectangle.x / tileSize));

    int rightColumn = static_cast<int>(
        std::ceil((rectangle.x + rectangle.w) / tileSize)) - 1;

    int topRow = static_cast<int>(
        std::floor(rectangle.y / tileSize));

    int bottomRow = static_cast<int>(
        std::ceil((rectangle.y + rectangle.h) / tileSize)) - 1;

    for (int row = topRow; row <= bottomRow; ++row) {
        for (int column = leftColumn; column <= rightColumn; ++column) {
            if (isWallAt(map, column * tileSize, row * tileSize, tileSize)) {
                return false;
            }
        }
    }

    return true;
}

void moveWithCollision(
    SDL_FRect& player,
    const std::vector<std::string>& map,
    float movementX,
    float movementY,
    int tileSize
) {
    float maxStep = tileSize / 4.0f;

    int steps = std::max(1, static_cast<int>(std::ceil(
        std::max(std::abs(movementX), std::abs(movementY)) / maxStep
    )));

    float stepX = movementX / steps;
    float stepY = movementY / steps;

    for (int i = 0; i < steps; ++i) {
        // Horizontal movement
        SDL_FRect nextPosition = player;
        nextPosition.x += stepX;

        if (canOccupy(map, nextPosition, tileSize)) {
            player.x = nextPosition.x;
        }
        else if (stepX > 0.0f) {
            float wallEdge = std::ceil(
                (player.x + player.w) / tileSize) * tileSize;

            player.x = wallEdge - player.w;
        }
        else if (stepX < 0.0f) {
            float wallEdge = std::floor(player.x / tileSize) * tileSize;

            player.x = wallEdge;
        }

        // Vertical movement
        nextPosition = player;
        nextPosition.y += stepY;

        if (canOccupy(map, nextPosition, tileSize)) {
            player.y = nextPosition.y;
        }
        else if (stepY > 0.0f) {
            float wallEdge = std::ceil(
                (player.y + player.h) / tileSize) * tileSize;

            player.y = wallEdge - player.h;
        }
        else if (stepY < 0.0f) {
            float wallEdge = std::floor(player.y / tileSize) * tileSize;

            player.y = wallEdge;
        }
    }
}