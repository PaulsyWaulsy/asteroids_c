#include "vec.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_quit.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_shape.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_system.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <bits/types/timer_t.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*----------------------------------CONSTANTS---------------------------------*/
// Screen dimensions
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 800;

// Time constants
const float MS_TO_SECONDS_F = 1000.0f;
const Uint32 MS_TO_SECONDS = 1000;
const int MAX_FPS = 120;
const Uint32 TICK_PER_FRAME = MS_TO_SECONDS / MAX_FPS;

// Ship constant
const int NUM_SHIP_POINTS = 5;
const Vector2 INIT_SHIP_SHAPE[] = {
    {0, -15}, {11.25, 15}, {10.75, 11.25}, {-10.75, 11.25}, {-11.25, 15},
};

const int NUM_FLAME_POINTS = 3;
const Vector2 INIT_FLAME_SHAPE[] = {
    {9, 11.25},
    {-9, 11.25},
    {0, 26.25},
};

const int FLICKER_RATE = 3;
const float PLAYER_SPEED = 20.0f;
const float PLAYER_SHOOT_FORCE = 30.0f;
const float PLAYER_ROTATION_RATE = 0.05f;
const float PLAYER_DRAG = 0.02f;
const float VERTICLE = M_PI / 2;

const Vector2 VECTOR2_ZERO = {0, 0};

const float MIN_PARTICLE_SPEED = 20.0f;
const float MAX_PARTICLE_SPEED = 80.0f;
const float MIN_RADIUS = 2.0f;
const float MAX_RADIUS = 4.0f;

const int INIT_CAPACITY = 20;
const int INIT_NUM_ASTEROIDS = 20;

const float PROJ_SPEED = 1000.0f;
const Uint32 PROJ_TIME = 10000;
const int PROJ_THICKNESS = 2;
const int BROKEN_ASTEROID_NUM = 3;

const Uint32 RESPAWN_TIME = 2000;
const Uint32 FIRE_RATE = 80;

const int NUM_PARTICLES = 30;
const int NUM_LINES = 4;
const float LINE_RADIUS = 20.0f;

/*------------------------------------ENUMS-----------------------------------*/
typedef enum {
    OK,
    WINDOW_ERROR,
    GAME_ERROR,
} ExitStatus;

typedef enum {
    SMALL = 5,
    MEDIUM = 10,
    LARGE = 15,
} AsteroidSize;

const AsteroidSize ASTEROID_SIZES[] = {SMALL, MEDIUM, LARGE};
const AsteroidSize MIN_ASTEROID_SPEEDS[] = {100.0f, 40.0f, 20.0f};
const AsteroidSize MAX_ASTEROID_SPEEDS[] = {200.0f, 80.0f, 30.0f};

/*-----------------------------------STRUCTS----------------------------------*/
typedef struct {
    float deltaTime;
    Uint32 time;
    Uint32 frameTime;
    Uint32 lastSecond;
    Uint32 lastFrame;
    int frames;
    int fps;
} Time;

typedef struct {
    int quit; // bool that checks if should close the window/quit (key press)
    int width;
    int height;
    char* title;
    SDL_Window* window;
    SDL_Renderer* renderer;
} Window;

typedef struct {
    int moving;
    int shoot;
    int crashed;
    Vector2 position;
    Vector2 velocity;
    float rotation;
    Uint32 lastShot;
    Uint32 crashTime;
} Player;

typedef struct {
    Uint32 seed;
    Vector2 velocity;
    Vector2 position;
    AsteroidSize size;
} Asteroid;

typedef struct {
    Uint32 spawnTime;
    Vector2 velocity;
    Vector2 position;
} Projectile;

typedef struct {
    float angle;
    Uint32 spawnTime;
    Vector2 position;
    Vector2 velocity;
} Line;

typedef struct {
    Line** lines;
    Projectile** particles;
} CrashInfo;

