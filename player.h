/**
 * player.h
 * platform-fun
 * September 27, 2011
 * Brandon Surmanski
 */

#ifndef _PLAYER_H
#define _PLAYER_H
struct player_data{
    int score;
    int lives;
};

void load_player_resources();
void set_player_position(int x, int y);
void respawn_player(int x, int y);
void draw_player();
int get_player_score();
void update_player();
void handle_player_key_event(SDL_Scancode key);

struct player_data *get_player_data();

#endif
