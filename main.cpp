
/*
    Compilation:
    g++ -Wall -Wextra main.cpp -o demo `sdl2-config --cflags --libs`
    Notes: Move the left rectangle with keyboard keys (ready)
    add a score punctuation on the terminal (ready)
    add a pause button 
    add score on the screen
    
*/
#include <iostream>
#include <SDL2/SDL.h>
#include <random>
#include <cmath>
using namespace std;

#define WIDTH 740 //Width of the main window 
#define HEIGHT 440 //Height of the main window 
#define BLACK 0, 0, 0, 255 // hex black color 
#define WHITE 255, 255, 255, 255 // hex withe color 

struct Ball {
    float x, y;       // Center
    float vx, vy;     // Velocity (pixels/sec)
    int   r;          // Radius
};

// Random float in [a, b]
static float randf(mt19937& rng, float a, float b) {
    uniform_real_distribution<float> dist(a, b);
    return dist(rng);
}

// Utility so the rectangle can't leave the window.
static inline int clampi(int v, int lo, int hi) {
    return (v < lo) ? lo : (v > hi ? hi : v);
}

// Declare CPU rect at file scope or inside main before use.
SDL_Rect cpu_rect = { WIDTH - 40, 1, 20, 60 };

// Follow-ball CPU paddle movement (Y only), with speed and clamping
void cpu_rectangle_move(SDL_Rect& cpu, const Ball& ball, float dt) {

    // Keep the right square on the screen
    cpu.x = WIDTH - 40;                    

    // Setting the ball speed
    const float paddleSpeed = 380.0f;      // Pixels/sec
    float targetY = ball.y - cpu.h * 0.5f; // Center paddle on ball
    // Setting the velocity of the rectangle depending of the velocity of the ball
    if (targetY > cpu.y) {
        cpu.y = static_cast<int>(cpu.y + paddleSpeed * dt);
        if (cpu.y > targetY) cpu.y = static_cast<int>(targetY);
    } else if (targetY < cpu.y) {
        cpu.y = static_cast<int>(cpu.y - paddleSpeed * dt);
        if (cpu.y < targetY) cpu.y = static_cast<int>(targetY);
    }
    //Keep the rectangle into the mainWindow 
    cpu.y = clampi(cpu.y, 0, HEIGHT - cpu.h);
}

// Drawing the ball function
void draw_circle_pixels(SDL_Renderer* renderer, int cx, int cy, int r) {
    int x = r - 1;
    int y = 0;
    int tx = 1;
    int ty = 1;
    int err = tx - (r << 1); // == tx - 2*r

    while (x >= y) {
        // 8-way symmetry
        SDL_RenderDrawPoint(renderer, cx + x, cy - y);
        SDL_RenderDrawPoint(renderer, cx + x, cy + y);
        SDL_RenderDrawPoint(renderer, cx - x, cy - y);
        SDL_RenderDrawPoint(renderer, cx - x, cy + y);
        SDL_RenderDrawPoint(renderer, cx + y, cy - x);
        SDL_RenderDrawPoint(renderer, cx + y, cy + x);
        SDL_RenderDrawPoint(renderer, cx - y, cy - x);
        SDL_RenderDrawPoint(renderer, cx - y, cy + x);

        if (err <= 0) {
            y++;
            err += ty;
            ty += 2;
        }
        if (err > 0) {
            x--;
            tx += 2;
            err += (tx - (r << 1));
        }
    }
}

// Fills a circle centered at (cx, cy) with radius r using horizontal spans
void fill_circle(SDL_Renderer* renderer, int cx, int cy, int r) {
    int x = r;
    int y = 0;
    int err = 1 - x; // Midpoint circle decision variable

    while (x >= y) {
        // Draw four horizontal spans for the current y
        SDL_RenderDrawLine(renderer, cx - x, cy + y, cx + x, cy + y);
        SDL_RenderDrawLine(renderer, cx - x, cy - y, cx + x, cy - y);
        SDL_RenderDrawLine(renderer, cx - y, cy + x, cx + y, cy + x);
        SDL_RenderDrawLine(renderer, cx - y, cy - x, cx + y, cy - x);

        y++;
        if (err < 0) {
            err += 2*y + 1;
        } else {
            x--;
            err += 2*(y - x + 1);
        }
    }
}


