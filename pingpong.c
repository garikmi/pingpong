#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL_ttf.h>

#define TRUE 1
#define FALSE 0

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define FPS 120
#define TARGET_FRAME_TIME (1000 / FPS)

typedef struct {
    float x;
    float y;
} xy_point;

typedef struct {
    unsigned int r;
    unsigned int g;
    unsigned int b;
} rgb;

typedef struct {
    float w;
    float h;
} rect;

typedef struct {
    xy_point point;
    xy_point velocity;
    rect dimension;
    rgb color;
    int alpha;
} particle;

struct node {
    struct node *next;
    particle data;
};

typedef struct {
    struct node *first;
    struct node *last;
} queue;

void qinit(queue *item)
{
    item->first = NULL;
    item->last = NULL;
}

void qput(queue *item, particle data)
{
    if(!item->first) {
        item->first = malloc(sizeof(struct node));
        item->last = item->first;
    } else {
        item->last->next = malloc(sizeof(struct node));
        item->last = item->last->next;
    }
    item->last->data = data;
    item->last->next = NULL;
}

void qget(queue *item, particle *data)
{
    if(data)
        *data = item->first->data;

    struct node *tmp = item->first;
    item->first = item->first->next;
    if(!item->first)
        item->last = NULL;
    free(tmp);
}

int qempty(queue *item)
{
    return !item->first;
}

typedef struct {
    int is_running;
    int last_frame_time;

    struct ball {
        float x;
        float y;
        float width;
        float height;
        float acceleration_x;
        float acceleration_y;
    } ball;

    struct player {
        float x;
        float y;
        float width;
        float height;
        float acceleration_y;
        int score;
        int up;
        int down;
    } player_one, player_two;

    queue particle_queue;
} game_state;

void exit_error(const char *error)
{
    fprintf(stderr, "%s\n", error);
    exit(1);
}

void initialize_window(SDL_Window **window, SDL_Renderer **renderer)
{
    int sdl;
    sdl = SDL_Init(SDL_INIT_EVERYTHING);
    if(sdl != 0)
        exit_error("Error initializing SDL.");

    *window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN);
    if(!window)
        exit_error("Error creating SDL Window.");

    *renderer = SDL_CreateRenderer(*window, -1, 0);
    if(!renderer)
        exit_error("Error creating SDL Renderer.");
}

void initialized_font(TTF_Font **font)
{
    int ttf;
    ttf = TTF_Init();
    if(ttf < 0)
        exit_error("Error opening font.");

    *font = TTF_OpenFont("./iosevka-bold.ttf", 36);
    if(!font)
        exit_error("Error opening font.");
}

void emit_particle(game_state *state, xy_point point, xy_point velocity, 
    rect dimension, rgb color, int alpha)
{
    particle p = {point, velocity, dimension, color, alpha};
    qput(&state->particle_queue, p);
}

void handle_ball(game_state *state, float delta_time)
{
    if(state->ball.x < state->player_one.x + state->player_one.width &&
        (state->ball.y < state->player_one.y + state->player_one.height &&
        state->ball.y > state->player_one.y))
    {
        state->ball.x = state->player_one.x + state->player_one.width;
        state->ball.acceleration_x *= -1;

        int i;
        for(i = 0; i < 50; i++) {
            xy_point point = {state->ball.x + state->ball.width, state->ball.y};
            xy_point velocity = {rand() % 81 + 80, rand() % 161 - 80};
            rgb color = {215, 78, 66};
            float width = (float)(rand() % 4 + 1);
            rect dimension = {width, width};
            emit_particle(state, point, velocity, dimension, color, 255);
        }
    }

    if(state->ball.x + state->ball.width > state->player_two.x &&
        (state->ball.y < state->player_two.y + state->player_two.height &&
        state->ball.y > state->player_two.y))
    {
        state->ball.x = state->player_two.x - state->ball.width;
        state->ball.acceleration_x *= -1;

        int i;
        for(i = 0; i < 50; i++) {
            xy_point point = {state->ball.x, state->ball.y};
            xy_point velocity = {-(rand() % 81 + 80), rand() % 161 - 80};
            rgb color = {16, 133, 255};
            float width = (float)(rand() % 4 + 1);
            rect dimension = {width, width};
            emit_particle(state, point, velocity, dimension, color, 255);
        }
    }

    if(state->ball.x <= 0) {
        state->ball.x = WINDOW_WIDTH / 2;
        state->ball.y = 0;
        state->player_two.score += 1;
        if(state->player_two.score > 5) {
            state->player_two.score = 1;
            state->player_one.score = 0;
        }
    }
    if(state->ball.x >= WINDOW_WIDTH - state->ball.width) {
        state->ball.x = WINDOW_WIDTH / 2;
        state->ball.y = 0;
        state->player_one.score += 1;
        if(state->player_one.score > 5) {
            state->player_one.score = 1;
            state->player_two.score = 0;
        }
    }

    if(state->ball.y < 0) {
        state->ball.acceleration_y *= -1;
        state->ball.y = 0;
    }
    if(state->ball.y > WINDOW_HEIGHT - state->ball.height) {
        state->ball.acceleration_y *= -1;
        state->ball.y = WINDOW_HEIGHT - state->ball.height;
    }

    state->ball.x += state->ball.acceleration_x * delta_time;
    state->ball.y += state->ball.acceleration_y * delta_time;
}