typedef struct {
    int asteroidCapacity;
    int asteroidSize;
    int projectileCapacity;
    int projectileSize;
    Player* player;
    Asteroid** asteroids;
    Projectile** projectiles;
    CrashInfo* crashInfo;
} State;

/*----------------------------------PROTOTYPES--------------------------------*/
Time* init_time(void);
void update_time(Time* time);
void limit_fps(Time* time);
Window* init_window(const int width, const int height, const char* title);
void close_window(Window* window);
void update(Window* window, State* state, Time* time);
void render(Window* window, State* state, Uint32 time);
void handle_events(Window* window, SDL_Event* event, Player* player);
Player* init_ship(const float x, const float y);
State* init_state(void);
void free_player(Player* player);
void free_state(State* state);
void update_player(Player* player, float deltaTime);
void draw_player(SDL_Renderer* renderer, Player* player, Uint32 time);
Asteroid* init_asteroid(AsteroidSize size, Vector2 position, Uint32 seed);
void draw_asteroid(SDL_Renderer* renderer, Asteroid* asteroid);
void add_asteroid(State* state, Asteroid* asteroid);
void update_asteroids(Asteroid** asteroids, int size, float deltaTime);
void draw_asteroids(SDL_Renderer* renderer, Asteroid** asteroids, int size);
void spawn_asteroids(State* state, int num, Uint32 seed);
int asteroid_size_idx(AsteroidSize size);
Projectile* init_projectile(Vector2 position, float angle, Uint32 time);
void add_projectile(State* state, Uint32 time);
void update_projectile(Projectile* proj, float deltaTime);
void update_projectiles(Projectile** projectiles, int size, float deltaTime);
void draw_projectile(SDL_Renderer* renderer, Projectile* proj);
void draw_projectiles(SDL_Renderer* renderer, Projectile** projectiles,
                      int size);
void delete_projectiles(State* state, Uint32 time);
void update_shoot(State* state, Uint32 time);
void detect_crash(State* state, Uint32 time);
void detect_Shoot(State* state);
void on_destroy(State* state, AsteroidSize size, Vector2 position, Uint32 seed);
void on_crash(CrashInfo* crashInfo, Vector2 position, Uint32 time);
void respawn(Player* player);
CrashInfo* init_crashinfo(void);
void draw_crashinfo(SDL_Renderer* renderer, CrashInfo* crashInfo);
void update_crashinfo(CrashInfo* crashInfo, float deltaTime);
void free_crashinfo(CrashInfo* crashInfo);

int main() {
    Time* gameTime = init_time();
    Window* window = init_window(SCREEN_WIDTH, SCREEN_HEIGHT, "asteroids");
    State* state = init_state();

    // NOTE: test asteroid
    spawn_asteroids(state, INIT_NUM_ASTEROIDS, time(NULL));

    while (!window->quit) {
        update_time(gameTime);
        update(window, state, gameTime);
        render(window, state, gameTime->time);
        limit_fps(gameTime);
    }
    close_window(window);

    return 0;
}

Time* init_time() {
    Time* time = (Time*)malloc(sizeof(Time));
    time->time = 0;
    time->frames = 0;
    time->lastFrame = 0;
    time->deltaTime = 0;
    return time;
}

void update_time(Time* time) {
    // Calculate delta time and keep physics consistent
    time->time = SDL_GetTicks();
    time->deltaTime = (time->time - time->lastFrame) / MS_TO_SECONDS_F;
    time->lastFrame = time->time;
    time->frames++;
    if (time->deltaTime < 0) {
        time->deltaTime = 0;
    }
    if (time->time - time->lastSecond >= MS_TO_SECONDS) {
        time->lastSecond = time->time;
        time->fps = time->frames;
        time->frames = 0;
    }
}

void limit_fps(Time* time) {
    time->frameTime = SDL_GetTicks() - time->time;
    if (time->frameTime < TICK_PER_FRAME) {
        SDL_Delay(TICK_PER_FRAME - time->frameTime);
    }
}

