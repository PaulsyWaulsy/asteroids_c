#include "vec.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keyboard.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_mutex.h>
#include <SDL2/SDL_quit.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_shape.h>
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_system.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/*----------------------------------CONSTANTS---------------------------------*/

const char* const EXPLOSION_PATH = "../sounds/explosion.wav";
const char* const SHOOT_PATH = "../sounds/shoot.wav";
const char* const HIT_PATH = "../sounds/hit.wav";
const char* const ALIEN_PATH = "../sounds/alien.wav";
const char* const RAN_PATH = "../sounds/random.wav";

// Screen dimensions
const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 800;

// Time constants
const float MS_TO_SECONDS_F = 1000.0f;
const Uint32 MS_TO_SECONDS = 1000;
const int MAX_FPS = 240;
const Uint32 TICK_PER_FRAME = MS_TO_SECONDS / MAX_FPS;

// Ship constant
const int NUM_SHIP_POINTS = 5;
const Vector2 INIT_SHIP_SHAPE[] = {
    {0, -15}, {11.25, 15}, {10.75, 11.25}, {-10.75, 11.25}, {-11.25, 15},
};

const int NUM_ALIEN_POINTS = 13;
const Vector2 INIT_ALIEN_SHAPE[] = {
    {-10, 30}, {10, 30}, {25, 25},  {15, 15},  {10, 15},  {10, 5},  {5, 0},
    {-5, 0},   {-10, 5}, {-10, 15}, {-15, 15}, {-25, 25}, {-10, 30}};

const int NUM_FLAME_POINTS = 3;
const Vector2 INIT_FLAME_SHAPE[] = {
    {9, 11.25},
    {-9, 11.25},
    {0, 26.25},
};

const Vector2 DIGIT_POINTS[][7] = {
    {{-10, 15}, {10, 15}, {10, -15}, {-10, -15}, {-10, 15}},         // 0
    {{10, 15}, {10, -15}},                                           // 1
    {{-10, -15}, {10, -15}, {10, 0}, {-10, 0}, {-10, 15}, {10, 15}}, // 2
    {{-10, -15},
     {10, -15},
     {10, 0},
     {-10, 0},
     {10, 0},
     {10, 15},
     {-10, 15}},                                                     // 3
    {{-10, -15}, {-10, 0}, {10, 0}, {10, -15}, {10, 15}},            // 4
    {{10, -15}, {-10, -15}, {-10, 0}, {10, 0}, {10, 15}, {-10, 15}}, // 5
    {{-10, -15}, {-10, 15}, {10, 15}, {10, 0}, {-10, 0}},            // 6
    {{-10, -15}, {10, -15}, {10, 15}},                               // 7
    {{-10, 15},
     {-10, -15},
     {10, -15},
     {10, 15},
     {-10, 15},
     {-10, 0},
     {10, 0}},                                           // 8
    {{10, 15}, {10, -15}, {-10, -15}, {-10, 0}, {10, 0}} // 9
};

const int DIGIT_COUNTS[] = {5, 2, 6, 7, 5, 6, 5, 3, 7, 5};

const int FLICKER_RATE = 3;
const float PLAYER_SPEED = 2000.0f;
const float PLAYER_SHOOT_FORCE = 30.0f;
const float PLAYER_ROTATION_RATE = 5.5f;
const float PLAYER_DRAG = 3.00f;
const float VERTICLE = M_PI / 2;

const Vector2 VECTOR2_ZERO = {0, 0};

const float MIN_PARTICLE_SPEED = 20.0f;
const float MAX_PARTICLE_SPEED = 80.0f;
const float MIN_RADIUS = 2.0f;
const float MAX_RADIUS = 4.0f;

const int INIT_CAPACITY = 20;
const int INIT_NUM_ASTEROIDS = 5;

const float PROJ_SPEED = 1000.0f;
const Uint32 PROJ_TIME = 10000;
const int PROJ_THICKNESS = 2;
const int BROKEN_ASTEROID_NUM = 2;

const Uint32 RESPAWN_TIME = 2000;
const Uint32 FIRE_RATE = 150;

const int NUM_PARTICLES = 30;
const int NUM_LINES = 4;
const float LINE_RADIUS = 20.0f;
const float DIGIT_WIDTH = 35.0f;
const float DIGIT_HEIGHT = 40.0f;
const float DRIFT_FRACTION = 0.4f;

