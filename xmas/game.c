/**
 * game.c
 * platform-fun
 * September 26, 2011
 * Brandon Surmanski
 */

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <sys/types.h>
#include <dirent.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "list.h"
#include "game.h"
#include "font.h"
#include "map.h"
#include "player.h"

extern const int RUNNING;
extern const int QUIT;


extern SDL_Surface *main_screen;
extern int game_state;
extern uint8_t *key_state;

int ticks;
bool win = false;

//MAP
static SDL_Surface *map = 0;
SDL_Surface *overlay = 0;
SDL_Surface *map_buffer = 0;

static SDL_Surface *blk_brick;
static SDL_Surface *blk_grass;
static SDL_Surface *blk_coin;
static SDL_Surface *blk_spike;
static SDL_Surface *blk_water;
static SDL_Surface *blk_obsid;
static SDL_Surface *blk_cake;

extern SDL_Surface *font;

int map_draw_offsetx = 0;
int map_draw_offsety = 0;

static int get_background_color(){
    return SDL_MapRGB(map_buffer->format, 150, 150, 200);
}

static void init_map_buffer()
{
    if(map_buffer){
        SDL_FreeSurface(map_buffer);
    }

    map_buffer = SDL_CreateRGBSurface(SDL_SWSURFACE,
                map->w * 16,
                map->h * 16,
                16,//main_screen->format->BitsPerPixel,
                main_screen->format->Rmask,
                main_screen->format->Gmask,
                main_screen->format->Bmask,
                main_screen->format->Amask);

    SDL_Rect map_size = {0,0, map_buffer->w, map_buffer->h};
    SDL_FillRect(map_buffer,
                &map_size,
                get_background_color());
    SDL_Rect dest = {0,0,16,16};

    int pxl;
    int bpp = map->format->BytesPerPixel;
    int i, j;
    for(i = 0; i < map->h; i++){
        for(j = 0; j < map->w; j++){
            dest.x = j * 16;
            dest.y = i * 16;
            memcpy(&pxl,  map->pixels + i * map->pitch + j * bpp, bpp);

            switch (pxl){
                case BLK_BRCK:
                    SDL_BlitSurface(blk_brick, NULL, map_buffer, &dest);
                    break;
                case BLK_GRSS:
                    SDL_BlitSurface(blk_grass, NULL, map_buffer, &dest);
                    break;
                case BLK_COIN:
                    SDL_BlitSurface(blk_coin, NULL, map_buffer, &dest);
                    break;
                case BLK_SPKE:
                    SDL_BlitSurface(blk_spike, NULL, map_buffer, &dest);
                    break;
                case BLK_WATR:
                    SDL_BlitSurface(blk_water, NULL, map_buffer, &dest);
                    break;
                case BLK_OBSD:
                    SDL_BlitSurface(blk_obsid, NULL, map_buffer, &dest);
                    break;
                case BLK_CAKE:
                    SDL_BlitSurface(blk_cake, NULL, map_buffer, &dest);
                default:
                    break;
            }
        }
    }
}

char **list_game_levels(int *num_levels)
{
static int levels_size = 0;
static int max_level_size = 20;
static char **levels = 0;
    if(levels){
        int i;
        for (i = 0; i < levels_size; i++){
            free(levels[i]);
        }
        free(levels);
        levels_size = 0;
    }
    levels = malloc(sizeof(char*) * max_level_size);
    DIR *dp;
    struct dirent *ep;
    dp = opendir("res/maps/");
    while((ep = readdir(dp))){
        if(!(strchr(ep->d_name, '.') - ep->d_name)) //name starts with '.'
            continue;

        int len = strlen(ep->d_name);//strnlen(ep->d_name, 20);
        if(strstr(ep->d_name, ".png")) //remove .png postfix
            len -= 4;
        char *tmp = malloc(len + 1);
        memset(tmp, 0, len + 1);
        strncpy(tmp, ep->d_name, len);
        levels[levels_size++] = tmp;
        if(levels_size >= max_level_size){
            max_level_size *= 2;
            levels = realloc(levels, sizeof(char*) * max_level_size);
        }
    }
    closedir(dp);
    *num_levels = levels_size;
    return levels;
}

/**
 * will load the game level and set the player position
 */
static void load_game_level(char *level){
    char buf[64];
    strcpy(buf, "res/maps/");
    strcat(buf, level);
    strcat(buf, ".png");

    if(map){
        SDL_FreeSurface(map);
    }
    map = IMG_Load(buf);

    init_map_buffer();

    if (overlay){
        SDL_FreeSurface(overlay);
    }
    overlay = SDL_CreateRGBSurface(SDL_SRCALPHA,
                map->w * 16,
                map->h * 16,
                main_screen->format->BitsPerPixel,
                main_screen->format->Rmask,
                main_screen->format->Gmask,
                main_screen->format->Bmask,
                main_screen->format->Amask);

    SDL_Rect fill_rect = {0, 0, overlay->w, overlay->h};
    SDL_FillRect(overlay,
                &fill_rect,
                SDL_MapRGBA(overlay->format, 0,0,0,0));

    int x, y;
    find_map_block(0xFF000000 ,&x, &y);
    respawn_player(x * 16, y * 16);
    win = false;
}

/**
 * Loads game resources such as images, and prepares structures
 *
 */