Window* init_window(const int width, const int height, const char* title) {
    Window* window = (Window*)malloc(sizeof(Window));
    window->height = height;
    window->width = width;
    window->title = strdup(title);
    window->quit = 0;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize!\n");
        free(window);
        exit(WINDOW_ERROR);
    }

    window->window = SDL_CreateWindow(
        title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window->width,
        window->height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (window->window == NULL) {
        fprintf(stderr, "Window could not be created!\n");
        free(window);
        exit(WINDOW_ERROR);
    }

    window->renderer =
        SDL_CreateRenderer(window->window, -1, SDL_RENDERER_ACCELERATED);
    if (window->renderer == NULL) {
        SDL_DestroyWindow(window->window);
        free(window);
        exit(WINDOW_ERROR);
    }
    return window;
}

void close_window(Window* window) {
    if (!window) {
        return;
    }

    if (window->title) {
        free(window->title);
    }

    if (window->window) {
        SDL_DestroyWindow(window->window);
    }

    if (window->renderer) {
        SDL_DestroyRenderer(window->renderer);
    }
    free(window);
    SDL_Quit();
}

void update(Window* window, State* state, Time* time) {
    SDL_Event event;
    handle_events(window, &event, state->player);

    // Update logic goes here
    float deltaTime = time->deltaTime;
    update_player(state->player, deltaTime);
    update_shoot(state, time->time);
    update_asteroids(state->asteroids, state->asteroidSize, deltaTime);
    update_projectiles(state->projectiles, state->projectileSize, deltaTime);
    delete_projectiles(state, time->time);
    detect_Shoot(state);

    if (!state->player->crashed) {
        detect_crash(state, time->time);
    }

    if (state->player->crashed) {
        update_crashinfo(state->crashInfo, deltaTime);
        if ((time->time - state->player->crashTime) >= RESPAWN_TIME) {
            respawn(state->player);
        }
    }
}

void render(Window* window, State* state, Uint32 time) {
    SDL_SetRenderDrawColor(window->renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(window->renderer);

    draw_asteroids(window->renderer, state->asteroids, state->asteroidSize);
    draw_projectiles(window->renderer, state->projectiles,
                     state->projectileSize);

    if (!state->player->crashed) {
        draw_player(window->renderer, state->player, time);
    }

    if (state->player->crashed) {
        draw_crashinfo(window->renderer, state->crashInfo);
    }
    SDL_RenderPresent(window->renderer);
}

void handle_events(Window* window, SDL_Event* event, Player* player) {
    while (SDL_PollEvent(event)) {
        switch (event->type) {
        case SDL_QUIT:
            window->quit = 1;
            break;
        case SDL_KEYDOWN:
            // If the ECS key is presed close the program
            if (event->key.keysym.sym == SDLK_ESCAPE) {
                window->quit = 1;
            }
            break;
        case SDL_WINDOWEVENT:
            if (event->window.event == SDL_WINDOWEVENT_RESIZED) {
                window->width = event->window.data1;
                window->height = event->window.data2;
            }
            break;
        }
    }

    if (!player->crashed) {
        const Uint8* state = SDL_GetKeyboardState(NULL);
        // Forward movement/thrusters
        if (state[SDL_SCANCODE_UP]) {
            float dX = cos(player->rotation) * -PLAYER_SPEED;
            float dY = sin(player->rotation) * -PLAYER_SPEED;
            Vector2 movement = create_vector(dX, dY);
            player->moving = 1;
            player->velocity = vector_sum(player->velocity, movement);
        } else {
            player->moving = 0; // Set the moving flag back to 0
        }

        // Rotation left
        if (state[SDL_SCANCODE_LEFT]) {
            player->rotation -= PLAYER_ROTATION_RATE;
        }

        // Rotation right
        if (state[SDL_SCANCODE_RIGHT]) {
            player->rotation += PLAYER_ROTATION_RATE;
        }

        if (state[SDL_SCANCODE_SPACE]) {
            player->shoot = 1;
        }
    }
}

Player* init_ship(const float x, const float y) {
    Player* player = (Player*)malloc(sizeof(Player));
    player->moving = 0;
    player->shoot = 0;
    player->crashed = 0;
    player->position = (Vector2){x, y};
    player->velocity = (Vector2){0, 0};
    player->rotation = VERTICLE;
    player->lastShot = 0;
    return player;
}

State* init_state(void) {
    State* state = (State*)malloc(sizeof(State));
    state->player = init_ship(SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0);
    state->asteroidSize = 0;
    state->asteroidCapacity = INIT_CAPACITY;
    state->asteroids = (Asteroid**)malloc(sizeof(Asteroid*) * INIT_CAPACITY);

    state->crashInfo = init_crashinfo();
    state->projectileSize = 0;
    state->projectileCapacity = INIT_CAPACITY;
    state->projectiles =
        (Projectile**)malloc(sizeof(Projectile*) * INIT_CAPACITY);
    return state;
}

void free_player(Player* player) {
    free(player);
}

void free_state(State* state) {
    free_player(state->player);
    free_crashinfo(state->crashInfo);
    for (int i = 0; i < state->asteroidSize; i++) {
        free(state->asteroids[i]);
    }
    free(state->asteroids);

    for (int i = 0; i < state->projectileSize; i++) {
        free(state->projectiles[i]);
    }
    free(state->projectiles);
    free(state);
}

void free_crashinfo(CrashInfo* crashInfo) {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        free(crashInfo->particles[i]);
    }

    for (int i = 0; i < NUM_LINES; i++) {
        free(crashInfo->lines[i]);
    }
    free(crashInfo->particles);
    free(crashInfo->lines);
    free(crashInfo);
}


