/*
 * game.h
 * platform-fun
 * September 26, 2011
 * Brandon Surmanski
 */

#ifndef _GAME_H
#define _GAME_H

enum GameState {
    TITLE = 0,
    LEVEL_SELECT,
    GAME,
    QUIT,
};

void upscaleCopy(SDL_Surface *dest, SDL_Surface *src, int scale);
void load_game_resources(void);
char **list_game_levels(int *num_levels);
void game_init(void);
void game_deinit(void);
void run_game(void);
int get_map_block(int x, int y);
void remove_map_block(int x, int y);
void find_map_block(int pxl, int *x, int *y);
int outside_buffer(SDL_Surface *surface, int x, int y);
SDL_Surface *get_horizontal_flipped(SDL_Surface *surface);
enum GameState update_game();
void draw_game();

#endif
