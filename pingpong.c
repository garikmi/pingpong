#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <SDL_ttf.h>

#include "types.h"
#include "settings.h"
#include "queue.h"

void exit_error(const char *error)
{
    fprintf(stderr, "%s\n", error);
    exit(1);
}

void initialize_window(game_state *state)
{
    int sdl;
    sdl = SDL_Init(SDL_INIT_EVERYTHING);
    if(sdl != 0)
        exit_error("Error initializing SDL.");

    state->window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN);
    if(!state->window)
        exit_error("Error creating SDL Window.");

    state->renderer = SDL_CreateRenderer(state->window, -1, 0);
    if(!state->renderer)
        exit_error("Error creating SDL Renderer.");
}

void initialize_font(game_state *state)
{
    int ttf;
    ttf = TTF_Init();
    if(ttf < 0)
        exit_error("Error opening font.");

    state->font = TTF_OpenFont("./iosevka-bold.ttf", 36);
    if(!state->font)
        exit_error("Error opening font.");
}

#if ENABLE_FPS
int get_fps(game_state *state)
{
    return (int)(1.0f / state->delta_time);
}
#endif

int rand_num(int m, int n)
{
    return m + rand() / (RAND_MAX / (n - m + 1) + 1);
}

void emit_particle(game_state *state, particle *fragment)
{
    if(fragment)
        qput(&state->particles, *fragment);
}

void collision_particles(game_state *state, vector3 color)
{
    int i;
    particle fragment;
    for(i = 0; i < 20; i++) {
        if(state->ball.point.x < WINDOW_WIDTH / 2)
            fragment.point.x = state->ball.point.x;
        else
            fragment.point.x = state->ball.point.x + state->ball.dimension.x;

        if(state->ball.point.y < WINDOW_HEIGHT / 2)
            fragment.point.y = state->ball.point.y;
        else
            fragment.point.y = state->ball.point.y + state->ball.dimension.y;

        fragment.velocity.x = 
            (float)rand_num((int)state->ball.velocity.x - 200,
                (int)state->ball.velocity.x + 200) / 4.0f;
        fragment.velocity.y = 
            (float)rand_num((int)state->ball.velocity.y - 200,
                (int)state->ball.velocity.y + 200) / 4.0f;

        fragment.dimension.x = (float)rand_num(3, 5);
        fragment.dimension.y = fragment.dimension.x;

        fragment.color.r = color.r;
        fragment.color.g = color.g;
        fragment.color.b = color.b;
        fragment.color.a = 255.0f;
        emit_particle(state, &fragment);
    }
}

void handle_ball(game_state *state)
{
    vector3 color;

    if(state->ball.point.x < state->player_one.point.x + 
        state->player_one.dimension.x && (state->ball.point.y < 
        state->player_one.point.y + state->player_one.dimension.y && 
        state->ball.point.y > state->player_one.point.y))
    {
        state->ball.point.x =
            state->player_one.point.x + state->player_one.dimension.x;
        state->ball.velocity.x *= -1;

        color.r = 215.0f;
        color.g = 78.0f;
        color.b = 66.0f;
        collision_particles(state, color);
    }

    if(state->ball.point.x + state->ball.dimension.x >
        state->player_two.point.x && (state->ball.point.y <
        state->player_two.point.y + state->player_two.dimension.y &&
        state->ball.point.y > state->player_two.point.y))
    {
        state->ball.point.x =
            state->player_two.point.x - state->ball.dimension.x;
        state->ball.velocity.x *= -1;

        color.r = 16.0f;
        color.g = 133.0f;
        color.b = 255.0f;
        collision_particles(state, color);
    }

    if(state->ball.point.x <= 0) {
        state->ball.point.x = WINDOW_WIDTH / 2;
        state->ball.point.y = 0;
        state->player_two.score += 1;
        if(state->player_two.score > 5) {
            state->player_two.score = 1;
            state->player_one.score = 0;
        }
    }
    if(state->ball.point.x >= WINDOW_WIDTH - state->ball.dimension.x) {
        state->ball.point.x = WINDOW_WIDTH / 2;
        state->ball.point.y = 0;
        state->player_one.score += 1;
        if(state->player_one.score > 5) {
            state->player_one.score = 1;
            state->player_two.score = 0;
        }
    }

    if(state->ball.point.y < 0) {
        state->ball.velocity.y *= -1;
        state->ball.point.y = 0;

        color.r = 198.0f;
        color.g = 204.0f;
        color.b = 215.0f;
        collision_particles(state, color);
    }
    if(state->ball.point.y > WINDOW_HEIGHT - state->ball.dimension.y) {
        state->ball.velocity.y *= -1;
        state->ball.point.y = WINDOW_HEIGHT - state->ball.dimension.y;

        color.r = 198.0f;
        color.g = 204.0f;
        color.b = 215.0f;
        collision_particles(state, color);
    }

    state->ball.point.x += state->ball.velocity.x * state->delta_time;
    state->ball.point.y += state->ball.velocity.y * state->delta_time;
}

