/*
 * main.c
 * platform-fun
 * September 26, 2011
 * Brandon Surmanski
 */

#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <SDL2/SDL.h>
#ifdef __APPLE__
#include <SDL2_image/SDL_image.h>
#include <SDL2_mixer/SDL_mixer.h>
#elif __EMSCRIPTEN__
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <emscripten.h>
#else
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#endif
#include "font.h"
#include "menu.h"
#include "game.h"

//GLOBAL VARS
const int RUNNING = 1;
const int QUIT = -1;

const int SCREENX = 320;
const int SCREENY = 240;

SDL_Renderer *renderer;
SDL_Texture *main_screen;
int game_state;
const uint8_t *key_state;
const char *res_pack = 0;

int current_month() {
    time_t timer;
    struct tm* tm_info;
    time(&timer);
    tm_info = localtime(&timer);
    return tm_info->tm_mon;
}

static int in_menu = 1;
void tick() {
    if(in_menu) {
        if(tick_menu()) {
            init_game();
            in_menu = 0;
        }
    } else {
        in_menu = tick_game();
    }

#ifdef __EMSCRIPTEN__
    if(game_state == QUIT) {
        emscripten_cancel_main_loop();
        printf("CANCELED\n");
    }
#endif
}

int main(int argc, char *argv[])
{
    int err;
    err = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    if(err){
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    }

    // december
    if (current_month() == 11) {
        printf("It's December, Christmas theme activated!\n");
        res_pack = "xmas";
    }

    SDL_Window *window = SDL_CreateWindow("Bighead's Adventure",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREENX * 2,
                                          SCREENY * 2, 0);

    renderer = SDL_CreateRenderer(window, -1, 0);

    SDL_RendererInfo info;
    SDL_GetRendererInfo(renderer, &info);
    if(!(info.flags & SDL_RENDERER_TARGETTEXTURE)) {
        printf("Cannot set a texture as a render target!\n");
    }

    main_screen = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888,
                                    SDL_TEXTUREACCESS_TARGET,
                                    SCREENX,
                                    SCREENY);

    if (!main_screen){
        fprintf(stderr, "SDL failed screen init: %s\n", SDL_GetError());
    }

#ifndef NO_AUDIO
    err = Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 2048);
    if (err){
        fprintf(stderr, "SDL audio failed: %s\n", SDL_GetError());
    }
#endif
    init_font();
    key_state = SDL_GetKeyboardState(0);
    game_state = 1;
    srand(time(NULL));
    load_game_resources();

    int ms_passed = 0;

    init_menu();
#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(tick, 60 /*fps*/, 1 /*blocking*/);
#else
    while(game_state != QUIT) {
        tick();
        ms_passed = SDL_GetTicks() - ms_passed;
        if(ms_passed < 16) SDL_Delay(16 - ms_passed);
    }
#endif
    return 0;
}
