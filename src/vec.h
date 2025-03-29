#ifndef  VEC_H

#include <SDL2/SDL.h>

// position in pixels 
typedef struct {
    float x;
    float y;
} Vector2;

Vector2 create_vector(float x, float y);
Vector2 vector_sum(const Vector2 a, const Vector2 b);
Vector2 vector_mul(const Vector2 vec, float factor);
Vector2 vector_rot(const Vector2 vec, float angle);
Vector2 vector_aro(const Vector2 vec, const Vector2 center, float angle);
void draw_line(SDL_Renderer* renderer, const Vector2 a, const Vector2 b);
void draw_shape(SDL_Renderer* renderer, const Vector2 points[], int size);
void draw_thick_point(SDL_Renderer* renderer, int x, int y, int thickness);


#endif