const float ALIEN_SPEED = 150.0f;
const Uint32 ALIEN_FIRE_RATE = 1000;
const float PLAYER_SIZE = 15.0f;
const float ALIEN_SIZE = 25.0f;
const Uint32 PLAYER_SAFE_TIME = 1000;

/*------------------------------------ENUMS-----------------------------------*/
typedef enum {
    OK,
    WINDOW_ERROR,
    GAME_ERROR,
} ExitStatus;

typedef enum {
    SMALL = 5,
    MEDIUM = 9,
    LARGE = 12,
} AsteroidSize;

typedef enum {
    SMALL_POINTS = 8,
    MEDIUM_POINTS = 10,
    LARGE_POINTS = 13,
} AsteroidPoints;

typedef enum {
    SMALL_SCORE = 100,
    MEDIUM_SCORE = 50,
    LARGE_SCORE = 20,
} AsteroidScores;

const AsteroidScores SCORES[] = {SMALL_SCORE, MEDIUM_SCORE, LARGE_SCORE};
const AsteroidPoints ASTEROID_POINTS[] = {SMALL_POINTS, MEDIUM_POINTS,
                                          LARGE_POINTS};

const AsteroidSize ASTEROID_SIZES[] = {SMALL, MEDIUM, LARGE};
const float MIN_ASTEROID_SPEEDS[] = {100.0f, 40.0f, 20.0f};
const float MAX_ASTEROID_SPEEDS[] = {200.0f, 80.0f, 30.0f};

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
    int hit;
    float rotation; // shoot angle
    Uint32 lastShot;
    Vector2 position;
    Vector2 velocity;
} Alien;

typedef struct {
    Mix_Chunk* explosion;
    Mix_Chunk* shoot;
    Mix_Chunk* hit;
    Mix_Chunk* alien;
    Mix_Chunk* ran;
} SoundManager;

typedef struct {
    int score;
    int asteroidCapacity;
    int asteroidSize;
    int projectileCapacity;
    int projectileSize;
    int alienProjCapacity;
    int alienProjSize;
    int level;
    Player* player;
    Alien* alien;
    Asteroid** asteroids;
    Projectile** projectiles;
    Projectile** alienProjs;
    CrashInfo* crashInfo;
    SoundManager* sounds;
} State;

/*----------------------------------PROTOTYPES--------------------------------*/
Time* init_time(void);
void update_time(Time* time);
void limit_fps(Time* time);
Window* init_window(const int width, const int height, const char* title);
void close_window(Window* window);
void update(Window* window, State* state, Time* gameTime);
void render(Window* window, State* state, Uint32 time);
void handle_events(Window* window, SDL_Event* event, Player* player,
                   float deltaTime);
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
void update_shoot(State* state, Time* time);
void detect_crash(State* state, Uint32 time);
void detect_Shoot(State* state);
void on_destroy(State* state, AsteroidSize size, Vector2 position, Uint32 seed);
void on_crash(CrashInfo* crashInfo, Player* player, Uint32 time);
void respawn(Player* player);
CrashInfo* init_crashinfo(void);
void draw_crashinfo(SDL_Renderer* renderer, CrashInfo* crashInfo);
void update_crashinfo(CrashInfo* crashInfo, float deltaTime);
void free_crashinfo(CrashInfo* crashInfo);
void draw_score(SDL_Renderer* renderer, int score);
void draw_digit(SDL_Renderer* renderer, Vector2 position, int num);
void play_sound(Mix_Chunk* sound);
SoundManager* init_soundmanager(const char* explosion, const char* shoot,
                                const char* hit, const char* alien,
                                const char* ran);
void free_soundmanager(SoundManager* sounds);
void player_shoot(State* state, Uint32 time);
int* get_digits(int number, int* num_digits);
Alien* init_alien();
void update_angle(Alien* alien, Vector2 playerPosition);
void alien_shoot(State* state, Uint32 time);
void update_alien(State* state, Time* time);
void draw_alien(SDL_Renderer* renderer, Alien* alien);

