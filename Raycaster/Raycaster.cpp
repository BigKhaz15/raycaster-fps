#include <SDL.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "Collision.h"
#include "Vector2.h"

int main(int argc, char* argv[]) {
    std::ifstream mapFile("map.txt");

    if (!mapFile.is_open()) {
        std::cerr << "Could not open map.txt\n";
        return 1;
    }
    std::vector<std::string> map;
    std::string row;

    while (std::getline(mapFile, row)) {
        map.push_back(row);
    }

    if (map.empty()) {
        std::cerr << "Map is empty\n";
        return 1;
    }

    for (const std::string& mapRow : map) {
        std::cout << mapRow << '\n';
        if (mapRow.size() != map[0].size()) {
            std::cerr << "Error.\n";
            return 1;
        }
    }

    SDL_Init(SDL_INIT_VIDEO);
    // Explicitly naming window dimensions to use for boundary math
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;
    const int TILE_SIZE = 100;

    SDL_Window* window = SDL_CreateWindow("Movable Character", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_FRect player = { 125.0f, 125.0f, 50.0f, 50.0f }; // X, Y, Width, Height
    float runSpeed = 300.0f;  // Units/pixels per second
    float walkSpeed = 120.0f;

    // Timing variables for Delta Time
    Uint64 lastTime = SDL_GetPerformanceCounter();
    float deltaTime = 0.0f;

    bool running = true;
    SDL_Event event;

    while (running) {
        Uint64 currentTime = SDL_GetPerformanceCounter();
        deltaTime = (float)(currentTime - lastTime) / (float)SDL_GetPerformanceFrequency();
        lastTime = currentTime;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if (event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_ESCAPE) {
                running = false;
            }
        }
        const Uint8* keyboard = SDL_GetKeyboardState(nullptr); // gives keyboard state

        bool up = keyboard[SDL_SCANCODE_W]
            || keyboard[SDL_SCANCODE_UP];

        bool down = keyboard[SDL_SCANCODE_S]
            || keyboard[SDL_SCANCODE_DOWN];

        bool left = keyboard[SDL_SCANCODE_A]
            || keyboard[SDL_SCANCODE_LEFT];

        bool right = keyboard[SDL_SCANCODE_D]
            || keyboard[SDL_SCANCODE_RIGHT];

        bool walking = keyboard[SDL_SCANCODE_LSHIFT]
            || keyboard[SDL_SCANCODE_RSHIFT]; // checks if either shift is held

        float currentSpeed = walking ? walkSpeed : runSpeed; // determines if walking or running

        // --- MOVEMENT LOGIC ---
        Vector2 inputDir{ 0.0f, 0.0f };
        if (up)    inputDir.y -= 1.0f;
        if (down)  inputDir.y += 1.0f;
        if (right) inputDir.x += 1.0f;
        if (left)  inputDir.x -= 1.0f;

        // Normalize to keep diagonal speed identical to orthogonal
        Vector2 moveDirection = inputDir.normalized();

        float movementX = moveDirection.x * currentSpeed * deltaTime;
        float movementY = moveDirection.y * currentSpeed * deltaTime;

        moveWithCollision(player, map, movementX, movementY, TILE_SIZE);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);

        for (int y = 0; y < static_cast<int>(map.size()); ++y) {
            for (int x = 0; x < static_cast<int>(map[y].size()); ++x) {
                if (map[y][x] == '1') {
                    SDL_Rect wall = {
                        x * TILE_SIZE,
                        y * TILE_SIZE,
                        TILE_SIZE,
                        TILE_SIZE
                    };

                    // Fill the tile gray
                    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
                    SDL_RenderFillRect(renderer, &wall);

                    // Outline the tile in black
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                    SDL_RenderDrawRect(renderer, &wall);
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRectF(renderer, &player);

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
