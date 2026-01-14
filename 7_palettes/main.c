#include <gb/gb.h>
#include <gb/metasprites.h>
#include <gb/cgb.h>
#include <gbdk/font.h>
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

// since we use fonts, allocate map data after it
// IceMap.c already has this offset baked from GBMB
#define MAP_TILES_START 0x25

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
// palettes for gbc
const uint8_t PALETTE_F = OAMF_CGB_PAL0;
const uint8_t PALETTE_M = OAMF_CGB_PAL1;
const palette_color_t Penguin_palettes[] = {
  // PALETTE_F
  RGB_GREEN, RGB_WHITE, RGB(31, 15, 31), RGB_BLACK,
  // PALETTE_M
  RGB_GREEN, RGB_WHITE, RGB(0, 12, 20), RGB_BLACK
};
const palette_color_t Bg_palette[] = {
  RGB_DARKGRAY, RGB(20, 27, 30), RGB_WHITE, RGB(0, 18, 29)
};
uint8_t current_palette;

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
      move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_UP_1, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos)
      : move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_UP_2, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos);
    scroll_bkg(0, -PENGUIN_SPRITE_SIZE);
  }
  if(current_btn & J_DOWN){
    frame ?
      move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_DOWN_1, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos)
      : move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_DOWN_2, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos);
    scroll_bkg(0, PENGUIN_SPRITE_SIZE);
  }
  if(current_btn & J_LEFT){
    frame ?
      move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_LEFT_1, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos)
      : move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_LEFT_2, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos);
    scroll_bkg(-PENGUIN_SPRITE_SIZE, 0);
  }
  if(current_btn & J_RIGHT){
    frame ?
      move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_RIGHT_1, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos)
      : move_metasprite_ex(Penguin_metasprite, PENGUIN_WALK_RIGHT_2, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos);
    scroll_bkg(PENGUIN_SPRITE_SIZE, 0);
  }

  frame = !frame;
  step_counter = 0;
}

// shows idle sprite according to last usable position
void show_idle(){
  if((current_btn & J_UP) || (last_btn & J_UP)){
    move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_UP, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos);
    facing = UP;
  }
  if((current_btn & J_DOWN) || (last_btn & J_DOWN)){
    move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_DOWN, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos);
    facing = DOWN;
  }
  if((current_btn & J_LEFT) || (last_btn & J_LEFT)){
    move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_LEFT, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos);
    facing = LEFT;
  }
  if((current_btn & J_RIGHT) || (last_btn & J_RIGHT)){
    move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_RIGHT, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos);
    facing = RIGHT;
  }
}

// flips current_palette between PALETTE_F / PALETTE_M (needs to redraw the current sprite)
void swap_palette(){
  current_palette = current_palette == PALETTE_F ? PALETTE_M : PALETTE_F;
  // redraw
  switch (facing){
    case UP:
      move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_UP, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos);
      break;
    case DOWN:
      move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_DOWN, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos);
      break;
    case LEFT:
      move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_LEFT, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos);
      break;
    case RIGHT:
      move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_RIGHT, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos);
      break;
  }
}

void main(){
  SPRITES_8x8; SHOW_SPRITES; SHOW_BKG; SHOW_WIN;

  // set window data (dialog)
  // NB. this is very scuffed, possible improvents using
  // https://laroldsretrogameyard.com/tutorials/gb/drawing-advanced-dialogue-boxes/
  const unsigned char DialogTextMap[] = {
    //"PRESS SELECT\nTO CHANGE COLOR"
    // blank is at 0x00, then thers number 0-9 then letters a-z
    // first row
    0x1A,0x1C,0x0F,0x1D,0x1D,0x00,0x1D,0x0F,0x16,0x0F,0x0D,0x1E,
    // 12 long -> needs 8 as padding
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    // second row
    0x1E,0x19,0x00,0x0D,0x12,0x0B,0x18,0x11,0x0F,0x00,0x0D,0x19,0x16,0x19,0x1C,
    // 15 long -> needs 5 as padding
    0x00,0x00,0x00,0x00,0x00
  };
  // 1. load font
  font_init();
  font_t min_font = font_load(font_min);
  font_set(min_font); // font is loaded from 0 to 24 in VRAM
  // 2. set window tiles
  set_win_tiles(0, 0, SCREENWIDTH/8, 2, DialogTextMap);
  // 3. move to the very bottom
  move_win(7, SCREENHEIGHT - 8*2);

  // set colors if supported
  // IMPORTANT !! needs -Wm-yc flag in build
  if(_cpu == CGB_TYPE){
    set_sprite_palette(0, 2, Penguin_palettes);
    set_bkg_palette(0, 1, Bg_palette);
    current_palette = PALETTE_M;
  }else{
    // set DMG sprite palette
    OBP0_REG = DMG_PALETTE(DMG_LITE_GRAY, DMG_WHITE, DMG_DARK_GRAY, DMG_BLACK);
  }
  // load all tiles for sprites
  set_sprite_data(0, 48, Penguin);
  // show idle down sprite as default
  facing = DOWN;
  move_metasprite_ex(Penguin_metasprite, PENGUIN_IDLE_DOWN, current_palette, PENGUIN_SPRITE_NR, x_pos, y_pos);
  // load tiles for background
  set_bkg_data(MAP_TILES_START, 2, MapTiles);
  // show map (starting AFTER font)
  set_bkg_based_tiles(0, 0, IceMapWidth, IceMapHeight, IceMap, MAP_TILES_START);


  while(1){
    // poll joypad status
    last_btn = current_btn;
    current_btn = joypad();

    // palette switcher
    if(_cpu == CGB_TYPE && (current_btn & J_SELECT) && !(last_btn & J_SELECT)) {
      swap_palette();
    }

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