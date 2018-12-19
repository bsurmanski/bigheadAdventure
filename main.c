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
#include <SDL2_image/SDL_image.h>
#include <SDL2_mixer/SDL_mixer.h>

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


int main(int argc, char *argv[])
{
    int err;
    err = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    if(err){
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
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

    err = Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 2048);
    if (err){
        fprintf(stderr, "SDL audio failed: %s\n", SDL_GetError());
    }
    init_font();
    key_state = SDL_GetKeyboardState(0);
    game_state = 1;
    srand(time(NULL));
    load_game_resources();

    while(game_state != QUIT){
        run_menu();
        if(game_state != QUIT){
            run_game();
        }
    }
    return 0;
}
