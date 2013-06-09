/**
 * font.c
 * platform-fun
 * October 07, 2011
 * Brandon Surmanski
 */

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>

#include "font.h"

static SDL_Surface *font;

void init_font(){
    font = IMG_Load("res/font.png");
}

extern int outside_buffer(SDL_Surface *s, int x, int y);

void draw_text(SDL_Surface *dest_img, const char *str, int x, int y){
    SDL_Rect src_rect = {0,0,font->w,font->w};
    SDL_Rect dest_rect = {x,y,font->w, font->w};
    while (*str && !outside_buffer(dest_img, dest_rect.x, dest_rect.y)){
        if ((*str) == ' '){
            dest_rect.x += src_rect.w;
            ++str;
            continue;
        }
        src_rect.y = (*str - '!') * src_rect.w; 
        SDL_BlitSurface(font, &src_rect, dest_img, &dest_rect);
        dest_rect.x += src_rect.w + 1; 
        ++str;
    }
}
