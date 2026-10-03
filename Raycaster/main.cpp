#include <SDL.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include "Collision.h"
#include "Vector2.h"
#include "Raycaster.h"

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

    float facingAngle = 0.0f;
    float mouseSensitivity = 0.003f;

    bool mouseCaptured = false;

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
            else if (event.type == SDL_MOUSEBUTTONDOWN &&
                event.button.button == SDL_BUTTON_LEFT) {
                if (!mouseCaptured) {
                    if (SDL_SetRelativeMouseMode(SDL_TRUE) == 0) {
                        mouseCaptured = true;
                    }
                    else {
                        std::cerr << "Mouse capture failed: "
                            << SDL_GetError() << '\n';
                    }
                }
            }
            else if (event.type == SDL_KEYDOWN &&
                event.key.keysym.sym == SDLK_ESCAPE) {
                SDL_SetRelativeMouseMode(SDL_FALSE);
                mouseCaptured = false;
            }
            else if (event.type == SDL_WINDOWEVENT &&
                event.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                SDL_SetRelativeMouseMode(SDL_FALSE);
                mouseCaptured = false;
            }
            else if (event.type == SDL_MOUSEMOTION && mouseCaptured) {
                facingAngle += event.motion.xrel * mouseSensitivity;

                const float TWO_PI = 6.283185307f;
                facingAngle = std::fmod(facingAngle, TWO_PI);
            }
        }
        Vector2 facingDirection{
            std::cos(facingAngle),
            std::sin(facingAngle)
        };

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
        Vector2 rightDirection{
            -facingDirection.y,
            facingDirection.x
        };

        float forwardInput = 0.0f;
        float strafeInput = 0.0f;

        if (up)    forwardInput += 1.0f;
        if (down)  forwardInput -= 1.0f;
        if (right) strafeInput += 1.0f;
        if (left)  strafeInput -= 1.0f;

        Vector2 inputDir{
            facingDirection.x * forwardInput + rightDirection.x * strafeInput,
            facingDirection.y * forwardInput + rightDirection.y * strafeInput
        };

        // Normalize to keep diagonal speed identical to orthogonal
        Vector2 moveDirection = inputDir.normalized();

        float movementX = moveDirection.x * currentSpeed * deltaTime;
        float movementY = moveDirection.y * currentSpeed * deltaTime;

        moveWithCollision(player, map, movementX, movementY, TILE_SIZE);

        // Draw the ceiling.
        SDL_SetRenderDrawColor(renderer, 35, 40, 50, 255);
        SDL_RenderClear(renderer);

        // Draw the floor across the bottom half.
        SDL_Rect floorArea{
            0,
            WINDOW_HEIGHT / 2,
            WINDOW_WIDTH,
            WINDOW_HEIGHT - WINDOW_HEIGHT / 2
        };

        SDL_SetRenderDrawColor(renderer, 65, 60, 55, 255);
        SDL_RenderFillRect(renderer, &floorArea);

        // Camera position and field of view.
        Vector2 rayOrigin{
            player.x + player.w / 2.0f,
            player.y + player.h / 2.0f
        };

        const float fieldOfView = 1.04719755f;
        float halfViewWidth = std::tan(fieldOfView / 2.0f);

        Vector2 cameraRight{
            -facingDirection.y,
            facingDirection.x
        };

        // Controls the projection scale in screen pixels.
        float projectionDistance = (WINDOW_WIDTH / 2.0f) / halfViewWidth;
        float wallHeight = static_cast<float>(TILE_SIZE);

        // Cast one ray per screen column.
        for (int screenX = 0; screenX < WINDOW_WIDTH; ++screenX) {
            float viewPosition =
                2.0f * (screenX + 0.5f) / WINDOW_WIDTH - 1.0f;

            Vector2 rayDir{
                facingDirection.x + cameraRight.x * viewPosition * halfViewWidth,
                facingDirection.y + cameraRight.y * viewPosition * halfViewWidth
            };

            rayDir = rayDir.normalized();

            RayHit wallHit = castRay(map, rayOrigin, rayDir, TILE_SIZE);

            if (!wallHit.hit) {
                continue;
            }

            // Convert distance along the ray into depth straight ahead.
            float alignment =
                rayDir.x * facingDirection.x +
                rayDir.y * facingDirection.y;

            float depth = wallHit.distance * alignment;

            if (depth < 0.01f) {
                depth = 0.01f;
            }

            // Project the wall's world height onto the screen.
            float sliceHeight = wallHeight * projectionDistance / depth;

            float drawTop = WINDOW_HEIGHT / 2.0f - sliceHeight / 2.0f;
            float drawBottom = WINDOW_HEIGHT / 2.0f + sliceHeight / 2.0f;

            if (drawTop < 0.0f) {
                drawTop = 0.0f;
            }

            if (drawBottom > WINDOW_HEIGHT - 1.0f) {
                drawBottom = WINDOW_HEIGHT - 1.0f;
            }

            // Simple distance shading.
            float brightness = 1.0f / (1.0f + depth / 500.0f);
            Uint8 shade = static_cast<Uint8>(220.0f * brightness);

            SDL_SetRenderDrawColor(renderer, shade, shade, shade, 255);

            SDL_RenderDrawLine(
                renderer,
                screenX,
                static_cast<int>(drawTop),
                screenX,
                static_cast<int>(drawBottom)
            );
        }

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