int main(int, char*[]) {

    // Initialize the video and verification 
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
        return -1;
    }
    // Creathe the window saved in mainWindow 
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
    // Render the graphics 
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

    // RNG
    random_device rd;
    mt19937 rng(rd());

    // Ball setup
    Ball ball;
    ball.r  = 15;
    ball.x  = 200.0f;
    ball.y  = 150.0f;

    // Random direction at constant speed
    float speed = 400.0f; // pixels per second
    const float TWO_PI = 6.283185307179586f;
    float angle = randf(rng, 0.0f, TWO_PI);
    //Angles when the ball hits 
    ball.vx = cos(angle) * speed; 
    ball.vy = sin(angle) * speed;

    // Timing 
    Uint32 last_ticks = SDL_GetTicks();

    // The draggable rectangle on the left 
    SDL_Rect rectangle_move = {20, 1, 20, 60};

    // Drawing the rectangle in the starting position
    SDL_Rect middle_rect = {(WIDTH / 2) - 10, 0, 20, HEIGHT};

    // Quit flag
    bool quit = false;
    // Event set
    SDL_Event e;
    // Flag if the ball hit the wall 
    int hit = 0;
    const float paddleSpeed = 380.0f;  // pixels/sec for the left paddle
    // Main loop 
    while (!quit) {
        // Events
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                // Quit button
                case SDL_QUIT:
                    quit = true;
                    break;
            }
        }

        const Uint8 *keyState = SDL_GetKeyboardState(NULL);


        // --- Time step for the ball ---
        Uint32 now = SDL_GetTicks();
        float dt = (now - last_ticks) / 1000.0f;
        last_ticks = now;

        // --- Update ball position ---
        float prevX = ball.x;
        float prevY = ball.y;
        ball.x += ball.vx * dt;
        ball.y += ball.vy * dt;
        //Pause 
        if (keyState[SDL_SCANCODE_ESCAPE]){
            cout << "Game paused!\nPress space to resume it!\n";
            //bool key_scape_flag = false;
            ball.vx = 0;
            ball.vy = 0;
        }
        //Resume
        if (keyState[SDL_SCANCODE_SPACE]){
            cout << "Game resumed!\n";
            ball.vx = cos(angle) * speed;
            ball.vy = sin(angle) * speed;
            prevX = ball.x;
            prevY = ball.y;
            ball.x += ball.vx * dt;
            ball.y += ball.vy * dt;
        }        
        // keyboard settings 
        if (keyState[SDL_SCANCODE_W]| keyState[SDL_SCANCODE_UP]) {
            rectangle_move.y -= static_cast<int>(paddleSpeed * dt);
            //ball.x += ball.vx * dt;
            //ball.y += ball.vy * dt;
        }
        if (keyState[SDL_SCANCODE_S] | keyState[SDL_SCANCODE_DOWN]){
            rectangle_move.y += static_cast<int>(paddleSpeed * dt);
        }
        // Cap if to don't go outside the screen
        rectangle_move.y = clampi(rectangle_move.y, 0, HEIGHT - rectangle_move.h);

        // --- Bounce on walls ---
        if (ball.y - ball.r < 0.0f) {
            ball.y = static_cast<float>(ball.r);
            ball.vy = -ball.vy;
        }
        if (ball.y + ball.r > static_cast<float>(HEIGHT)) {
            ball.y = static_cast<float>(HEIGHT - ball.r);
            ball.vy = -ball.vy;
        }
        
        if (ball.x - ball.r < 0.0f) {
            ball.x = static_cast<float>(ball.r);
            ball.vx = -ball.vx;
            hit ++;
            cout << "The ball hit the left wall " << hit << " times" << endl;
            }
            
        if (ball.x + ball.r > static_cast<float>(WIDTH)) {
            ball.x = static_cast<float>(WIDTH - ball.r);
            ball.vx = -ball.vx;
        }

        // --- Paddle (left rectangle) collision ---
        if (ball.vx < 0.0f) {
            float paddleRight = static_cast<float>(rectangle_move.x + rectangle_move.w);
            float nextLeft = ball.x - ball.r;
            float prevLeft = prevX - ball.r;
            if (prevLeft >= paddleRight && nextLeft <= paddleRight) {
                float top    = static_cast<float>(rectangle_move.y) - ball.r;
                float bottom = static_cast<float>(rectangle_move.y + rectangle_move.h) + ball.r;
                if (ball.y >= top && ball.y <= bottom) {
                    ball.x = paddleRight + ball.r;
                    ball.vx = -ball.vx;
                    float rel = ((ball.y - rectangle_move.y) - (rectangle_move.h * 0.5f)) / (rectangle_move.h * 0.5f);
                    ball.vy += rel * 250.0f;
                    float mag = sqrt(ball.vx*ball.vx + ball.vy*ball.vy);
                    if (mag > 0.0f) {
                        ball.vx = (ball.vx / mag) * speed;
                        ball.vy = (ball.vy / mag) * speed;
                    }
                }
            }
        }
        
        // --- Paddle (right rectangle) collision ---
        if (ball.vx > 0.0f) {
            float paddleLeft = static_cast<float>(cpu_rect.x);
            float nextRight = ball.x + ball.r;
            float prevRight = prevX + ball.r;
            if (prevRight <= paddleLeft && nextRight >= paddleLeft) {
                float top    = static_cast<float>(cpu_rect.y) - ball.r;
                float bottom = static_cast<float>(cpu_rect.y + cpu_rect.h) + ball.r;
                if (ball.y >= top && ball.y <= bottom) {
                    ball.x = paddleLeft - ball.r;
                    ball.vx = -ball.vx;
                    float rel = ((ball.y - cpu_rect.y) - (cpu_rect.h * 0.5f)) / (cpu_rect.h * 0.5f);
                    ball.vy += rel * 250.0f;
                    float mag = sqrt(ball.vx*ball.vx + ball.vy*ball.vy);
                    if (mag > 0.0f) {
                        ball.vx = (ball.vx / mag) * speed;
                        ball.vy = (ball.vy / mag) * speed;
                    }
                }
            }
        }
        
        // Clear the first screen to render the rest
        SDL_SetRenderDrawColor(renderer, BLACK);
        SDL_RenderClear(renderer);

        // Make the CPU follow the ball here (after dt is computed)
        cpu_rectangle_move(cpu_rect, ball, dt); // updates & clamps once per frame

        // Set draw color first (RGBA)
        SDL_SetRenderDrawColor(renderer, WHITE); // circle white

        // (Filled circle first, so the outline sits on top)
        fill_circle(renderer,
                    static_cast<int>(ball.x),
                    static_cast<int>(ball.y),
                    ball.r);

        // Optional: outline for a crisp edge
        draw_circle_pixels(renderer,
                           static_cast<int>(ball.x),
                           static_cast<int>(ball.y),
                           ball.r);

        SDL_SetRenderDrawColor(renderer, WHITE);
        SDL_RenderFillRect(renderer, &middle_rect);
        SDL_RenderFillRect(renderer, &rectangle_move);
        SDL_RenderFillRect(renderer, &cpu_rect);

        SDL_RenderPresent(renderer);
        
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(mainWindow);
    SDL_Quit();
    return 0;
}