void handle_paddles(game_state *state)
{
    if(state->player_one.up)
        state->player_one.point.y -= state->player_one.vy * state->delta_time;
    if(state->player_one.down)
        state->player_one.point.y += state->player_one.vy * state->delta_time;
    if(state->player_two.up)
        state->player_two.point.y -= state->player_one.vy * state->delta_time;
    if(state->player_two.down)
        state->player_two.point.y += state->player_one.vy * state->delta_time;
}

void handle_particles(game_state *state)
{
    if(!qempty(&state->particles)) {
        struct node *tmp = state->particles.first;
        while(tmp) {
            tmp->data.color.a -= 300.0f * state->delta_time;
            if(tmp->data.color.a < 0.0f) {
                tmp = tmp->next;
                qget(&state->particles, NULL);
                continue;
            }
            tmp->data.point.x += tmp->data.velocity.x * state->delta_time;
            tmp->data.point.y += tmp->data.velocity.y * state->delta_time;

            tmp = tmp->next;
        }
    }
}

void render_text_leading(game_state *state, const char *text, int fsize,
    vector2 pos, vector4 color)
{    
    /* Render */
    TTF_SetFontSize(state->font, fsize);
    SDL_Surface *text_surface;
    SDL_Color surface_color = { 
        (int)color.r, (int)color.g, (int)color.b, (int)color.a };
    text_surface = TTF_RenderText_Solid(state->font, text, surface_color);
    if (!text_surface)
        exit_error("Error initializing Text.");

    SDL_Texture *text_texture;
    text_texture = SDL_CreateTextureFromSurface(state->renderer,
        text_surface);

    SDL_Rect dest = { (int)pos.x, (int)pos.y, text_surface->w,
        text_surface->h };
    SDL_RenderCopy(state->renderer, text_texture, NULL, &dest);

    /* free */
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);

}

void render_text_centered(game_state *state, const char *text, int fsize,
    vector2 pos, vector4 color)
{    
    /* Render */
    TTF_SetFontSize(state->font, fsize);
    SDL_Surface *text_surface;
    SDL_Color surface_color = { 
        (int)color.r, (int)color.g, (int)color.b, (int)color.a };
    text_surface = TTF_RenderText_Solid(state->font, text, surface_color);
    if (!text_surface)
        exit_error("Error initializing Text.");

    SDL_Texture *text_texture;
    text_texture = SDL_CreateTextureFromSurface(state->renderer,
        text_surface);

    SDL_Rect dest = { (int)(pos.x - (float)text_surface->w / 2), (int)pos.y,
        text_surface->w, text_surface->h };
    SDL_RenderCopy(state->renderer, text_texture, NULL, &dest);

    /* free */
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text_surface);

}

void render_score(game_state *state)
{
    /* Build score string */
    char *score;
    score = malloc(10*sizeof(char));
    sprintf(score, "%d:%d",
            state->player_one.score, state->player_two.score);


    /* Render */
    vector2 pos = { (float)(WINDOW_WIDTH / 2), 10.0f };
    vector4 color = { 198.0f, 204.0f, 215.0f, 255.0f };
    render_text_centered(state, score, 30, pos, color);

    /* free */
    free(score);
}

#if ENABLE_FPS
void render_fps(game_state *state)
{
    /* Build score string */
    char *fps;
    fps = malloc(10*sizeof(char));
    sprintf(fps, "%d", get_fps(state));

    /* Render */
    vector2 pos = { 10.0f, 10.0f };
    vector4 color = { 198.0f, 204.0f, 215.0f, 255.0f };
    render_text_leading(state, fps, 16, pos, color);

    /* free */
    free(fps);
}
#endif


void render_ball(game_state *state)
{
    SDL_Rect ball_rect = { state->ball.point.x, state->ball.point.y,
        state->ball.dimension.x, state->ball.dimension.y };
    SDL_SetRenderDrawColor(state->renderer, 198, 204, 215, 255);
    SDL_RenderFillRect(state->renderer, &ball_rect);
}

