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
#include <unistd.h>

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>

#include "list.h"
#include "game.h"
#include "font.h"
#include "map.h"
#include "menu.h"
#include "player.h"

extern const int RUNNING;
extern const int QUIT;


extern SDL_Renderer *renderer;
extern SDL_Texture *main_screen;
extern int game_state;
extern uint8_t *key_state;
extern const char *res_pack;

int ticks;
bool win = false;

//MAP
static SDL_Surface *map = 0;
SDL_Texture *map_buffer = 0;

static SDL_Texture *blk_brick;
static SDL_Texture *blk_grass;
static SDL_Texture *blk_coin;
static SDL_Texture *blk_spike;
static SDL_Texture *blk_water;
static SDL_Texture *blk_obsid;
static SDL_Texture *blk_cake;

extern SDL_Surface *font;
static int flicker = 0;

int map_draw_offsetx = 0;
int map_draw_offsety = 0;

void upscaleCopy(SDL_Texture *src, SDL_Texture *dst, int scale)
{
    SDL_SetRenderTarget(renderer, dst);
    SDL_RenderCopy(renderer, src, NULL, NULL);
    SDL_SetRenderTarget(renderer, NULL);
}


static SDL_Color get_background_color(){
    SDL_Color r = {200, 200, 255, 255};
    return r;
}

int getRGBA(int pxl, const SDL_PixelFormat* format) {
    uint8_t r,g,b,a;
    SDL_GetRGBA(pxl, format, &r, &g, &b, &a);
    return (((int) r) << 24) |
           (((int) g) << 16) |
           (((int) b) << 8) | a;
}

static void init_map_buffer()
{
    if(map_buffer){
        SDL_DestroyTexture(map_buffer);
    }

    map_buffer = SDL_CreateTexture(renderer,
                                   SDL_PIXELFORMAT_RGBA8888,
                                   SDL_TEXTUREACCESS_TARGET,
                                   map->w * 16,
                                   map->h * 16);
    SDL_SetRenderTarget(renderer, map_buffer);

    // clear map
    SDL_Color bgc = get_background_color();
    SDL_SetRenderDrawColor(renderer, bgc.r, bgc.g, bgc.b, bgc.a);
    SDL_RenderClear(renderer);


    SDL_Rect dest = {0,0,16,16};
    int pxl;
    int bpp = map->format->BytesPerPixel;
    int i, j;
    for(i = 0; i < map->h; i++){
        for(j = 0; j < map->w; j++){
            dest.x = j * 16;
            dest.y = i * 16;
            memcpy(&pxl,  map->pixels + i * map->pitch + j * bpp, bpp);
            int rgba = getRGBA(pxl, map->format);

            switch (rgba){
                case BLK_BRCK:
                    SDL_RenderCopy(renderer, blk_brick, NULL, &dest);
                    break;
                case BLK_GRSS:
                    SDL_RenderCopy(renderer, blk_grass, NULL, &dest);
                    break;
                case BLK_COIN:
                    SDL_RenderCopy(renderer, blk_coin, NULL, &dest);
                    break;
                case BLK_SPKE:
                    SDL_RenderCopy(renderer, blk_spike, NULL, &dest);
                    break;
                case BLK_WATR:
                    SDL_RenderCopy(renderer, blk_water, NULL, &dest);
                    break;
                case BLK_OBSD:
                    SDL_RenderCopy(renderer, blk_obsid, NULL, &dest);
                    break;
                case BLK_CAKE:
                    SDL_RenderCopy(renderer, blk_cake, NULL, &dest);
                default:
                    break;
            }
        }
    }

    SDL_SetRenderTarget(renderer, NULL);
}

int strcompare(const void* a, const void *b) {
    const char **ia = (const char **)a;
    const char **ib = (const char **)b;
    return strcmp(*ia, *ib);
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


    qsort(levels, levels_size, sizeof(char*), strcompare);

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

    int x, y;
    find_map_block(BLK_SPWN, &x, &y);
    respawn_player(x * 16, y * 16);
    win = false;
}

/**
 * Loads game resources such as images, and prepares structures
 *
 */
void load_game_resources()
{
    blk_brick = get_texture("brick.png");
    blk_grass = get_texture("grass.png");
    blk_coin = get_texture("coin.png");
    blk_spike = get_texture("spikes.png");
    blk_water = get_texture("water.png");
    blk_obsid = get_texture("obsidian.png");
    blk_cake = get_texture("cake.png");

    load_player_resources();
}

SDL_Texture *get_texture(const char *filename) {
    char buf[128];
    if(res_pack) {
        snprintf(buf, 127, "res/%s/%s", res_pack, filename);
    }

    // file does not exist
    if(access(buf, F_OK) == -1) {
        snprintf(buf, 127, "res/%s", filename);
    }

    printf("BUF: %s\n", buf);

    return IMG_LoadTexture(renderer, buf);
}

