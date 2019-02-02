/**
 * menu.h
 * platform-fun
 * October 17, 2011
 * Brandon Surmanski
 */

#ifndef _MENU_H
#define _MENU_H
enum GameState;

void draw_main_menu(void);
void draw_level_select(void);
enum GameState update_main_menu(void);
enum GameState update_level_select(void);

void menu_init(void);
void menu_deinit(void);
void run_menu(void);
int get_menu_selection(void);
int get_level_selection(void);

#endif