void update_player(Player* player, float deltaTime) {
    // Ensure there is a constant frictional/drag on the ship
    player->velocity = vector_mul(player->velocity, (1.0f - PLAYER_DRAG));

    float newX = player->position.x + player->velocity.x * deltaTime;
    float newY = player->position.y + player->velocity.y * deltaTime;
    newX = fmod(newX + SCREEN_WIDTH, SCREEN_WIDTH);
    newY = fmod(newY + SCREEN_HEIGHT, SCREEN_HEIGHT);

    player->position = create_vector(newX, newY);
}

void draw_player(SDL_Renderer* renderer, Player* player, Uint32 time) {
    Vector2 ship[NUM_SHIP_POINTS];
    for (int i = 0; i < NUM_SHIP_POINTS; i++) {
        Vector2 pos = vector_sum(player->position, INIT_SHIP_SHAPE[i]);
        ship[i] =
            vector_aro(pos, player->position, (player->rotation - (M_PI / 2)));
    }

    Vector2 flame[NUM_FLAME_POINTS];
    for (int i = 0; i < NUM_FLAME_POINTS; i++) {
        Vector2 pos = vector_sum(player->position, INIT_FLAME_SHAPE[i]);
        flame[i] =
            vector_aro(pos, player->position, (player->rotation - (M_PI / 2)));
    }

    draw_shape(renderer, ship, NUM_SHIP_POINTS);
    if (player->moving && ((time % FLICKER_RATE) == 0)) {
        draw_shape(renderer, flame, NUM_FLAME_POINTS);
    }
}

float rand_float(float min, float max) {
    return min + (max - min) * ((float)rand() / RAND_MAX);
}

void update_asteroid(Asteroid* asteroid, float deltaTime) {
    float newX = asteroid->position.x + asteroid->velocity.x * deltaTime;
    float newY = asteroid->position.y + asteroid->velocity.y * deltaTime;
    newX = fmod(newX + SCREEN_WIDTH, SCREEN_WIDTH);
    newY = fmod(newY + SCREEN_HEIGHT, SCREEN_HEIGHT);

    asteroid->position = create_vector(newX, newY);
}

void add_asteroid(State* state, Asteroid* asteroid) {
    if (state->asteroidSize == state->asteroidCapacity - 1) {
        state->asteroidCapacity *= 2;
        state->asteroids = (Asteroid**)realloc(
            state->asteroids, sizeof(Asteroid*) * state->asteroidCapacity);
    }
    state->asteroids[state->asteroidSize++] = asteroid;
}

