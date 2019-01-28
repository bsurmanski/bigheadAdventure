/**
 * menu.c
 * platform-fun
 * October 17, 2011
 * Brandon Surmanski
 */

#include <ctype.h>

#include <SDL2/SDL.h>
#ifdef __APPLE__
#include <SDL2_image/SDL_image.h>
#include <SDL2_mixer/SDL_mixer.h>
#elif __EMSCRIPTEN__
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#else
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#endif

#include "font.h"
#include "game.h"
#include "menu.h"

extern SDL_Renderer *renderer;
extern SDL_Texture *main_screen;
extern int game_state;
extern uint8_t *key_state;
extern int RUNNING;
extern int QUIT;

static int menu_state;
static int selection;
static int level_selection;
const int MAX_SELECTION = 1;
const int LEVEL_SELECT = 2;
Mix_Chunk *mix_blip = NULL;
SDL_Texture *selector = NULL;
static int num_levels;
static char **levels;

static void draw_main_menu(void);
static void update_main_menu(void);
static void draw_level_select(void);
static void update_level_select(void);
void upscaleCopy(SDL_Texture *src, SDL_Texture *dst, int scale);

void init_menu(void) {
    menu_state = RUNNING;
    selection = 0;
    if(!mix_blip) mix_blip = get_sound("blip.wav");
    if(!selector) selector = get_texture("pointer.png");
    levels = list_game_levels(&num_levels);
}

int tick_menu(void)
{
    if(menu_state == RUNNING){
        update_main_menu();
        draw_main_menu();
    } else {
        update_level_select();
        draw_level_select();
    }
    SDL_RenderCopy(renderer, main_screen, NULL, NULL);
    SDL_RenderPresent(renderer);
    return menu_state == QUIT;
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
    SDL_SetRenderTarget(renderer, main_screen);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    draw_text("BIGHEAD'S ADVENTURE", 50, 50);
    draw_text("START", 130, 150);
    draw_text("QUIT", 135, 175);
    int w, h;
    SDL_QueryTexture(selector, NULL, NULL, &w, &h);
    SDL_Rect select_dest = {100, 150 + 25 * selection, w, h};
    SDL_RenderCopy(renderer, selector, NULL, &select_dest);
    SDL_SetRenderTarget(renderer, NULL);
}

static void draw_level_select(void)
{
    SDL_SetRenderTarget(renderer, main_screen);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    char buf[20];
    memset(buf, 0, 20);
    int i;
    for(i = 0; i < strlen(levels[level_selection]) && i < 20; i++){
        buf[i] = toupper(levels[level_selection][i]);
    }
    draw_text(buf, 150 - i * 5, 120);
    SDL_SetRenderTarget(renderer, NULL);
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
                if(event.key.keysym.scancode == SDL_SCANCODE_DOWN){
                    selection++;
                    selection %= (MAX_SELECTION+1);
                    play_sound(mix_blip);
                } else if (event.key.keysym.scancode == SDL_SCANCODE_UP){
                    selection--;
                    play_sound(mix_blip);
                    if(selection < 0)
                        selection = MAX_SELECTION;
                } else if (event.key.keysym.scancode == SDL_SCANCODE_RETURN){
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
                    play_sound(mix_blip);
                } else if (event.key.keysym.sym == SDLK_UP){
                    level_selection--;
                    play_sound(mix_blip);
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


