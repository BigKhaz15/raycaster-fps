#include <SDL.h>

int main(int argc, char* argv[]) {
    SDL_Init(SDL_INIT_VIDEO);

    // Explicitly naming window dimensions to use for boundary math
    const int WINDOW_WIDTH = 1000;
    const int WINDOW_HEIGHT = 800;

    SDL_Window* window = SDL_CreateWindow("Movable Character", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    SDL_FRect player = { 375.0f, 275.0f, 50.0f, 50.0f }; // X, Y, Width, Height
    float moveSpeed = 300.0f;  // Units/pixels per second

    // Timing variables for Delta Time
    Uint64 lastTime = SDL_GetPerformanceCounter();
    float deltaTime = 0.0f;

    bool up = false, down = false, left = false, right = false;
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
            else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                case SDLK_ESCAPE: running = false; break;
                case SDLK_w:     up = true;     break;
                case SDLK_s:     down = true;   break;
                case SDLK_a:     left = true;   break;
                case SDLK_d:     right = true;  break;
                }
            }
            else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                case SDLK_w:     up = false;    break;
                case SDLK_s:     down = false;  break;
                case SDLK_a:     left = false;  break;
                case SDLK_d:     right = false; break;
                }
            }
        }

        if (up)    player.y -= moveSpeed * deltaTime;
        if (down)  player.y += moveSpeed * deltaTime;
        if (left)  player.x -= moveSpeed * deltaTime;
        if (right) player.x += moveSpeed * deltaTime;

        // --- NEW BOUNDARY CHECKING CODE ---
        if (player.x < 0.0f) {
            player.x = 0.0f;
        }
        if (player.x + player.w > (float)WINDOW_WIDTH) {
            player.x = (float)WINDOW_WIDTH - player.w;
        }
        if (player.y < 0.0f) {
            player.y = 0.0f;
        }
        if (player.y + player.h > (float)WINDOW_HEIGHT) {
            player.y = (float)WINDOW_HEIGHT - player.h;
        }
        // ----------------------------------

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRectF(renderer, &player);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
