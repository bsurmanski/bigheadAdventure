/**
 * font.h
 * platform-fun
 * October 07, 2011
 * Brandon Surmanski
 */

#ifndef _FONT_H
#define _FONT_H

void init_font();
void draw_text(SDL_Surface *dest, const char *str, int x, int y);

#endif