void render_paddles(game_state *state)
{
    SDL_Rect player_one_rect = {
        state->player_one.point.x, state->player_one.point.y,
        state->player_one.dimension.x, state->player_one.dimension.y };
    SDL_Rect player_two_rect = {
        state->player_two.point.x, state->player_two.point.y,
        state->player_two.dimension.x, state->player_two.dimension.y };

    SDL_SetRenderDrawColor(state->renderer, 215, 78, 66, 255);
    SDL_RenderFillRect(state->renderer, &player_one_rect);

    SDL_SetRenderDrawColor(state->renderer, 16, 133, 255, 255);
    SDL_RenderFillRect(state->renderer, &player_two_rect);
}

void render_particles(game_state *state)
{
    if(!qempty(&state->particles)) {
        SDL_Rect particle_rect;
        struct node *tmp = state->particles.first;
        while(tmp) {
            particle_rect.x = tmp->data.point.x;
            particle_rect.y = tmp->data.point.y;
            particle_rect.w = tmp->data.dimension.x;
            particle_rect.h = tmp->data.dimension.y;

            SDL_SetRenderDrawBlendMode(state->renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(state->renderer,
                (Uint8)tmp->data.color.r, (Uint8)tmp->data.color.g,
                (Uint8)tmp->data.color.b, (Uint8)tmp->data.color.a);
            SDL_RenderFillRect(state->renderer, &particle_rect);

            tmp = tmp->next;
        }
    }
}

void setup(game_state *state)
{
    srand(time(NULL));

    initialize_window(state);
    initialize_font(state);

    state->is_running = TRUE;
    state->last_frame_time = 0;

    qinit(&state->particles);

    state->ball.point.x = 40;
    state->ball.point.y = 40;
    state->ball.dimension.x = 15;
    state->ball.dimension.y = 15;
    state->ball.velocity.x = 500;
    state->ball.velocity.y = 600;

    state->player_one.point.x = 10;
    state->player_one.point.y = 10;
    state->player_one.dimension.x = 10;
    state->player_one.dimension.y = 100;
    state->player_one.vy = 600;
    state->player_one.score = 0;
    state->player_one.up = FALSE;
    state->player_one.down = FALSE;

    state->player_two.point.x = WINDOW_WIDTH - 20;
    state->player_two.point.y = 10;
    state->player_two.dimension.x = 10;
    state->player_two.dimension.y = 100;
    state->player_two.vy = 600;
    state->player_two.score = 0;
    state->player_two.up = FALSE;
    state->player_two.down = FALSE;
}

void process_input(game_state *state)
{
    SDL_Event event;
    const Uint8 *currentKeyStates;
    SDL_PollEvent(&event);
    currentKeyStates = SDL_GetKeyboardState(NULL);

    if(currentKeyStates[SDL_SCANCODE_W]) {
        state->player_one.up = TRUE;
        state->player_one.down = FALSE;
    } else if(currentKeyStates[SDL_SCANCODE_S]) {
        state->player_one.up = FALSE;
        state->player_one.down = TRUE;
    } else {
        state->player_one.up = FALSE;
        state->player_one.down = FALSE;
    }

    if(currentKeyStates[SDL_SCANCODE_UP]) {
        state->player_two.up = TRUE;
        state->player_two.down = FALSE;
    } else if(currentKeyStates[SDL_SCANCODE_DOWN]) {
        state->player_two.up = FALSE;
        state->player_two.down = TRUE;
    } else {
        state->player_two.up = FALSE;
        state->player_two.down = FALSE;
    }

    switch(event.type) {
        case SDL_QUIT:
            state->is_running = FALSE;
            break;
        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_ESCAPE ||
                event.key.keysym.sym == SDLK_q)
            {
                state->is_running = FALSE;
            }
            break;
    }
}

void update(game_state *state)
{
    int time_to_wait;

    time_to_wait = TARGET_FRAME_TIME - (SDL_GetTicks() - 
        state->last_frame_time);

    if(time_to_wait > 0 && time_to_wait <= TARGET_FRAME_TIME)
        SDL_Delay(time_to_wait);

    state->delta_time = (SDL_GetTicks() - state->last_frame_time) / 1000.0f;

    state->last_frame_time = SDL_GetTicks();

    handle_ball(state);
    handle_paddles(state);
    handle_particles(state);
}

void render(game_state *state)
{
    SDL_SetRenderDrawColor(state->renderer, 24, 26, 31, 255);
    SDL_RenderClear(state->renderer);

    render_paddles(state);
    render_ball(state);
    render_particles(state);

    render_score(state);
#if ENABLE_FPS
    render_fps(state);
#endif

    SDL_RenderPresent(state->renderer);
}

void destroy_window(game_state *state)
{
    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);
    SDL_Quit();
}

int main()
{
    game_state state;

    setup(&state);
    while(state.is_running) {
        process_input(&state);
        update(&state);
        render(&state);
    }

    destroy_window(&state);

    return 0;
}