Asteroid* init_asteroid(AsteroidSize size, Vector2 position, Uint32 seed) {
    Asteroid* asteroid = (Asteroid*)malloc(sizeof(Asteroid));
    asteroid->size = size;
    asteroid->seed = seed;
    asteroid->position = position;

    int idx = asteroid_size_idx(size);
    float speed =
        rand_float(MIN_ASTEROID_SPEEDS[idx], MAX_ASTEROID_SPEEDS[idx]);
    float angle = rand_float(0, (2.0f * M_PI)); // Any angle within circle
    float dX = cos(angle) * speed;
    float dY = sin(angle) * speed;
    asteroid->velocity = create_vector(dX, dY);
    return asteroid;
}

void update_asteroids(Asteroid** asteroids, int size, float deltaTime) {
    for (int i = 0; i < size; i++) {
        Asteroid* asteroid = asteroids[i];
        update_asteroid(asteroid, deltaTime);
    }
}

void draw_asteroids(SDL_Renderer* renderer, Asteroid** asteroids, int size) {
    for (int i = 0; i < size; i++) {
        Asteroid* asteroid = asteroids[i];
        draw_asteroid(renderer, asteroid);
    }
}

void draw_asteroid(SDL_Renderer* renderer, Asteroid* asteroid) {

    srand(asteroid->seed); // Set the seed for the given asteroids
    int size = asteroid->size;
    Vector2 points[size];

    float angleStep = (2 * M_PI) / (float)size; // get the step of each
    for (int i = 0; i < (int)size; i++) {
        float radius = size * rand_float(MIN_RADIUS, MAX_RADIUS);
        float angle = angleStep * i;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;
        Vector2 vector = create_vector(x, y);
        points[i] = vector_sum(asteroid->position, vector);
    }
    draw_shape(renderer, points, (int)size);
}

void spawn_asteroids(State* state, int num, Uint32 seed) {
    for (int i = 0; i < num; i++) {
        seed = rand();
        AsteroidSize size = ASTEROID_SIZES[seed % 3];

        srand(seed);
        float x = rand_float(0, SCREEN_WIDTH);
        float y = rand_float(0, SCREEN_HEIGHT);
        Vector2 position = create_vector(x, y);
        Asteroid* asteroid = init_asteroid(size, position, rand());
        add_asteroid(state, asteroid);
    }
}

int asteroid_size_idx(AsteroidSize size) {
    switch (size) {
    case SMALL:
        return 0;
        break;
    case MEDIUM:
        return 1;
        break;
    case LARGE:
        return 1;
        break;
    default:
        break;
    }
    return -1;
}

Projectile* init_projectile(Vector2 position, float angle, Uint32 time) {
    Projectile* proj = (Projectile*)malloc(sizeof(Projectile));
    proj->spawnTime = time;
    proj->position = position;

    float dX = cos(angle) * PROJ_SPEED;
    float dY = sin(angle) * PROJ_SPEED;
    proj->velocity = create_vector(-dX, -dY);
    return proj;
}

void add_projectile(State* state, Uint32 time) {
    Player* player = state->player;
    Projectile* proj =
        init_projectile(player->position, player->rotation, time);

    if (state->projectileSize == state->projectileCapacity - 1) {
        state->projectileCapacity *= 2;
        state->projectiles = (Projectile**)realloc(
            state->projectiles,
            sizeof(Projectile*) * state->projectileCapacity);
    }
    state->projectiles[state->projectileSize++] = proj;
}

void update_projectile(Projectile* proj, float deltaTime) {
    Vector2 delta = vector_mul(proj->velocity, deltaTime);
    proj->position = vector_sum(proj->position, delta);
}

void update_projectiles(Projectile** projectiles, int size, float deltaTime) {
    for (int i = 0; i < size; i++) {
        update_projectile(projectiles[i], deltaTime);
    }
}

