#include <stdio.h>
#include <gb/gb.h>
#include <rand.h>

#include "res/bg/title/TitleTiles.h"
#include "res/bg/title/TitleMap.h"
#include "res/bg/gg/GameOverTiles.h"
#include "res/bg/gg/GameOverMap.h"
#include "res/bg/title/ScoreTiles.h"

#include "title.h"
#include "map.h"

/* local defs */
// tile offsets in VRAM
#define TITLE_TILES_START 0x30
#define FONT_TILES_START  0x16
// how many frames to idle animations for (aka animate every N)
#define IDLE_TITLE_FRAMES 45
// animate-able tiles data (the "press start" text)
#define TEXT_AREA_START   ((0x0C * DEVICE_SCREEN_WIDTH) + 0x04) // index of the text start in TitleMap
#define EMPTY_AREA_START  0                                     // start index of at least H*W tiles in TitleMap
#define ANIMATE_AREA_H    1                                     // area height in tiles
#define ANIMATE_AREA_W   12                                     // area width in tiles

// draw/animate titlescreen
// wait for start button to init rand seed and return to caller
void show_titlescreen(void){
  // reset camera
  SCX_REG = 0;
  // remove sprites
  HIDE_SPRITES;

  HIDE_WIN; SHOW_BKG;

  // set starting tiles
  set_bkg_data(0, 16, TitleTiles);
  set_bkg_tiles(0, 0, TitleMapWidth, TitleMapHeight, TitleMap);

  // to animate the "press start"
  uint16_t tile_start; // index within TitleMap so > 255 potentially
  uint8_t idle_frames = IDLE_TITLE_FRAMES;
  uint8_t should_show_text = 1;

  while(1){
    vsync(); // await frame

    // animate "press start"
    if(idle_frames == IDLE_TITLE_FRAMES){
      tile_start = should_show_text ? TEXT_AREA_START : EMPTY_AREA_START;
      set_bkg_tiles(0x04, 0x0C, ANIMATE_AREA_W, ANIMATE_AREA_H, &TitleMap[tile_start]);
      // update state
      should_show_text = !should_show_text;
      idle_frames = 0;
    }

    if(joypad() & J_START){
      // init rand seed
      // https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/rand/src/rand.c
      uint16_t seed;
      waitpad(J_START);
      seed = DIV_REG;
      waitpadup();
      seed |= (UWORD)DIV_REG << 8;
      initrand(seed);
      break;
    }

    idle_frames++;
  }
}

// draw/animate end screen
// wait for start or A button return to caller
void show_endscreen(void){
  // reset camera
  // SCX_REG = 0;
  // remove sprites
  // HIDE_SPRITES;
  SHOW_BKG;

  // load ending tiles to bkg
  set_bkg_data(TITLE_TILES_START, 17, GameOverTiles);
  set_bkg_based_tiles(SCX_REG/8, 0, GameOverMapWidth, GameOverMapHeight, GameOverMap, TITLE_TILES_START);

  // ... maybe some recap/score in the future

  waitpad(J_START);
  waitpadup();
}

// loads font tiles for scoring/HUD to VRAM
void init_hud(void){
  set_bkg_data(FONT_TILES_START, 12, ScoreTiles);
}

// draws a number on screen
// @param x X coordinate in tiles of where to start
// @param y Y coordinate in tiles of where to start
// @param n the number to write
// @param layer 0: background, 1: window
//
// it does NOT call move_win even if layer is set to 1
void draw_number(uint8_t x, uint8_t y, uint16_t n, uint8_t layer){
  uint8_t buffer[5] = {0}; // uint16_t holds up to 65535
  // get digits from number
  uint16_t int_part = n;
  uint8_t buffer_index = 4; // start from the bottom (right align)
  while(int_part > 0){
    uint16_t int_part_left = int_part / 10;               // integer part
    buffer[buffer_index] = int_part - (int_part_left*10); // remainder
    int_part = int_part_left;
    buffer_index--;
    if(buffer_index > 4) break; // overflow
  }

  unsigned char current_digit;
  for(uint8_t cursor=0; cursor<5; cursor++){
    // 0-9 is the actual digit to draw, we offset it to account TILE_START + extra characters (2) in the tileset
    current_digit = buffer[cursor];
    if(layer == 1){
      // win
      set_win_tile_xy(x + cursor, y, FONT_TILES_START + 2 + current_digit);
      SHOW_WIN;
    }else{
      // bkg
      set_bkg_tile_xy(x + cursor, y, FONT_TILES_START + 2 + current_digit);
      SHOW_BKG;
    }
  }
}