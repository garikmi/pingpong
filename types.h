#ifndef TYPES_H_SENTRY
#define TYPES_H_SENTRY

#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>


#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef struct {
    float x;
    float y;
} vector2;

typedef struct {
    float r;
    float g;
    float b;
} vector3;

typedef struct {
    float r;
    float g;
    float b;
    float a;
} vector4;

typedef struct {
    float w;
    float h;
} rect;

typedef struct {
    vector2 point;
    vector2 velocity;
    vector2 dimension;
    vector4 color;
} particle;

struct node {
    struct node *next;
    particle data;
};

typedef struct {
    struct node *first;
    struct node *last;
} queue;

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    TTF_Font *font;

    int is_running;
    int last_frame_time;
    float delta_time;

    struct {
        vector2 point;
        vector2 velocity;
        vector2 dimension;
    } ball;

    struct {
        vector2 point;
        vector2 dimension;
        float vy;
        int score;
        int up;
        int down;
    } player_one, player_two;

    queue particles;
} game_state;

#endif
