// https://laroldsretrogameyard.com/tutorials/gb/using-metasprites-in-your-gbdk-games/
// https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/cross-platform/metasprites/src/metasprites.c

#include <gb/gb.h>
#include <gb/metasprites.h>
#include "Penguin.h"
#include "MapTiles.h"
#include "IceMap.h"

// frame indexes of "Penguin"
#define PENGUIN_IDLE_UP        0
#define PENGUIN_WALK_UP_1      4
#define PENGUIN_WALK_UP_2      8
#define PENGUIN_IDLE_RIGHT    12
#define PENGUIN_WALK_RIGHT_1  16
#define PENGUIN_WALK_RIGHT_2  20
#define PENGUIN_IDLE_DOWN     24
#define PENGUIN_WALK_DOWN_1   28
#define PENGUIN_WALK_DOWN_2   32
#define PENGUIN_IDLE_LEFT     36
#define PENGUIN_WALK_LEFT_1   40
#define PENGUIN_WALK_LEFT_2   44

// directions
#define UP    0
#define RIGHT 1
#define DOWN  2
#define LEFT  3

// sprite memory number
const uint8_t PENGUIN_SPRITE_NR = 0;
// metasprite pixel size
const uint8_t PENGUIN_SPRITE_SIZE = 16;
// metasprite tile composition
const metasprite_t Penguin_metasprite[] = {
  // start(n) = n-1
  { .dtile=0, .dx=0, .dy=0 },   // TL
  { .dtile=1, .dx=0, .dy=8 },   // BL
  { .dtile=2, .dx=8, .dy=-8 },  // TR
  { .dtile=3, .dx=0, .dy=8 },   // BR
  METASPR_TERM
};

// screen center
uint8_t x_pos = SCREENWIDTH/2;
uint8_t y_pos = SCREENHEIGHT/2;

// aniframe switch (0/1)
uint8_t frame = 0;
// animation steps ("speed")
uint8_t step_counter = 0;

// btnpress watcher
uint8_t current_btn;
uint8_t last_btn;
uint8_t facing;

// displays animation frame in current direction (current_btn) and sets the next frame
void move_ahead(){
  if(current_btn & J_UP){
    frame ?
      move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_UP_1, 0, PENGUIN_SPRITE_NR, x_pos, y_pos)
      : move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_UP_2, 0, PENGUIN_SPRITE_NR, x_pos, y_pos);
    scroll_bkg(0, -PENGUIN_SPRITE_SIZE);
  }
  if(current_btn & J_DOWN){
    frame ?
      move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_DOWN_1, 0, PENGUIN_SPRITE_NR, x_pos, y_pos)
      : move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_DOWN_2, 0, PENGUIN_SPRITE_NR, x_pos, y_pos);
    scroll_bkg(0, PENGUIN_SPRITE_SIZE);
  }
  if(current_btn & J_LEFT){
    frame ?
      move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_LEFT_1, 0, PENGUIN_SPRITE_NR, x_pos, y_pos)
      : move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_LEFT_2, 0, PENGUIN_SPRITE_NR, x_pos, y_pos);
    scroll_bkg(-PENGUIN_SPRITE_SIZE, 0);
  }
  if(current_btn & J_RIGHT){
    frame ?
      move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_RIGHT_1, 0, PENGUIN_SPRITE_NR, x_pos, y_pos)
      : move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_RIGHT_2, 0, PENGUIN_SPRITE_NR, x_pos, y_pos);
    scroll_bkg(PENGUIN_SPRITE_SIZE, 0);
  }

  frame = !frame;
  step_counter = 0;
}

// shows idle sprite according to last usable position
void show_idle(){
  if((current_btn & J_UP) || (last_btn & J_UP)){
    move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_UP, 0, PENGUIN_SPRITE_NR, x_pos, y_pos);
    facing = UP;
  }
  if((current_btn & J_DOWN) || (last_btn & J_DOWN)){
    move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_DOWN, 0, PENGUIN_SPRITE_NR, x_pos, y_pos);
    facing = DOWN;
  }
  if((current_btn & J_LEFT) || (last_btn & J_LEFT)){
    move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_LEFT, 0, PENGUIN_SPRITE_NR, x_pos, y_pos);
    facing = LEFT;
  }
  if((current_btn & J_RIGHT) || (last_btn & J_RIGHT)){
    move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_RIGHT, 0, PENGUIN_SPRITE_NR, x_pos, y_pos);
    facing = RIGHT;
  }
}

void main(){
  SPRITES_8x8; SHOW_SPRITES; SHOW_BKG;

  // set sprite palette OBjectPalette0
  OBP0_REG = DMG_PALETTE(DMG_LITE_GRAY, DMG_WHITE, DMG_DARK_GRAY, DMG_BLACK);

  // load all tiles for sprites
  set_sprite_data(0, 48, Penguin);
  // show idle down sprite as default
  move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_DOWN, 0, PENGUIN_SPRITE_NR, x_pos, y_pos);
  // load tiles for background
  set_bkg_data(0, 2, MapTiles);
  // show map
  set_bkg_tiles(0, 0, IceMapWidth, IceMapHeight, IceMap);


  while(1){
    // poll joypad status
    last_btn = current_btn;
    current_btn = joypad();

    if(current_btn & last_btn){
      // is pressing same button -> is walking
      step_counter++;
      if(step_counter >= 32) move_ahead();
    }else if(((current_btn & J_UP) && facing == UP    )
      || ((current_btn & J_DOWN)   && facing == DOWN  )
      || ((current_btn & J_LEFT)   && facing == LEFT  )
      || ((current_btn & J_RIGHT)  && facing == RIGHT )
    ){
      // just pressed after idle BUT it was already facing that direction -> one step
      move_ahead();
    }else{
      show_idle();
    }

    wait_vbl_done();
  }
}