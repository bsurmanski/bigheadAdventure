/*
 * game.h
 * platform-fun
 * September 26, 2011
 * Brandon Surmanski
 */

#ifndef _GAME_H
#define _GAME_H

void load_game_resources(void);
char **list_game_levels(int *num_levels);
void run_game(void);
int get_map_block(int x, int y);
void remove_map_block(int x, int y);
void find_map_block(int pxl, int *x, int *y);
int outside_buffer(SDL_Texture *surface, int x, int y);
SDL_Texture *get_horizontal_flipped(SDL_Texture *surface);

#endif
