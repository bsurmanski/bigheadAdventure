/*
 * main.c
 * platform-fun
 * September 26, 2011
 * Brandon Surmanski
 */

#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>

#include "font.h"
#include "menu.h"
#include "game.h"

//GLOBAL VARS
const int RUNNING = 1;
const int QUIT = -1;

SDL_Surface *main_screen;
int game_state;
uint8_t *key_state;


int main(int argc, char *argv[])
{
    int err;
    err = SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
    if(err){
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
    }

    main_screen = SDL_SetVideoMode(320, 240, 8, SDL_HWSURFACE );
    if (!main_screen){
        fprintf(stderr, "SDL failed screen init: %s\n", SDL_GetError());
    }

    err = Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 2048);
    if (err){
        fprintf(stderr, "SDL audio failed: %s\n", SDL_GetError());
    }
    init_font();
    key_state = SDL_GetKeyState(NULL);
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