SDL_Texture *get_horizontal_flipped(SDL_Texture *src)
{
    int w, h;
    SDL_QueryTexture(src, NULL, NULL, &w, &h);
    SDL_Texture *flipped = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_RGBA8888,
                                             SDL_TEXTUREACCESS_TARGET,
                                             w, h);
    SDL_SetRenderTarget(renderer, flipped);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetTextureBlendMode(flipped, SDL_BLENDMODE_BLEND);

    // clear target texture
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
    SDL_RenderClear(renderer);

    SDL_RenderCopyEx(renderer, src, NULL, NULL, 0, NULL, SDL_FLIP_HORIZONTAL);
    SDL_SetRenderTarget(renderer, NULL);

    return flipped;
}

/**
 * determine if the point x,y falls outside the buffer space
 */
int outside_buffer(SDL_Texture *t, int x, int y)
{
    int w, h;
    SDL_QueryTexture(t, NULL, NULL, &w, &h);
    return (    x < 0
            ||  y < 0
            ||  x >= w
            ||  y >= h
            );
}

static void draw_player_data()
{
    //draw score
    char buffer[20];
    int score = get_player_score();
    sprintf(buffer, "SCORE:%d", score);
    SDL_SetRenderTarget(renderer, main_screen);
    draw_text(buffer, 5, 5);
    SDL_SetRenderTarget(renderer, NULL);
}

#define min(x,y) (x<y?x:y)
/**
 * draws the game to the screen, and calls all other relevant draw functions
 */
static void draw_game()
{
    int msw, msh;
    SDL_QueryTexture(main_screen, NULL, NULL, &msw, &msh);
    int mbw, mbh;
    SDL_QueryTexture(map_buffer, NULL, NULL, &mbw, &mbh);

    //clear screen
    SDL_SetRenderTarget(renderer, main_screen);
    SDL_RenderDrawRect(renderer, NULL);

    SDL_Rect overlay_rect = {map_draw_offsetx,
                            -map_draw_offsety,
                            min(msw, mbw),
                            min(msh, mbh)};

    SDL_Rect fill_rect = {0, 0, min(msw, mbw), min(msh, mbh)};
    SDL_RenderCopy(renderer, map_buffer, &overlay_rect, &fill_rect);
    draw_player();
    draw_player_data();

    SDL_RenderCopy(renderer, main_screen, NULL, NULL);
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
    return getRGBA(*pxl, map->format);
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
    int pxl = BLK_BKGR; // sky;
    memcpy(map->pixels + blky * map->pitch + blkx * map->pitch / map->w, &pxl, 4);

    SDL_Rect fill_rect = {blkx * 16, blky * 16, 16, 16};
    SDL_Color bgc = get_background_color();
    SDL_SetRenderDrawColor(renderer, bgc.r, bgc.g, bgc.b, bgc.a);
    SDL_SetRenderTarget(renderer, map_buffer);
    SDL_RenderFillRect(renderer, &fill_rect);
    SDL_SetRenderTarget(renderer, NULL);
}

void find_map_block(int pxl, int *x, int *y)
{
    int rgba = SDL_MapRGBA(map->format,
                           (pxl & 0xFF000000) >> 24,
                           (pxl & 0x00FF0000) >> 16,
                           (pxl & 0x0000FF00) >> 8,
                           (pxl & 0x000000FF));
    int i, j;
    int bpp = map->pitch / map->w;
    for(i = 0; i < map->h; i++)
        for(j = 0; j < map->w; j++) {
            if(!memcmp(map->pixels + i * map->pitch + j * bpp,
                        &rgba, bpp)){
                *x = j;
                *y = i;
                return;
            }
        }
}

static void update_game()
{
    static int time = 0;
    if(SDL_GetTicks() - time > 16){
        flicker = !flicker;
        time = SDL_GetTicks();
    }

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
    if (key_state[SDL_SCANCODE_SPACE]){
        int x, y;
        find_map_block(BLK_SPWN, &x, &y);
        respawn_player(x * 16, y * 16);
    }
    if (key_state[SDL_SCANCODE_RETURN]){
        //game_state = QUIT;
    }
    if (key_state[SDL_SCANCODE_LEFT]){
        handle_player_key_event(SDL_SCANCODE_LEFT);
    }
    if (key_state[SDL_SCANCODE_RIGHT]){
        handle_player_key_event(SDL_SCANCODE_RIGHT);
    }
    if(key_state[SDL_SCANCODE_UP]){
        handle_player_key_event(SDL_SCANCODE_UP);
    }
    if(key_state[SDL_SCANCODE_DOWN]){
        handle_player_key_event(SDL_SCANCODE_DOWN);
    }
    if (key_state[SDL_SCANCODE_A]){
        handle_player_key_event(SDL_SCANCODE_A);
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

        SDL_SetRenderTarget(renderer, NULL);
        SDL_RenderPresent(renderer);
        ms_passed = SDL_GetTicks() - ms_passed;
        if(ms_passed < 16) SDL_Delay(16 - ms_passed);
    }
}
