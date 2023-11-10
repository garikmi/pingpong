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
        vector2 point;
        vector2 velocity;
        vector2 dimension;
    } ball;

    struct player {
        vector2 point;
        vector2 dimension;
        float vy;
        int score;
        int up;
        int down;
    } player_one, player_two;

    queue particles;
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

void handle_ball(game_state *state, float delta_time)
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

    state->ball.point.x += state->ball.velocity.x * delta_time;
    state->ball.point.y += state->ball.velocity.y * delta_time;
}

void handle_paddles(game_state *state, float delta_time)
{
    if(state->player_one.up)
        state->player_one.point.y -= state->player_one.vy * delta_time;
    if(state->player_one.down)
        state->player_one.point.y += state->player_one.vy * delta_time;
    if(state->player_two.up)
        state->player_two.point.y -= state->player_one.vy * delta_time;
    if(state->player_two.down)
        state->player_two.point.y += state->player_one.vy * delta_time;
}

void handle_particles(game_state *state, float delta_time)
{
    if(!qempty(&state->particles)) {
        struct node *tmp = state->particles.first;
        while(tmp) {
            tmp->data.color.a -= 300.0f * delta_time;
            if(tmp->data.color.a < 0.0f) {
                tmp = tmp->next;
                qget(&state->particles, NULL);
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
    SDL_Rect ball_rect = { state->ball.point.x, state->ball.point.y,
        state->ball.dimension.x, state->ball.dimension.y };
    SDL_SetRenderDrawColor(renderer, 198, 204, 215, 255);
    SDL_RenderFillRect(renderer, &ball_rect);
}

void render_paddles(game_state *state, SDL_Renderer *renderer)
{
    SDL_Rect player_one_rect = {
        state->player_one.point.x, state->player_one.point.y,
        state->player_one.dimension.x, state->player_one.dimension.y };
    SDL_Rect player_two_rect = {
        state->player_two.point.x, state->player_two.point.y,
        state->player_two.dimension.x, state->player_two.dimension.y };

    SDL_SetRenderDrawColor(renderer, 215, 78, 66, 255);
    SDL_RenderFillRect(renderer, &player_one_rect);

    SDL_SetRenderDrawColor(renderer, 16, 133, 255, 255);
    SDL_RenderFillRect(renderer, &player_two_rect);
}

void render_particles(game_state *state, SDL_Renderer *renderer)
{
    if(!qempty(&state->particles)) {
        SDL_Rect particle_rect;
        struct node *tmp = state->particles.first;
        while(tmp) {
            particle_rect.x = tmp->data.point.x;
            particle_rect.y = tmp->data.point.y;
            particle_rect.w = tmp->data.dimension.x;
            particle_rect.h = tmp->data.dimension.y;

            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer,
                (Uint8)tmp->data.color.r, (Uint8)tmp->data.color.g,
                (Uint8)tmp->data.color.b, (Uint8)tmp->data.color.a);
            SDL_RenderFillRect(renderer, &particle_rect);

            tmp = tmp->next;
        }
    }
}

void setup(game_state *state)
{
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
    float delta_time;

    time_to_wait = TARGET_FRAME_TIME - (SDL_GetTicks() - 
        state->last_frame_time);

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
