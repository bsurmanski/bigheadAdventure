/**
 * player.c
 * platform-fun
 * September 27, 2011
 * Brandon Surmanski
 */

#include <math.h>
#include <stdbool.h>

#include <SDL2/SDL.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_mixer/SDL_mixer.h>

#include "player.h"
#include "map.h"
#include "game.h"
#include "list.h"

extern SDL_Texture *main_screen; // FROM MAIN.C
extern SDL_Texture *map_buffer; // FROM GAME.C
extern const char *res_pack;


/*
 * bit structure of 'near_blocks' field
 *
 * 8 bit number bit order
 * 7654 3210
 *
 *  order of bits representing player's surrounding blocks
 *  543
 *  6-2
 *  701
 *
 *  So, (near_blocks & (1 << 6)) is represents whether the block to the left is solid or not
 */

typedef struct {
    float x;
    float y;
    float vx;
    float vy;
    float max_speed;
    int score;
    uint8_t state; // BITMASK: plummet|run|?|?|?|burning|wet|dead
    /* potential future states: invincible, invisible, hurt,
     celebration (end level), low grav
     */
    uint8_t facing_dir;
    uint8_t near_blocks; //bitmask of surrounding blocks
    uint8_t action_timeout;
    SDL_Texture *sprite;
} Player;

typedef struct Particle{
    float x;
    float y;
    float vx;
    float vy;
    SDL_Color color;
    uint32_t timeout;
    bool (*update)(struct Particle *p);
    void (*draw)(struct Particle *p, int offsetx, int offsety);
} Particle;
bool particle_update_chunky(Particle *p);
bool particle_update_normal(Particle *p);
bool particle_update_snow(Particle *p);
void particle_draw_chunky(Particle *p, int offsetx, int offsety);
void particle_draw_normal(Particle *p, int offsetx, int offsety);

List *particles;

extern SDL_Renderer *renderer;
static Player current_player;
/**
 * Player resources
 */
static SDL_Texture *frame0;
static SDL_Texture *frame1;
static SDL_Texture *frame2;
static SDL_Texture *raml;
static SDL_Texture *ramr;
static SDL_Texture *jumpl;
static SDL_Texture *jumpr;
static SDL_Texture *falll;
static SDL_Texture *fallr;
static SDL_Texture *run1l;
static SDL_Texture *run2l;
static SDL_Texture *run1r;
static SDL_Texture *run2r;
static SDL_Texture *dead;
static SDL_Texture *plummet;

static Mix_Chunk *mix_jump;
static Mix_Chunk *mix_step1;
static Mix_Chunk *mix_step2;
static Mix_Chunk *mix_blkboom;
static Mix_Chunk *mix_coin;
static Mix_Chunk *mix_splash;
static Mix_Chunk *mix_spikes;
static Mix_Chunk *mix_ouch;

/**
 * load all the resources needed by the player.
 * initialize everything
 */
void load_player_resources()
{
    current_player.x = 0;
    current_player.y = 0;
    current_player.vx = 0;
    current_player.vy = 0;
    current_player.score = 0;
    current_player.max_speed = 3;
    current_player.state = 0; // bit map of player state effects
    current_player.facing_dir = 0;
    current_player.action_timeout = 0;

    //bit map of near block transparency eg. lsb = block bellow, lsb+1 = block right ...
    current_player.near_blocks = 0;

    frame0 = get_texture("bighead.png");
    frame1 = get_texture("bigheadl.png");
    frame2 = get_horizontal_flipped(frame1);
    raml = get_texture("bighead-raml.png");
    ramr = get_horizontal_flipped(raml);
    dead = get_texture("bighead-dead.png");
    run1l = get_texture("bighead-run1l.png");
    run2l = get_texture("bighead-run2l.png");
    run1r = get_horizontal_flipped(run1l);
    run2r = get_horizontal_flipped(run2l);
    plummet = get_texture("bighead-plummet.png");
    jumpl = get_texture("bighead-jump.png");
    jumpr = get_horizontal_flipped(jumpl);
    falll = get_texture("bighead-fall.png");
    fallr = get_horizontal_flipped(falll);

    current_player.sprite = frame0;

    particles = list_create(sizeof(Particle));

    mix_jump = Mix_LoadWAV("res/jump.wav");
    mix_step1 = Mix_LoadWAV("res/step1.wav");
    mix_step2 = Mix_LoadWAV("res/step2.wav");
    mix_blkboom = Mix_LoadWAV("res/block-break.wav");
    mix_coin = Mix_LoadWAV("res/coin.wav");
    mix_splash = Mix_LoadWAV("res/splash.wav");
    mix_spikes = Mix_LoadWAV("res/spikes.wav");
    mix_ouch = Mix_LoadWAV("res/ouch.wav");

}