void handle_paddles(game_state *state, float delta_time)
{
    if(state->player_one.up)
        state->player_one.y -= state->player_one.acceleration_y * delta_time;
    if(state->player_one.down)
        state->player_one.y += state->player_one.acceleration_y * delta_time;
    if(state->player_two.up)
        state->player_two.y -= state->player_one.acceleration_y * delta_time;
    if(state->player_two.down)
        state->player_two.y += state->player_one.acceleration_y * delta_time;
}

void handle_particles(game_state *state, float delta_time)
{
    if(!qempty(&state->particle_queue)) {
        struct node *tmp = state->particle_queue.first;
        while(tmp) {
            tmp->data.alpha -= 2; /* TODO: Lock to delta time. */
            if(tmp->data.alpha < 0.0f) {
                tmp = tmp->next;
                qget(&state->particle_queue, NULL);
                continue;
            }
            tmp->data.point.x += tmp->data.velocity.x * delta_time;
            tmp->data.point.y += tmp->data.velocity.y * delta_time;

            tmp = tmp->next;
        }
    }
}

void render_score(game_state *state, SDL_Renderer *renderer, TTF_Font *font)
{
    /* Build score string */
    char *score;
    score = malloc(3*sizeof(char));
    score[0] = (char)(state->player_one.score + 48); /* FIX */
    score[1] = ':';
    score[2] = (char)(state->player_two.score + 48);

    /* Render */
    SDL_Surface *text;
    SDL_Color color = { 198, 204, 215, 255 };
    text = TTF_RenderText_Solid(font, score, color);
    if (!text)
        exit_error("Error initializing Text.");

    SDL_Texture *text_texture;
    text_texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_Rect dest = { WINDOW_WIDTH/2-20, 10, text->w, text->h };
    SDL_RenderCopy(renderer, text_texture, NULL, &dest);

    /* free */
    SDL_DestroyTexture(text_texture);
    SDL_FreeSurface(text);
    free(score);
}

void render_ball(game_state *state, SDL_Renderer *renderer)
{
    SDL_Rect ball_rect = { state->ball.x, state->ball.y, state->ball.width,
        state->ball.height };
    SDL_SetRenderDrawColor(renderer, 198, 204, 215, 255);
    SDL_RenderFillRect(renderer, &ball_rect);
}

void render_paddles(game_state *state, SDL_Renderer *renderer)
{
    SDL_Rect player_one_rect = { state->player_one.x, state->player_one.y,
        state->player_one.width, state->player_one.height };
    SDL_Rect player_two_rect = { state->player_two.x, state->player_two.y,
        state->player_two.width, state->player_two.height };

    SDL_SetRenderDrawColor(renderer, 215, 78, 66, 255);
    SDL_RenderFillRect(renderer, &player_one_rect);

    SDL_SetRenderDrawColor(renderer, 16, 133, 255, 255);
    SDL_RenderFillRect(renderer, &player_two_rect);
}

void render_particles(game_state *state, SDL_Renderer *renderer)
{
    if(!qempty(&state->particle_queue)) {
        SDL_Rect particle_rect;
        struct node *tmp = state->particle_queue.first;
        while(tmp) {
            particle_rect.x = tmp->data.point.x;
            particle_rect.y = tmp->data.point.y;
            particle_rect.w = tmp->data.dimension.w;
            particle_rect.h = tmp->data.dimension.h;

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer,
                tmp->data.color.r, tmp->data.color.g,
                tmp->data.color.b, tmp->data.alpha);
            SDL_RenderFillRect(renderer, &particle_rect);

            tmp = tmp->next;
        }
    }
}

void setup(game_state *state)
{
    state->is_running = TRUE;
    state->last_frame_time = 0;

    qinit(&state->particle_queue);

    state->ball.x = 40;
    state->ball.y = 40;
    state->ball.width = 15;
    state->ball.height = 15;
    state->ball.acceleration_x = 300;
    state->ball.acceleration_y = 400;

    state->player_one.x = 10;
    state->player_one.y = 10;
    state->player_one.width = 10;
    state->player_one.height = 100;
    state->player_one.acceleration_y = 500;
    state->player_one.score = 0;
    state->player_one.up = FALSE;
    state->player_one.down = FALSE;

    state->player_two.x = WINDOW_WIDTH - 20;
    state->player_two.y = 10;
    state->player_two.width = 10;
    state->player_two.height = 100;
    state->player_two.acceleration_y = 500;
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
    float delta_time;

    time_to_wait = TARGET_FRAME_TIME - (SDL_GetTicks() - state->last_frame_time);

    if(time_to_wait > 0 && time_to_wait <= TARGET_FRAME_TIME)
        SDL_Delay(time_to_wait);

    delta_time = (SDL_GetTicks() - state->last_frame_time) / 1000.0f;

    state->last_frame_time = SDL_GetTicks();

    handle_ball(state, delta_time);
    handle_paddles(state, delta_time);
    handle_particles(state, delta_time);
}

void render(game_state *state, SDL_Renderer *renderer, TTF_Font *font)
{
    SDL_SetRenderDrawColor(renderer, 24, 26, 31, 255);
    SDL_RenderClear(renderer);

    render_paddles(state, renderer);
    render_ball(state, renderer);
    render_score(state, renderer, font);
    render_particles(state, renderer);

    SDL_RenderPresent(renderer);
}

void destroy_window(SDL_Window *window, SDL_Renderer *renderer)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int main()
{
    srand(time(NULL));

    game_state state;
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    TTF_Font *font;

    initialize_window(&window, &renderer);
    initialized_font(&font);

    setup(&state);
    while(state.is_running) {
        process_input(&state);
        update(&state);
        render(&state, renderer, font);
    }

    destroy_window(window, renderer);

    return 0;
}
