
/*
    Compilation:
    g++ -Wall -Wextra main.cpp -o demo `sdl2-config --cflags --libs`
*/
#include <iostream>
#include <SDL2/SDL.h>

#define WIDTH 740
#define HEIGHT 440
#define BLACK 0, 0, 0, 255
#define WHITE 255, 255, 255, 255

// Optional: clamp utility so the rectangle can't leave the window.
static inline int clampi(int v, int lo, int hi) {
    return (v < lo) ? lo : (v > hi ? hi : v);
}

int main(int, char*[]) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return -1;
    }

    SDL_Window* mainWindow = SDL_CreateWindow(
        "MainWindow",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WIDTH, HEIGHT,
        0
    );
    if (!mainWindow) {
        std::cout << "The window wasn't initialized correctly: " << SDL_GetError() << "\n";
        SDL_Quit();
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        mainWindow, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        std::cout << "Renderer creation failed: " << SDL_GetError() << "\n";
        SDL_DestroyWindow(mainWindow);
        SDL_Quit();
        return -1;
    }

    // The draggable rectangle (left paddle style)
    SDL_Rect rectangle_move = {20, 1, 20, 60};

    // A static middle divider so you can see both being drawn
    SDL_Rect middle_rect = {(WIDTH / 2) - 10, 0, 20, HEIGHT};

    bool is_dragging = false;
    SDL_Point click_offset = {0, 0}; // mouse click offset inside the rect

    bool quit = false;
    SDL_Event e;

    // Optional: If you want to keep receiving mouse motion while out of window during drag,
    // you can capture the mouse when drag starts and release it when drag ends.
    // (Uncomment if desired.)
    // auto set_capture = [&](bool enable) {
    //     SDL_CaptureMouse(enable ? SDL_TRUE : SDL_FALSE);
    // };

    while (!quit) {
        // --- Event handling ---
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT:
                    quit = true;
                    break;

                case SDL_KEYDOWN:
                    // Press any key to quit in this simple demo.
                    quit = true;
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        SDL_Point mouse_pos = {e.button.x, e.button.y};
                        if (SDL_PointInRect(&mouse_pos, &rectangle_move)) {
                            is_dragging = true;
                            // remember where inside the rect the user clicked:
                            click_offset.x = e.button.x - rectangle_move.x;
                            click_offset.y = e.button.y - rectangle_move.y;

                            // set_capture(true); // optional
                        }
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    if (e.button.button == SDL_BUTTON_LEFT) {
                        is_dragging = false;
                        // set_capture(false); // optional
                    }
                    break;

                case SDL_MOUSEMOTION:
                    if (is_dragging) {
                        // Update rect position based on current mouse position minus initial offset
                        rectangle_move.x = e.motion.x - click_offset.x;
                        rectangle_move.y = e.motion.y - click_offset.y;

                        // Optional: clamp so the rectangle stays fully on screen
                        rectangle_move.x = clampi(rectangle_move.x, 0, WIDTH  - rectangle_move.w);
                        rectangle_move.y = clampi(rectangle_move.y, 0, HEIGHT - rectangle_move.h);
                    }
                    break;

                default:
                    break;
            }
        }

        // --- Render pass ---
        SDL_SetRenderDrawColor(renderer, BLACK);
        SDL_RenderClear(renderer);

        SDL_SetRenderDrawColor(renderer, WHITE);
        SDL_RenderFillRect(renderer, &middle_rect);
        SDL_RenderFillRect(renderer, &rectangle_move);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(mainWindow);
    SDL_Quit();
    return 0;
}

