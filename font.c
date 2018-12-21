/**
 * font.c
 * platform-fun
 * October 07, 2011
 * Brandon Surmanski
 */

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>

#include "font.h"
#include "game.h"

extern SDL_Renderer *renderer;
static SDL_Texture *font;

void init_font(){
    font = get_texture("font.png");
}

void draw_text(const char *str, int x, int y){
    int font_w, font_h;

    SDL_QueryTexture(font, NULL, NULL, &font_w, &font_h);
    SDL_Rect src_rect = {0,0,font_w,font_w};
    SDL_Rect dest_rect = {x,y,font_w, font_w};
    while (*str){
        if ((*str) == ' '){
            dest_rect.x += src_rect.w;
            ++str;
            continue;
        }
        src_rect.y = (*str - '!') * src_rect.w;
        SDL_RenderCopy(renderer, font, &src_rect, &dest_rect);
        dest_rect.x += src_rect.w + 1;
        ++str;
    }
}