void shoot(State* state, Uint32 time) { add_projectile(state, time); }

void draw_projectile(SDL_Renderer* renderer, Projectile* proj) {
    draw_thick_point(renderer, proj->position.x, proj->position.y,
                     PROJ_THICKNESS);
}

void draw_projectiles(SDL_Renderer* renderer, Projectile** projectiles,
                      int size) {
    for (int i = 0; i < size; i++) {
        draw_projectile(renderer, projectiles[i]);
    }
}

void delete_projectiles(State* state, Uint32 time) {
    int i = 0;
    while (i < state->projectileSize) {
        if ((time - state->projectiles[i]->spawnTime) >= PROJ_TIME) {
            free(state->projectiles[i]);
            for (int j = i; j < state->projectileSize - 1; j++) {
                state->projectiles[j] = state->projectiles[j + 1];
            }
            state->projectileSize--; // Reduce the count
        } else {
            i++; // Only move forward if no deletion occurs
        }
    }
}

void update_shoot(State* state, Uint32 time) {
    if (state->player->shoot && (time - state->player->lastShot) > FIRE_RATE) {
        state->player->lastShot = time;
        add_projectile(state, time);

        float dX = cos(state->player->rotation) * PLAYER_SHOOT_FORCE;
        float dY = sin(state->player->rotation) * PLAYER_SHOOT_FORCE;
        Vector2 delta = create_vector(dX, dY);
        state->player->velocity = vector_sum(state->player->velocity, delta);

    } else {
        state->player->shoot = 0;
    }
}

// Check if a specific point is inside radius of asteroids
// distance^2=(x−cx)^2+(y−cy)^2
void detect_crash(State* state, Uint32 time) {
    Vector2 position = state->player->position;
    for (int i = 0; i < state->asteroidSize; i++) {
        Asteroid* asteroid = state->asteroids[i];
        float radius = asteroid->size * MAX_RADIUS;
        float dX = position.x - asteroid->position.x;
        float dY = position.y - asteroid->position.y;
        if ((dX * dX + dY * dY) <= (radius * radius)) {
            state->player->crashed = 1;
            state->player->crashTime = time;
            on_crash(state->crashInfo, state->player->position, time);
        }
    }
}

void respawn(Player* player) {
    player->position = create_vector(SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f);
    player->crashed = 0;
}

void detect_Shoot(State* state) {
    for (int i = 0; i < state->projectileSize; i++) {
        Vector2 position = state->projectiles[i]->position;
        for (int j = 0; j < state->asteroidSize; j++) {
            Asteroid* asteroid = state->asteroids[j];
            float radius = asteroid->size * MAX_RADIUS;
            float dX = position.x - asteroid->position.x;
            float dY = position.y - asteroid->position.y;
            if ((dX * dX + dY * dY) <= (radius * radius)) {
                AsteroidSize size = asteroid->size;
                Vector2 position = asteroid->position;
                Uint32 seed = asteroid->seed;
                free(asteroid);
                for (int k = j; k < state->asteroidSize - 1; k++) {
                    state->asteroids[k] = state->asteroids[k + 1];
                }
                state->asteroidSize--; // Reduce the count
                j--;

                on_destroy(state, size, position, seed);

                free(state->projectiles[i]);
                for (int l = i; l < state->projectileSize - 1; l++) {
                    state->projectiles[l] = state->projectiles[l + 1];
                }
                state->projectileSize--; // Reduce the count
                i--;
                break;
            }
        }
    }
}

void on_destroy(State* state, AsteroidSize size, Vector2 position,
                Uint32 seed) {
    if (size == MEDIUM) {
        for (int i = 0; i < BROKEN_ASTEROID_NUM; i++) {
            seed = rand();
            AsteroidSize brokenSize = SMALL;
            srand(seed);
            Asteroid* asteroid = init_asteroid(brokenSize, position, seed);
            add_asteroid(state, asteroid);
        }
    } else if (size == LARGE) {
        for (int i = 0; i < BROKEN_ASTEROID_NUM; i++) {
            seed = rand();
            AsteroidSize brokenSize = MEDIUM;
            srand(seed);
            Asteroid* asteroid = init_asteroid(brokenSize, position, seed);
            add_asteroid(state, asteroid);
        }
    }
}