int main() {

    Time* gameTime = init_time();
    if (!gameTime) {
        fprintf(stderr, "Failed to initialize game time!\n");
        return GAME_ERROR;
    }

    Window* window = init_window(SCREEN_WIDTH, SCREEN_HEIGHT, "asteroids");
    if (!window) {
        fprintf(stderr, "Failed to initialize window!\n");
        free(gameTime);
        return WINDOW_ERROR;
    }

    State* state = init_state();
    if (!state) {
        fprintf(stderr, "Failed to initialize game state!\n");
        close_window(window);
        free(gameTime);
        return GAME_ERROR;
    }

    // Initialize asteroids
    spawn_asteroids(state, INIT_NUM_ASTEROIDS, time(NULL));

    while (!window->quit) {
        update_time(gameTime);
        update(window, state, gameTime);
        render(window, state, gameTime->time);
        limit_fps(gameTime);
    }

    // Cleanup
    free_state(state);
    close_window(window);
    free(gameTime);

    return OK;
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
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
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

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        fprintf(stderr, "SDL_mixer could not be initialize!\n");
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

void update(Window* window, State* state, Time* gameTime) {
    SDL_Event event;

    // Update logic goes here
    float deltaTime = gameTime->deltaTime;
    handle_events(window, &event, state->player, deltaTime);
    update_player(state->player, deltaTime);
    update_shoot(state, gameTime);
    update_asteroids(state->asteroids, state->asteroidSize, deltaTime);
    update_projectiles(state->projectiles, state->projectileSize, deltaTime);

    if (state->level >= 2 && !state->alien->hit) {
        update_alien(state, gameTime);
    }
    update_angle(state->alien, state->player->position);

    delete_projectiles(state, gameTime->time);
    detect_Shoot(state);

    if (state->asteroidSize <= 0) {
        state->level++;
        spawn_asteroids(state, state->level * INIT_NUM_ASTEROIDS, time(NULL));
        state->alien->hit = 0;
        Mix_PlayChannel(-1, state->sounds->ran, 0);
        state->alien->position = create_vector(0, 100);
    }

    if (!state->player->crashed) {
        detect_crash(state, gameTime->time);
    }

    if (state->player->crashed) {
        update_crashinfo(state->crashInfo, deltaTime);
        if ((gameTime->time - state->player->crashTime) >= RESPAWN_TIME) {
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
    draw_projectiles(window->renderer, state->alienProjs, state->alienProjSize);
    draw_score(window->renderer, state->score);

    if (!state->alien->hit) {
        draw_alien(window->renderer, state->alien);
    }
    if (!state->player->crashed) {
        draw_player(window->renderer, state->player, time);
    }

    if (state->player->crashed) {
        draw_crashinfo(window->renderer, state->crashInfo);
    }
    SDL_RenderPresent(window->renderer);
}

void handle_events(Window* window, SDL_Event* event, Player* player,
                   float deltaTime) {
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
            float dX = cos(player->rotation) * -PLAYER_SPEED * deltaTime;
            float dY = sin(player->rotation) * -PLAYER_SPEED * deltaTime;
            Vector2 movement = create_vector(dX, dY);
            player->moving = 1;
            player->velocity = vector_sum(player->velocity, movement);
        } else {
            player->moving = 0; // Set the moving flag back to 0
        }

        // Rotation left
        if (state[SDL_SCANCODE_LEFT]) {
            player->rotation -= PLAYER_ROTATION_RATE * deltaTime;
        }

        // Rotation right
        if (state[SDL_SCANCODE_RIGHT]) {
            player->rotation += PLAYER_ROTATION_RATE * deltaTime;
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
    player->crashTime = 0;
    player->position = (Vector2){x, y};
    player->velocity = (Vector2){0, 0};
    player->rotation = VERTICLE;
    player->lastShot = 0;
    return player;
}

State* init_state(void) {
    State* state = (State*)malloc(sizeof(State));
    if (!state) {
        fprintf(stderr, "Failed to allocate game state!\n");
        return NULL;
    }

    state->score = 0;
    state->level = 1;
    state->player = init_ship(SCREEN_WIDTH / 2.0, SCREEN_HEIGHT / 2.0);
    if (!state->player) {
        fprintf(stderr, "Failed to initialize player!\n");
        free(state);
        return NULL;
    }

    state->alien = init_alien();
    if (!state->alien) {
        fprintf(stderr, "Failed to initialize alien!\n");
        free(state);
        return NULL;
    }

    state->asteroidSize = 0;
    state->asteroidCapacity = INIT_CAPACITY;
    state->asteroids = (Asteroid**)malloc(sizeof(Asteroid*) * INIT_CAPACITY);
    if (!state->asteroids) {
        fprintf(stderr, "Failed to allocate asteroids array!\n");
        free_player(state->player);
        free(state);
        return NULL;
    }

    state->crashInfo = init_crashinfo();
    if (!state->crashInfo) {
        fprintf(stderr, "Failed to initialize crash info!\n");
        free(state->asteroids);
        free_player(state->player);
        free(state);
        return NULL;
    }

    state->projectileSize = 0;
    state->projectileCapacity = INIT_CAPACITY;
    state->projectiles =
        (Projectile**)malloc(sizeof(Projectile*) * INIT_CAPACITY);
    if (!state->projectiles) {
        fprintf(stderr, "Failed to allocate projectiles array!\n");
        free_crashinfo(state->crashInfo);
        free(state->asteroids);
        free_player(state->player);
        free(state);
        return NULL;
    }

    state->alienProjSize = 0;
    state->alienProjCapacity = INIT_CAPACITY;
    state->alienProjs =
        (Projectile**)malloc(sizeof(Projectile*) * INIT_CAPACITY);
    if (!state->projectiles) {
        fprintf(stderr, "Failed to allocate projectiles array!\n");
        free_crashinfo(state->crashInfo);
        free(state->asteroids);
        free_player(state->player);
        free(state);
        return NULL;
    }

    state->sounds = init_soundmanager(EXPLOSION_PATH, SHOOT_PATH, HIT_PATH,
                                      ALIEN_PATH, RAN_PATH);
    if (!state->sounds) {
        fprintf(stderr, "Failed to initialize sound manager!\n");
        free(state->projectiles);
        free_crashinfo(state->crashInfo);
        free(state->asteroids);
        free_player(state->player);
        free(state);
        return NULL;
    }

    return state;
}

void free_player(Player* player) { free(player); }

void free_state(State* state) {
    free_player(state->player);
    free_crashinfo(state->crashInfo);
    free_soundmanager(state->sounds);
    for (int i = 0; i < state->asteroidSize; i++) {
        free(state->asteroids[i]);
    }
    free(state->asteroids);

    for (int i = 0; i < state->projectileSize; i++) {
        free(state->projectiles[i]);
    }
    free(state->projectiles);

    for (int i = 0; i < state->alienProjSize; i++) {
        free(state->alienProjs[i]);
    }
    free(state->alienProjs);
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
    player->velocity =
        vector_mul(player->velocity, (1.0f - PLAYER_DRAG * deltaTime));

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
    int idx = asteroid_size_idx(asteroid->size);
    int numPoints = ASTEROID_POINTS[idx];
    Vector2 points[numPoints];

    float angleStep = (2 * M_PI) / (float)numPoints; // get the step of each
    for (int i = 0; i < (int)numPoints; i++) {
        float radius = ASTEROID_SIZES[idx] * rand_float(MIN_RADIUS, MAX_RADIUS);
        float angle = angleStep * i;
        float x = cos(angle) * radius;
        float y = sin(angle) * radius;
        Vector2 vector = create_vector(x, y);
        points[i] = vector_sum(asteroid->position, vector);
    }
    draw_shape(renderer, points, (int)numPoints);
}

void spawn_asteroids(State* state, int num, Uint32 seed) {
    for (int i = 0; i < num; i++) {
        seed = rand();
        AsteroidSize size = LARGE;

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
        return 2;
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

void player_shoot(State* state, Uint32 time) {
    add_projectile(state, time);
    Mix_PlayChannel(-1, state->sounds->shoot, 0);
}

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

    i = 0;
    while (i < state->alienProjSize) {
        if ((time - state->alienProjs[i]->spawnTime) >= PROJ_TIME) {
            free(state->alienProjs[i]);
            for (int j = i; j < state->alienProjSize - 1; j++) {
                state->alienProjs[j] = state->alienProjs[j + 1];
            }
            state->alienProjSize--; // Reduce the count
        } else {
            i++; // Only move forward if no deletion occurs
        }
    }
}

void update_shoot(State* state, Time* time) {
    if (state->player->shoot &&
        (time->time - state->player->lastShot) > FIRE_RATE) {
        state->player->lastShot = time->time;
        player_shoot(state, time->time);

        float dX =
            cos(state->player->rotation) * PLAYER_SHOOT_FORCE * time->deltaTime;
        float dY =
            sin(state->player->rotation) * PLAYER_SHOOT_FORCE * time->deltaTime;
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
            on_crash(state->crashInfo, state->player, time);
            Mix_PlayChannel(-1, state->sounds->explosion, 0);
        }
    }

    for (int i = 0; i < state->alienProjSize; i++) {
        Projectile* proj = state->alienProjs[i];
        float radius = PLAYER_SIZE;
        float dX = proj->position.x - position.x;
        float dY = proj->position.y - position.y;
        if ((dX * dX + dY * dY) <= (radius * radius)) {
            state->player->crashed = 1;
            state->player->crashTime = time;
            on_crash(state->crashInfo, state->player, time);
            Mix_PlayChannel(-1, state->sounds->explosion, 0);
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
                Mix_PlayChannel(-1, state->sounds->hit, 0);
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

    if (!state->alien->hit) {
        for (int i = 0; i < state->projectileSize; i++) {
            Vector2 position = state->projectiles[i]->position;
            Alien* alien = state->alien;
            float radius = ALIEN_SIZE;
            float dX = position.x - alien->position.x;
            float dY = position.y - alien->position.y;
            if ((dX * dX + dY * dY) <= (radius * radius)) {
                Mix_PlayChannel(-1, state->sounds->ran, 0);
                alien->hit = 1;
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
    int idx = asteroid_size_idx(size);
    state->score += (int)SCORES[idx];
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

void on_crash(CrashInfo* crashInfo, Player* player, Uint32 time) {
    srand(time);
    for (int i = 0; i < NUM_PARTICLES; i++) {
        rand();
        float speed = rand_float(MIN_PARTICLE_SPEED, MAX_PARTICLE_SPEED);
        float angle = rand_float(0, (2.0f * M_PI)); // Any angle within circle
        float dX = cos(angle) * speed;
        float dY = sin(angle) * speed;
        Vector2 velocity = create_vector(dX, dY);
        Vector2 drift = vector_mul(player->velocity, DRIFT_FRACTION);
        crashInfo->particles[i]->position = player->position;
        crashInfo->particles[i]->velocity = vector_sum(velocity, drift);
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
        Vector2 drift = vector_mul(player->velocity, DRIFT_FRACTION);
        crashInfo->lines[i]->position = player->position;
        crashInfo->lines[i]->velocity = vector_sum(velocity, drift);
        crashInfo->lines[i]->angle = angle;
    }
    player->velocity = create_vector(0, 0);
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

int* get_digits(int number, int* num_digits) {
    if (!num_digits) {
        return NULL;
    }

    // Special case for 0
    if (number == 0) {
        *num_digits = 1;
        int* digits = (int*)malloc(sizeof(int));
        if (!digits) {
            *num_digits = 0;
            return NULL;
        }
        digits[0] = 0;
        return digits;
    }

    int temp = number;
    *num_digits = 0;
    while (temp != 0) {
        (*num_digits)++;
        temp /= 10;
    }

    int* digits = (int*)malloc(*num_digits * sizeof(int));
    if (!digits) {
        *num_digits = 0;
        return NULL;
    }

    for (int i = *num_digits - 1; i >= 0; i--) {
        digits[i] = number % 10;
        number /= 10;
    }

    return digits;
}

void draw_digit(SDL_Renderer* renderer, Vector2 position, int num) {
    for (int i = 1; i < DIGIT_COUNTS[num]; i++) {
        Vector2 a = DIGIT_POINTS[num][i - 1];
        Vector2 b = DIGIT_POINTS[num][i];
        Vector2 newA = vector_sum(position, a);
        Vector2 newB = vector_sum(position, b);
        draw_line(renderer, newA, newB);
    }
}

void draw_score(SDL_Renderer* renderer, int score) {
    int numDigits;
    int* digits = get_digits(score, &numDigits);

    if (!digits || numDigits <= 0) {
        return;
    }

    float x = SCREEN_WIDTH - DIGIT_WIDTH * numDigits;
    float y = DIGIT_HEIGHT;
    for (int i = 0; i < numDigits; i++) {
        Vector2 position = create_vector(x + DIGIT_WIDTH * i, y);
        draw_digit(renderer, position, digits[i]);
    }

    free(digits);
}

void play_sound(Mix_Chunk* sound) {
    if (sound) {
        Mix_PlayChannel(-1, sound, 0);
    }
}

SoundManager* init_soundmanager(const char* explosion, const char* shoot,
                                const char* hit, const char* alien,
                                const char* ran) {
    if (!explosion || !shoot || !hit || !alien || !ran) {
        fprintf(stderr, "Invalid sound file paths!\n");
        return NULL;
    }

    SoundManager* sounds = (SoundManager*)malloc(sizeof(SoundManager));
    if (!sounds) {
        fprintf(stderr, "Failed to allocate sound manager!\n");
        return NULL;
    }

    sounds->explosion = Mix_LoadWAV(explosion);
    sounds->shoot = Mix_LoadWAV(shoot);
    sounds->hit = Mix_LoadWAV(hit);
    sounds->alien = Mix_LoadWAV(alien);
    sounds->ran = Mix_LoadWAV(ran);

    if (!sounds->explosion || !sounds->shoot || !sounds->hit ||
        !sounds->alien || !sounds->ran) {
        fprintf(stderr, "Failed to load sound effects!\n");
        free_soundmanager(sounds);
        return NULL;
    }

    // Set the volume to quarter of maximum
    Mix_Volume(-1, MIX_MAX_VOLUME / 4);

    return sounds;
}

void free_soundmanager(SoundManager* sounds) {
    if (!sounds) {
        return;
    }

    if (sounds->explosion) {
        Mix_FreeChunk(sounds->explosion);
    }

    if (sounds->shoot) {
        Mix_FreeChunk(sounds->shoot);
    }

    if (sounds->hit) {
        Mix_FreeChunk(sounds->hit);
    }

    if (sounds->alien) {
        Mix_FreeChunk(sounds->alien);
    }

    if (sounds->ran) {
        Mix_FreeChunk(sounds->ran);
    }

    free(sounds);
}

Alien* init_alien() {
    Alien* alien = (Alien*)malloc(sizeof(Alien));
    alien->hit = 0;
    alien->rotation = 0;
    alien->lastShot = 0;
    alien->position = create_vector(-20, 100);
    alien->velocity = create_vector(0, 0);
    return alien;
}

void update_angle(Alien* alien, Vector2 playerPosition) {
    float dX = playerPosition.x - alien->position.x;
    float dY = playerPosition.y - alien->position.y;

    // atan2 returns the angle in radians between the two points
    float angle = atan2f(-dY, -dX);
    alien->rotation = angle;
}

void alien_shoot(State* state, Uint32 time) {
    Alien* alien = state->alien;
    alien->lastShot = time;
    Projectile* proj = init_projectile(alien->position, alien->rotation, time);
    if (state->alienProjSize == state->alienProjCapacity - 1) {
        state->alienProjCapacity *= 2;
        state->alienProjs = (Projectile**)realloc(
            state->alienProjs, sizeof(Projectile*) * state->alienProjCapacity);
    }
    state->alienProjs[state->alienProjSize++] = proj;
}

void update_alien(State* state, Time* time) {
    Alien* alien = state->alien;
    Player* player = state->player;

    float dX = player->position.x - alien->position.x;
    float direction = (dX > 0) ? 1.0f : -1.0f;
    float deltaX = direction * ALIEN_SPEED * time->deltaTime;

    if (fabs(dX) > 1.0f) {
        alien->position =
            create_vector(alien->position.x + deltaX, alien->position.y);
    }

    if ((time->time - alien->lastShot) >= ALIEN_FIRE_RATE &&
        !state->player->crashed &&
        (time->time - state->player->crashTime) >=
            RESPAWN_TIME + PLAYER_SAFE_TIME) {
        alien_shoot(state, time->time);
    }
    update_projectiles(state->alienProjs, state->alienProjSize,
                       time->deltaTime);
}

void draw_alien(SDL_Renderer* renderer, Alien* alien) {
    Vector2 ship[NUM_ALIEN_POINTS];
    for (int i = 0; i < NUM_ALIEN_POINTS; i++) {
        Vector2 pos = vector_sum(alien->position, INIT_ALIEN_SHAPE[i]);
        ship[i] = pos;
    }

    draw_shape(renderer, ship, NUM_ALIEN_POINTS);
}
