#include <gb/gb.h>
#include <gb/metasprites.h>

#include "map.h"

#include "res/bg/FishBlock.h"

/* local defs */
// fish animate-able tiles index in FishBlock (inner 2x2 of a 4x4 block)
#define FISH_TILE_TL     5
#define FISH_TILE_TR     6
#define FISH_TILE_BL     9
#define FISH_TILE_BR    10
// empty block to cover previous tiles (we'll get it from FishBlock too)
#define FISH_TILE_EMPTY  0
// animation direction
#define ANIMATE_NONE  0
#define ANIMATE_UP    1
#define ANIMATE_DOWN  2
#define ANIMATE_LEFT  3
#define ANIMATE_RIGHT 4

uint8_t fish_ani_dir; // TODO: make this an array to handle different fishes differently

void animate_bg(void){
  /* FISHES */
  uint8_t start_block_x     = 0;
  uint8_t last_block_x      = MAP_COLS;
  uint8_t min_fish_block_y  = 1; // at least sitting on a level 0
  uint8_t max_fish_block_y  = WORLD_MAX_Y+1; // at most sitting on a max height block

  for(uint8_t block_x=start_block_x; block_x<last_block_x; block_x++){
    for(uint8_t block_y=min_fish_block_y; block_y<max_fish_block_y; block_y++){
      if(current_map_state[block_y*MAP_COLS + block_x] == FISH){
        uint8_t tile_y    = STARING_TILE_BLOCK_Y - (block_y*BLOCK_HEIGHT_TILES);  // subtract one block height every entry (stack)
        uint8_t y_offset  = current_map_height[block_x]*STEP_HEIGHT_OFFSET_TILES; // offset DOWN based on the y value
        uint8_t fish_top_left_y = tile_y + y_offset;
        uint8_t fish_top_left_x = block_x*BLOCK_WIDTH_TILES;

        switch (fish_ani_dir){
          case ANIMATE_NONE:
            // draw fish in the middle
            set_bkg_tile_xy(fish_top_left_x+1, fish_top_left_y+1, FishBlock[FISH_TILE_TL]);
            set_bkg_tile_xy(fish_top_left_x+2, fish_top_left_y+1, FishBlock[FISH_TILE_TR]);
            set_bkg_tile_xy(fish_top_left_x+1, fish_top_left_y+2, FishBlock[FISH_TILE_BL]);
            set_bkg_tile_xy(fish_top_left_x+2, fish_top_left_y+2, FishBlock[FISH_TILE_BR]);
            // mask bottom/top rows with empty
            set_bkg_tile_xy(fish_top_left_x+1, fish_top_left_y, FishBlock[FISH_TILE_EMPTY]);
            set_bkg_tile_xy(fish_top_left_x+2, fish_top_left_y, FishBlock[FISH_TILE_EMPTY]);
            set_bkg_tile_xy(fish_top_left_x+1, fish_top_left_y+3, FishBlock[FISH_TILE_EMPTY]);
            set_bkg_tile_xy(fish_top_left_x+2, fish_top_left_y+3, FishBlock[FISH_TILE_EMPTY]);
            // set next frame
            fish_ani_dir = ANIMATE_UP;
            break;
          case ANIMATE_UP:
            // draw fish at the top
            set_bkg_tile_xy(fish_top_left_x+1, fish_top_left_y, FishBlock[FISH_TILE_TL]);
            set_bkg_tile_xy(fish_top_left_x+2, fish_top_left_y, FishBlock[FISH_TILE_TR]);
            set_bkg_tile_xy(fish_top_left_x+1, fish_top_left_y+1, FishBlock[FISH_TILE_BL]);
            set_bkg_tile_xy(fish_top_left_x+2, fish_top_left_y+1, FishBlock[FISH_TILE_BR]);
            // mask bottom with empty
            set_bkg_tile_xy(fish_top_left_x+1, fish_top_left_y+2, FishBlock[FISH_TILE_EMPTY]);
            set_bkg_tile_xy(fish_top_left_x+2, fish_top_left_y+2, FishBlock[FISH_TILE_EMPTY]);
            fish_ani_dir = ANIMATE_NONE;
          default:
            break;
        }
      }
    }
  }
}

void init_animations(void){
  fish_ani_dir = ANIMATE_NONE;
}