void respawn_player(int x, int y)
{
    set_player_position(x, y);
    current_player.vy = 0;
    current_player.vx = 0;
    current_player.state = 0;
}

/**
 * move the player to the possition (x, y). used when spawning on a new map
 */
void set_player_position(int x, int y)
{
    current_player.x = x;
    current_player.y = y;
}

extern int map_draw_offsetx; // FROM GAME.C
extern int map_draw_offsety;


static void draw_particles()
{
    Particle *p;
    Node *current_node = list_first_node(particles);
    int OFFSETX, OFFSETY;

    while(current_node){
        p = (Particle*) node_value(current_node);
        p->draw(p, -map_draw_offsetx, map_draw_offsety);
        current_node = node_next(current_node);
    }
}

/**
 * particle draw functions
 */
void particle_draw_normal(Particle *p, int offsetx, int offsety)
{
    SDL_SetRenderDrawColor(renderer,
                           p->color.r,
                           p->color.g,
                           p->color.b,
                           p->color.a);
    SDL_RenderDrawPoint(renderer,
                        p->x + offsetx,
                        p->y + offsety);
}

void particle_draw_chunky(Particle *p, int offsetx, int offsety){
    SDL_Point points[5] = {{0, 0}, {-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for(int i = 0; i < 5; i++) {
        points[i].x += p->x + offsetx;
        points[i].y += p->y + offsety;
    }
    SDL_SetRenderDrawColor(renderer,
                           p->color.r,
                           p->color.g,
                           p->color.b,
                           p->color.a);
    SDL_RenderDrawPoints(renderer,
                         points, 5);
}

/**
 * draw the player to the default surface
 */
void draw_player()
{
    int w, h;
    SDL_QueryTexture(current_player.sprite, NULL, NULL, &w, &h);
    SDL_Rect dest = {(int)current_player.x - map_draw_offsetx,
                    (int)current_player.y + map_draw_offsety,
                    w,
                    h};

    SDL_RenderCopy(renderer, current_player.sprite, NULL, &dest);
    draw_particles();
}


/**
 * if the block type passed as blk_type is solid (cannot pass though)
 * then return 1, else return 0
 */
static uint8_t block_is_solid(int blk_code)
{
    bool ret;
    switch(blk_code){
        case BLK_BRCK: // BRICK
        case BLK_GRSS: // GRASS
        case BLK_OBSD: // OBSIDIAN
            ret = true;
            break;
        default:
            ret = false;
    }
    return ret;
}

static bool block_is_breakable(int blk_code)
{
    bool ret;
    switch(blk_code){
        case BLK_OBSD:
            ret = false;
            break;
        default:
            ret = true;
            break;
    }
    return ret;
}

/**
 * if the block type passed as blk_code is collectible, return 1, else return 0
 */
static bool block_is_collectible(int blk_code)
{
    bool ret;
    switch (blk_code){
        case BLK_COIN: //coin
            ret = true;
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}

/**
 * if the block type given by blk_code is deadly, return 1, else return 0
 */
static bool block_is_deadly(int blk_code)
{
    bool ret;
    switch(blk_code){
        case BLK_SPKE:
            ret = true;
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}

static bool block_is_goal(int blk_code)
{
    bool ret;
    switch(blk_code){
        case BLK_CAKE:
            ret = true;
            break;
        default:
            ret = false;
            break;
    }
    return ret;
}

static uint8_t attempt_to_break_block(int x, int y)
{
    int ret = 0;
    if (block_is_breakable(get_map_block(x,y))){
        remove_map_block(x, y);
        Mix_PlayChannel(-1, mix_blkboom, 0);
        ret = 1;

        int i;
        for(i = 0; i < 10; i++){
            Particle *p = malloc(sizeof(Particle));
            p->x = x;
            p->y = y;
            p->vx = rand() % 5 - 2.5;
            p->vy = rand() % 5 - 2.5;
            p->color = (SDL_Color){0, 0, 0, 255};
            p->draw = particle_draw_chunky;
            p->update = particle_update_normal;
            p->timeout = rand() % 100;
            list_append(particles, p);
        }
    }
    return ret;
}

/**
 * Updates a particle array (eg blood). will move the particles according to
 * the particles velocity, and if a particle hits a solid block, will stop
 * the particle.
 */
static void update_particles()
{
    Particle *p;
    Node *current_node = list_first_node(particles);
    while(current_node){
        p = node_value(current_node);
        if(p->update(p)){
            Node *next = node_next(current_node);
            list_remove(particles, current_node);
            current_node = next;
        } else {
            current_node = node_next(current_node);
        }
    }
}

int blk;
bool particle_update_normal(Particle *p)
{
    p->x += p->vx;
    p->y += p->vy;
    blk = get_map_block(p->x, p->y);
    if (block_is_solid(blk)){

        //ADD TO DEATH BUFFER
        SDL_SetRenderTarget(renderer, map_buffer);
        particle_draw_normal(p, 0, 0);
        SDL_SetRenderTarget(renderer, NULL);
        return true;
    } else {
        p->vy += 0.1;
        if(outside_buffer(map_buffer, p->x, p->y)){
          return true;
        }
    }
    return false;
}

bool particle_update_chunky(Particle *p)
{
    p->timeout--;
    if(p->timeout <= 0){
        return true;
    }

    int nextx = p->x + p->vx;
    int nexty = p->x + p->vy;
    blk = get_map_block(nextx, nexty);
    if(block_is_solid(blk)){
       if(((int)(nextx / 16.0)) != ((int)p->x) / 16){
           //p->vx *= -1;
       } else {
           p->vy *= -1;
       }
    } else {
        p->vy += 0.1;
    }
    p->x += p->vx;
    p->y += p->vy;
    return false;
}

bool particle_update_snow(Particle *p)
{
    int nextx = (int)(p->x + p->vx);
    int nexty = (int)(p->y + p->vy);
    if(outside_buffer(map_buffer, nextx, nexty)){
        return true;
    }
    p->x += p->vx;
    p->y += p->vy;
    return false;
}

/**
 * Will update the array of near blocks. If the currently touching block is
 * collectible, it will be collected. if it is deadly, it will kill the player
 */
static void update_near_blocks()
{
    current_player.near_blocks = 0;

    int w, h;
    SDL_QueryTexture(current_player.sprite, NULL, NULL, &w, &h);
    int nextx = current_player.x + current_player.vx;
    int nexty = current_player.y + current_player.vy;

    int blk = 0;
    blk = get_map_block(nextx + 4, nexty + h - 1);
    current_player.near_blocks |= block_is_solid(blk);
    blk = get_map_block(nextx + w - 4, nexty + h -1);
    current_player.near_blocks |= block_is_solid(blk);

    blk = get_map_block(nextx + w - 1, nexty + 4);
    current_player.near_blocks |= (block_is_solid(blk) << 2);
    blk = get_map_block(nextx + w - 1, nexty + h - 4);
    current_player.near_blocks |= (block_is_solid(blk) << 2);

    blk = get_map_block(nextx, nexty + 4);
    current_player.near_blocks |= (block_is_solid(blk) << 6);
    blk = get_map_block(nextx, nexty + h - 4);
    current_player.near_blocks |= (block_is_solid(blk) << 6);

    blk = get_map_block(nextx + 4, nexty);
    current_player.near_blocks |= (block_is_solid(blk) << 4);
    blk = get_map_block(nextx + w - 4, nexty);
    current_player.near_blocks |= (block_is_solid(blk) << 4);


    blk = get_map_block(nextx + w/2, nexty + h/2); //current block

    if(block_is_goal(blk)){
        //win
        extern bool win;
        win = true;
    }

    if (blk == BLK_WATR){
        if (!(current_player.state & (1 << 1))){ // in water and not wet
            Mix_PlayChannel(-1, mix_splash, 0);
            int x = current_player.x + w / 2;
            int y = current_player.y + h;
            int i;
            for (i = 0; i < 100; i++){
                    Particle *p = malloc(sizeof(Particle));
                    p->x = x;
                    p->y = y;
                    p->vx = ((rand() % 100)/40.0) - 1.25;
                    p->vy = -((rand() % 100)/40.0);
                    p->draw = particle_draw_normal;
                    p->update = particle_update_normal;
                    p->color = (SDL_Color){0, 0, 255, 255};
                list_append(particles, p);
                free(p);
            }
            current_player.state |= (1 << 1);
        }
    } else {
        current_player.state &= ~(1<<1); // MAKE PLAYER NOT WET
    }

    if (!(current_player.state & (1 << 0))){ // PLAYER NOT DEAD
        if(block_is_collectible(blk)){ //remove collectible block
            remove_map_block(nextx + w/2, nexty + w/2);
            Mix_PlayChannel(-1, mix_coin, 0);
            if (current_player.state & (1<<7)){
                current_player.score+= 500;
            } else{
                current_player.score+= 100;
            }
        }
        if(block_is_deadly(blk)){
            //KILL PLAYER
             current_player.state = (1 << 0); //set player dead
             Mix_PlayChannel(-1, mix_ouch, 0);
             Mix_PlayChannel(-1, mix_spikes, 0);

            int x = current_player.x + w / 2;
            int y = current_player.y + h;
            int i;
            for (i = 0; i < 1000; i++){
                Particle *p = malloc(sizeof(Particle));
                p->x = x;
                p->y = y;
                p->vx = ((rand() % 100)/20.0) - 2.5;
                p->vy = -((rand() % 100)/20.0);
                p->color = (SDL_Color){255, 0, 0, 255};
                p->draw = particle_draw_normal;
                p->update = particle_update_normal;
                list_append(particles, p);
                free(p);
            }
        }
    }
}

static void update_sprites(){
    extern int ticks;

    if (!(current_player.state & (1<<0))){ // NOT DEAD
        if (current_player.state & (1 << 7)){ //PLUMMETING
            current_player.sprite = plummet;
          } else if (current_player.vy < 0){ // JUMPING
           current_player.sprite = current_player.facing_dir ?
                    jumpr :
                    jumpl;
        /*} else if (current_player.vy > 0) { //FALLING
            current_player.sprite = current_player.facing_dir?
                    fallr :
                    falll;*/
        } else if (current_player.vx > 0.5 || current_player.vx < -0.5){ //MOVING
            if (ticks % 10 == 0){
                if (current_player.state & (1<<6) && current_player.vx < 0){ //running left
                    if (current_player.sprite == run1l){
                        current_player.sprite = run2l;
                        Mix_PlayChannel(-1, mix_step2, 0);
                    } else {
                        current_player.sprite = run1l;
                        Mix_PlayChannel(-1, mix_step1, 0);
                    }
                } else if (current_player.state & (1<<6) && current_player.vx > 0){ //running right
                    if (current_player.sprite == run1r){
                        current_player.sprite = run2r;
                        Mix_PlayChannel(-1, mix_step2, 0);
                    } else {
                        current_player.sprite = run1r;
                        Mix_PlayChannel(-1, mix_step1, 0);
                    }
                } else {
                    if (current_player.sprite == frame1){
                        current_player.sprite = frame2;
                    } else {
                        current_player.sprite = frame1;
                    }
                }
            }
        } else {
            current_player.sprite = frame0;
            current_player.vx *= 0.9;
        }
    } else {
        current_player.sprite = dead;
    }
}

void update_player()
{
    extern int ticks; // FROM GAME.C

    update_near_blocks();
    update_particles();
    update_sprites();

    if (current_player.vx >= current_player.max_speed ||
                        current_player.vx <= -current_player.max_speed){
        current_player.state |= (1<<6);
    } else {
        current_player.state &= ~(1<<6);
    }

    if (current_player.near_blocks & ((1 << 0))){ //SOLID BLOCK BELLOW
        if (current_player.state & (1<<7)){ // if plummeting
            if (current_player.vy >= current_player.max_speed){
                int w, h;
                SDL_QueryTexture(current_player.sprite, NULL, NULL, &w, &h);
                int x = current_player.x + w/2;
                int y = current_player.y + 3*h/2;
                attempt_to_break_block(x,y);
            }
            current_player.action_timeout--;
            current_player.vx *=0.8;
            if (!current_player.action_timeout){
                current_player.state &= ~(1<<7); //stop plummeting
            }
        }
        current_player.vy = 0;
        //current_player.y-=0.01;
    } else if (current_player.vy < 5 && ticks % 5 == 0){
            current_player.vy+=0.5;
    }

    if (current_player.near_blocks & (1 << 4)){ //SOLID BLOCK ABOVE
        current_player.vy = 0;
        current_player.y+=0.2;
    }

    if (current_player.near_blocks & (1 << 2)){ // SOLID BLOCK ON RIGHT
        current_player.vx = -current_player.vx * 0.25;
        current_player.x-=0.1;
    }

    if (current_player.near_blocks & (1 << 6)){ // SOLID BLOCK ON LEFT
        current_player.vx = -current_player.vx * 0.25;
        current_player.x+=0.1;
    }

    if (!(current_player.state & (1<<0))){ // NOT DEAD
        if (current_player.vx > 0.5 || current_player.vx < -0.5){
        } else {
            current_player.vx *= 0.9;
        }
    } else { //IS DEAD
        current_player.vx *= 0.8;
    }

    if (current_player.state & (1<<1)){ // WET
        if (current_player.vx > 1 || current_player.vx < -1){
            current_player.vx *= 0.9;
        }
        current_player.vy *= 0.90;
    }

    current_player.x += current_player.vx;
    current_player.y += current_player.vy;

    int msw, msh;
    int mbw, mbh;
    SDL_QueryTexture(main_screen, NULL, NULL, &msw, &msh);
    SDL_QueryTexture(map_buffer, NULL, NULL, &mbw, &mbh);
    map_draw_offsetx += (current_player.x - map_draw_offsetx - msw / 2) / (6.0);
    map_draw_offsety -= (current_player.y + map_draw_offsety - msh / 2) / (6.0);

    //limit screen from showing outside of the map
    if (map_draw_offsetx + msw  > mbw)
        map_draw_offsetx = mbw - msw;
    if(-map_draw_offsety + msh > mbh)
        map_draw_offsety = -mbh + msh;
    if(map_draw_offsetx < 0)
        map_draw_offsetx = 0;
    if(map_draw_offsety > 0)
        map_draw_offsety = 0;

    // Create snow if using the xmas res pack
    if(rand() % 5 == 0 && !strcmp("xmas", res_pack)) {
        Particle p;
        p.x = map_draw_offsetx + rand() % msw;
        p.y = -map_draw_offsety + 1;
        p.vx = ((rand() % 100) / 100.0) - 0.5;
        p.vy = ((rand() % 100) / 100.0);
        p.draw = particle_draw_chunky;
        p.update = particle_update_snow;
        p.color = (SDL_Color){255, 255, 255, 255};
        list_append(particles, &p);
    }
}

int get_player_score()
{
    return current_player.score;
}

void handle_player_key_event(SDL_Scancode key)
{
    if (current_player.state & (1<<0)
            || current_player.state & (1<<7)) // PLAYER DEAD, or plummeting
        return;
    int w, h;
    SDL_QueryTexture(current_player.sprite, NULL, NULL, &w, &h);

    switch (key){
        case SDL_SCANCODE_LEFT:
            if (current_player.x - map_draw_offsetx < 0){
                current_player.x = map_draw_offsetx;
            }
            if(current_player.vx > -current_player.max_speed){
                current_player.vx -= 0.1;
                current_player.facing_dir = 0;
            }
            break;
        case SDL_SCANCODE_RIGHT:
            if(current_player.vx < current_player.max_speed){
                current_player.vx += 0.1;
                current_player.facing_dir = 1;
            }
            break;
        case SDL_SCANCODE_UP:
            if(!(current_player.state & (1<<7))){
            if(current_player.near_blocks & (1 << 0) && //SOLID BLOCK BELLOW
                        !(current_player.near_blocks & (1<<4))){ //and no block above
                current_player.vy = -current_player.max_speed * 1;
                Mix_PlayChannel(-1, mix_jump, 0);
            }
            if(current_player.state & (1<<1))
                current_player.vy = -current_player.max_speed * 0.75;
            }
            break;
        case SDL_SCANCODE_DOWN:
            if(!(current_player.near_blocks & (1 << 0))){
                current_player.state |= (1 << 7);
                current_player.action_timeout = 20;
            } else {
                current_player.vx *= 0.9;
            }
            break;
        case SDL_SCANCODE_A:
            if (current_player.facing_dir){ // FACING RIGHT
                if (current_player.near_blocks & (1 << 2) && current_player.state & (1<<6)){
                    attempt_to_break_block(current_player.x + 3 * w/2, current_player.y + h/2);
                    current_player.sprite = ramr;
                }
            } else { // FACING LEFT
                if (current_player.near_blocks & (1 << 6) && current_player.state & (1<<6)){
                    attempt_to_break_block(current_player.x - w/2, current_player.y + h/2);
                    current_player.sprite = raml;
                }
            }

        default:
            break;
    }


}
