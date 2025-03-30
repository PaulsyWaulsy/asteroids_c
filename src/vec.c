#include "vec.h"
#include <math.h>

inline Vector2 create_vector(float x, float y) {
    return (Vector2){x, y};
}

inline Vector2 vector_sum(const Vector2 a, const Vector2 b) {
    return create_vector(a.x + b.x, a.y + b.y);
}

inline Vector2 vector_mul(const Vector2 vec, float factor) {
    return create_vector(vec.x * factor, vec.y * factor);
}

inline Vector2 vector_rot(const Vector2 vec, float angle) {
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);

    float rotatedX = vec.x * cosTheta - vec.y * sinTheta;
    float rotatedY = vec.y * sinTheta + vec.y * cosTheta;

    return create_vector(rotatedX, rotatedY);
}

inline Vector2 vector_aro(const Vector2 vec, const Vector2 center, float angle) {
    float cosTheta = cos(angle);
    float sinTheta = sin(angle);

    float translatedX = vec.x - center.x;
    float translatedY = vec.y - center.y;

    float rotatedX = translatedX * cosTheta - translatedY * sinTheta;
    float rotatedY = translatedX * sinTheta + translatedY * cosTheta;

    return create_vector(rotatedX + center.x, rotatedY + center.y);
}

void draw_line(SDL_Renderer* renderer, Vector2 start, Vector2 end) {
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderDrawLine(renderer, start.x, start.y, end.x, end.y);
}

void draw_shape(SDL_Renderer* renderer, const Vector2 points[], int size) {
    for (int i = 0; i < size - 1; i++) {
        draw_line(renderer, points[i], points[i + 1]);
    }
    draw_line(renderer, points[0], points[size - 1]);
}

void draw_thick_point(SDL_Renderer* renderer, int x, int y, int thickness) {
    SDL_Rect rect = { x - thickness / 2, y - thickness / 2, thickness, thickness };
    SDL_RenderFillRect(renderer, &rect);
}