void load_game_resources()
{
    blk_brick = IMG_Load("res/brick.png");
    blk_grass = IMG_Load("res/grass.png");
    blk_coin = IMG_Load("res/coin.png");
    blk_spike = IMG_Load("res/spikes.png");
    blk_water = IMG_Load("res/water.png");
    blk_obsid = IMG_Load("res/obsidian.png");
    blk_cake = IMG_Load("res/cake.png");

    load_player_resources();
}

SDL_Surface *get_horizontal_flipped(SDL_Surface *surface)
{
    SDL_Surface *flipped = SDL_CreateRGBSurface(SDL_SWSURFACE,
            surface->w,
            surface->h,
            surface->format->BitsPerPixel,
            surface->format->Rmask,
            surface->format->Gmask,
            surface->format->Bmask,
            surface->format->Amask
            );

    int bpp = surface->format->BytesPerPixel;
    int i, j;
    for(i = 0; i < surface->h; i++){
        for(j = 0; j < surface->w; j++){
            memcpy(flipped->pixels + ((i * bpp) + (j * surface->pitch)),
                    surface->pixels + (surface->w -1  - i) * bpp + j * surface->pitch,
                    bpp);
        }
    }
    return flipped;
}

/**
 * determine if the point x,y falls outside the buffer space
 */
int outside_buffer(SDL_Surface *surface, int x, int y)
{
    return (    x < 0
            ||  y < 0
            ||  x >= surface->w
            ||  y >= surface->h
            );
}

static void draw_player_data()
{
    //draw score
    char buffer[20];
    int score = get_player_score();
    sprintf(buffer, "SCORE:%d", score);
    draw_text(main_screen, buffer, 5, 5);
}

/**
 * draws the game to the screen, and calls all other relevant draw functions
 */
static void draw_game()
{
    //clear screen
    SDL_Rect fill_rect = {0, 0, main_screen->w, main_screen->h};

    SDL_Rect overlay_rect = {map_draw_offsetx,
                            -map_draw_offsety,
                            main_screen->w,
                            main_screen->h};

    SDL_FillRect(main_screen, &fill_rect, SDL_MapRGB(main_screen->format, 255,255,255));
    SDL_BlitSurface(map_buffer, &overlay_rect, main_screen, &fill_rect);
    draw_player();
    draw_player_data();

    ticks++;
}

/**
 * returns the block type given at the screen position (x,y)
 */
int get_map_block(int x, int y)
{
    int blkx = x/16;
    int blky = y/16;
    if(blky == map->h)
        return  BLK_SPKE;
    if(blky < 0 || blkx < 0 || blkx >= map->w || blky > map->h)
        return BLK_OBSD;
    int *pxl = (map->pixels + blky * map->pitch + blkx * map->pitch / map->w);
    return *pxl;
}

/**
 * removes the map block at the given (x,y) position
 * (uses map offset position not block array position or screen position)
 */
void remove_map_block(int x, int y)
{
    int blkx = x/16;
    int blky = y/16;
    if(blky < 0 || blkx < 0)
        return;
    int pxl = 0xFFFFFFFF; // sky;
    memcpy(map->pixels + blky * map->pitch + blkx * map->pitch / map->w, &pxl, 4);

    SDL_Rect fill_rect = {blkx * 16, blky * 16, 16, 16};
    SDL_FillRect(map_buffer, &fill_rect, get_background_color());
}

void find_map_block(int pxl, int *x, int *y)
{
    int i, j;
    int bpp = map->pitch / map->w;
    for(i = 0; i < map->h; i++)
        for(j = 0; j < map->w; j++){
            if(!memcmp(map->pixels + i * map->pitch + j * bpp,
                        &pxl, bpp)){
                *x = j;
                *y = i;
                return;
            }
        }
}

static void update_game()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)){
        switch (event.type){
            case SDL_QUIT:
                game_state = QUIT;
                break;
            case SDL_KEYDOWN:
                break;
            case SDL_KEYUP:
                break;
        }
    }

    SDL_PumpEvents();
    if (key_state[SDLK_SPACE]){
        int x, y;
        find_map_block(0xFF000000, &x, &y);
        respawn_player(x * 16, y * 16);
    }
    if (key_state[SDLK_RETURN]){
        //game_state = QUIT;
    }
    if (key_state[SDLK_LEFT]){
        handle_player_key_event(SDLK_LEFT);
    }
    if (key_state[SDLK_RIGHT]){
        handle_player_key_event(SDLK_RIGHT);
    }
    if(key_state[SDLK_UP]){
        handle_player_key_event(SDLK_UP);
    }
    if(key_state[SDLK_DOWN]){
        handle_player_key_event(SDLK_DOWN);
    }
    if (key_state[SDLK_a]){
        handle_player_key_event(SDLK_a);
    }

    update_player();
}

static int ms_passed;
void run_game()
{
    ticks = 0;
    win = 0;
    int len;
    char **levels = list_game_levels(&len);
    load_game_level(levels[get_level_selection()]);
    while(game_state == RUNNING && !win){
        ms_passed = SDL_GetTicks();
        update_game();
        draw_game();
        //SDL_Flip(main_screen);
        SDL_UpdateRect(main_screen,0,0,main_screen->w,main_screen->h);
        ms_passed = SDL_GetTicks() - ms_passed;
        if(ms_passed < 16){
            SDL_Delay(16 - ms_passed);
        }
    }
}