void on_crash(CrashInfo* crashInfo, Vector2 position, Uint32 time) {
    srand(time);
    for (int i = 0; i < NUM_PARTICLES; i++) {
        rand();
        float speed = rand_float(MIN_PARTICLE_SPEED, MAX_PARTICLE_SPEED);
        float angle = rand_float(0, (2.0f * M_PI)); // Any angle within circle
        float dX = cos(angle) * speed;
        float dY = sin(angle) * speed;
        Vector2 velocity = create_vector(dX, dY);
        crashInfo->particles[i]->position = position;
        crashInfo->particles[i]->velocity = velocity;
        crashInfo->particles[i]->spawnTime = time;
    }

    for (int i = 0; i < NUM_LINES; i++) {
        rand();
        float speed = rand_float(MIN_PARTICLE_SPEED, MAX_PARTICLE_SPEED);
        float angle = rand_float(0, (2.0f * M_PI)); // Any angle within circle
        float dir = rand_float(0, (2.0f * M_PI));   // Any angle within circle
        float dX = cos(dir) * speed;
        float dY = sin(dir) * speed;
        Vector2 velocity = create_vector(dX, dY);
        crashInfo->lines[i]->velocity = velocity;
        crashInfo->lines[i]->position = position;
        crashInfo->lines[i]->angle = angle;
    }
}

void update_crashinfo(CrashInfo* crashInfo, float deltaTime) {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        Vector2 delta =
            vector_mul(crashInfo->particles[i]->velocity, deltaTime);
        Vector2 position = crashInfo->particles[i]->position;
        crashInfo->particles[i]->position = vector_sum(position, delta);
    }

    for (int i = 0; i < NUM_LINES; i++) {
        Vector2 delta = vector_mul(crashInfo->lines[i]->velocity, deltaTime);
        Vector2 position = crashInfo->lines[i]->position;
        crashInfo->lines[i]->position = vector_sum(position, delta);
    }
}

void draw_crashinfo(SDL_Renderer* renderer, CrashInfo* crashInfo) {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        draw_projectile(renderer, crashInfo->particles[i]);
    }

    for (int i = 0; i < NUM_LINES; i++) {
        float angle = crashInfo->lines[i]->angle;
        Vector2 position = crashInfo->lines[i]->position;

        float x = position.x + cos(angle) * LINE_RADIUS;
        float y = position.y + sin(angle) * LINE_RADIUS;
        Vector2 end = create_vector(x, y);
        draw_line(renderer, position, end);
    }
}

CrashInfo* init_crashinfo(void) {
    CrashInfo* crashInfo = (CrashInfo*)malloc(sizeof(CrashInfo));
    crashInfo->particles =
        (Projectile**)malloc(sizeof(Projectile*) * NUM_PARTICLES);
    crashInfo->lines = (Line**)malloc(sizeof(Line*) * NUM_LINES);

    for (int i = 0; i < NUM_PARTICLES; i++) {
        crashInfo->particles[i] = (Projectile*)malloc(sizeof(Projectile));
        crashInfo->particles[i]->spawnTime = 0;
        crashInfo->particles[i]->position = create_vector(0, 0);
        crashInfo->particles[i]->velocity = create_vector(0, 0);
    }

    for (int i = 0; i < NUM_LINES; i++) {
        crashInfo->lines[i] = (Line*)malloc(sizeof(Line));
        crashInfo->lines[i]->spawnTime = 0;
        crashInfo->lines[i]->angle = 0;
        crashInfo->lines[i]->position = create_vector(0, 0);
        crashInfo->lines[i]->velocity = create_vector(0, 0);
    }
    return crashInfo;
}
