/*
 * main.c
 * platform-fun
 * September 26, 2011
 * Brandon Surmanski
 */

#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <stdio.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include "font.h"
#include "menu.h"
#include "game.h"

//GLOBAL VARS
const int SCREENX = 320;
const int SCREENY = 240;

SDL_Surface *main_screen;
SDL_Surface *scaled;
enum GameState game_state;
uint8_t *key_state;

void tick() {
    enum GameState next_state = game_state;
    switch (game_state) {
        case TITLE:
            next_state = update_main_menu();
            draw_main_menu();
            if (next_state != TITLE) menu_deinit();
            break;
        case LEVEL_SELECT:
            next_state = update_level_select();
            draw_level_select();
            if (next_state != LEVEL_SELECT) menu_deinit();
            break;
        case GAME:
            next_state = update_game();
            draw_game();
            if (next_state != GAME) game_deinit();
            break;
        // handle quit?
    }

    if (next_state != game_state) {
        switch (next_state) {
        case TITLE:
            menu_init();
            //TODO
            break;
        case LEVEL_SELECT:
            menu_init();
            break;
        case GAME:
            game_init();
            break;
        }
        game_state = next_state;
    }

    upscaleCopy(scaled, main_screen, 2);
    //SDL_BlitScaled(main_screen, 0, scaled, 0);
    SDL_Flip(scaled);
}

int main(int argc, char *argv[])
{
    int err;
    err = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    if(err){
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    }


    scaled = SDL_SetVideoMode(SCREENX * 2, SCREENY * 2, 32, SDL_DOUBLEBUF | SDL_SWSURFACE);

    main_screen = SDL_CreateRGBSurface(SDL_SWSURFACE,
            SCREENX,
            SCREENY,
            scaled->format->BitsPerPixel,
            scaled->format->Rmask,
            scaled->format->Gmask,
            scaled->format->Bmask,
            scaled->format->Amask
            );


    if (!main_screen){
        fprintf(stderr, "SDL failed screen init: %s\n", SDL_GetError());
    }

    err = Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 2048);
    if (err){
        fprintf(stderr, "SDL audio failed: %s\n", SDL_GetError());
    }
    init_font();
    key_state = SDL_GetKeyboardState(NULL);
    game_state = TITLE;
    srand(time(NULL));
    load_game_resources();

    menu_init();

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(tick, 0, 1);
#endif
    /*
    while(game_state != QUIT){
        menu_init();
        run_menu();
        menu_deinit();
        if(game_state != QUIT){
            game_init();
            run_game();
            game_deinit();
        }
    } */
    return 0;
}
