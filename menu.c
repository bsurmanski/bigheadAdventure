/**
 * menu.c
 * platform-fun
 * October 17, 2011
 * Brandon Surmanski
 */

#include <ctype.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_mixer.h>

#include "font.h"
#include "game.h"
#include "menu.h"

extern SDL_Surface *main_screen;
extern int game_state;
extern uint8_t *key_state;
extern int RUNNING;
extern int QUIT;

static int menu_state;
static int selection;
static int level_selection;
const int MAX_SELECTION = 1;
const int LEVEL_SELECT = 2;
Mix_Chunk *mix_blip;
SDL_Surface *selector;
static int num_levels;
static char **levels;

static void draw_main_menu(void);
static void update_main_menu(void);
static void draw_level_select(void);
static void update_level_select(void);


void run_menu(void)
{
    static int ms_passed;
    menu_state = RUNNING;
    selection = 0;
    mix_blip = Mix_LoadWAV("res/blip.wav");
    selector = IMG_Load("res/pointer.png");
    levels = list_game_levels(&num_levels);
    while(game_state == RUNNING && menu_state == RUNNING || menu_state == LEVEL_SELECT){
        //SDL_WarpMouse(10,10);
        ms_passed = SDL_GetTicks();
        if(menu_state == RUNNING){
            update_main_menu();
            draw_main_menu();
        } else {
            update_level_select();
            draw_level_select();
        }
        SDL_UpdateRect(main_screen,0,0,main_screen->w, main_screen->h);
        ms_passed = SDL_GetTicks() - ms_passed;
        SDL_Delay(16 - ms_passed);
    } 
    Mix_FreeChunk(mix_blip);
    SDL_FreeSurface(selector);
}


int get_menu_selection(void)
{
    return selection;
}

int get_level_selection(void)
{
    return level_selection;
}

static void draw_main_menu(void)
{
    SDL_Rect clear = {0, 0, main_screen->w, main_screen->h};
    SDL_FillRect(main_screen, &clear, SDL_MapRGB(main_screen->format, 0, 0, 0));
    draw_text(main_screen, "BIGHEAD'S ADVENTURE", 50, 50); 
    draw_text(main_screen, "START", 130, 150);
    draw_text(main_screen, "QUIT", 135, 175);
    SDL_Rect select_src = {0, 0, selector->w, selector->h};
    SDL_Rect select_dest = {100, 150 + 25 * selection, selector->w, selector->h}; 
    SDL_BlitSurface(selector, &select_src, main_screen, &select_dest); 
}

static void draw_level_select(void)
{
    SDL_Rect clear = {0, 0, main_screen->w, main_screen->h};
    SDL_FillRect(main_screen, &clear, SDL_MapRGB(main_screen->format, 0, 0, 0));
    char buf[20];
    memset(buf, 0, 20);
    int i;
    for(i = 0; i < strlen(levels[level_selection]) && i < 20; i++){
        buf[i] = toupper(levels[level_selection][i]); 
    }
    draw_text(main_screen, buf, 150 - i * 5, 120);
}

static void update_main_menu(void)
{
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_QUIT:
                game_state = QUIT;
                menu_state = QUIT;
            case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_DOWN){
                    selection++;
                    selection %= (MAX_SELECTION+1);
                    Mix_PlayChannel(-1, mix_blip, 0);
                } else if (event.key.keysym.sym == SDLK_UP){
                    selection--;
                    Mix_PlayChannel(-1, mix_blip, 0);
                    if(selection < 0)
                        selection = MAX_SELECTION;
                } else if (event.key.keysym.sym == SDLK_RETURN){
                    if(menu_state == RUNNING){
                            if(selection == 1)
                                game_state = QUIT;
                            else{
                                menu_state = LEVEL_SELECT;
                                selection = 0;
                            }
                    }else{
                            menu_state = QUIT;
                    }
                }
                break;
            default:
                break;
        }
    }
}

static void update_level_select(void)
{
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch(event.type){
            case SDL_QUIT:
                game_state = QUIT;
                menu_state = QUIT;
            case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_DOWN){
                    level_selection++;
                    level_selection %= (num_levels);
                    Mix_PlayChannel(-1, mix_blip, 0);
                } else if (event.key.keysym.sym == SDLK_UP){
                    level_selection--;
                    Mix_PlayChannel(-1, mix_blip, 0);
                    if(level_selection < 0)
                        level_selection = num_levels - 1;
                } else if (event.key.keysym.sym == SDLK_RETURN){
                    menu_state = QUIT;
                }
                break;
            default:
                break;
        }
    }
}


