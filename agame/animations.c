#include <gb/gb.h>
#include <gb/metasprites.h>

#include "map.h"
#include "utils.h"

#include "res/bg/map/FishBlock.h"

/* local defs */
// fish animate-able tiles index in FishBlock (inner 2x2 of a 4x4 block)
#define FISH_TILE_TL     5
#define FISH_TILE_TR     6
#define FISH_TILE_BL     9
#define FISH_TILE_BR    10
// empty block to cover previous tiles (we'll get it from FishBlock too)
#define FISH_TILE_EMPTY  0
// animation direction / position in a 4x4 tiles block
#define ANIMATE_NONE  0
#define ANIMATE_UP    1
#define ANIMATE_DOWN  2
#define ANIMATE_LEFT  3
#define ANIMATE_RIGHT 4

const uint8_t fish_ani_steps[4] = { ANIMATE_NONE, ANIMATE_UP, ANIMATE_NONE, ANIMATE_DOWN };
uint8_t fish_ani_frame_idx[MAP_COLS] = {0}; // current index of animation, one frame per fish, max COL fishes (recycled)

void animate_bg(void){
  /* FISHES */
  // (this might be very heavy when added with everything else, we'll see)
  uint8_t start_block_x     = 0;
  uint8_t last_block_x      = MAP_COLS;
  uint8_t min_fish_block_y  = 1; // at least sitting on a level 0
  uint8_t max_fish_block_y  = WORLD_MAX_Y+1; // at most sitting on a max height block

  for(uint8_t block_x=start_block_x; block_x<last_block_x; block_x++){
    for(uint8_t block_y=min_fish_block_y; block_y<=max_fish_block_y; block_y++){
      if(current_map_state[block_y*MAP_COLS + block_x] == FISH){
        uint8_t tile_y    = STARING_TILE_BLOCK_Y - (block_y*BLOCK_HEIGHT_TILES);  // subtract one block height every entry (stack)
        uint8_t y_offset  = current_map_height[block_x]*STEP_HEIGHT_OFFSET_TILES; // offset DOWN based on the y value
        uint8_t fish_top_left_y = tile_y + y_offset;
        uint8_t fish_top_left_x = block_x*BLOCK_WIDTH_TILES;

        uint8_t current_ani_frame_idx = fish_ani_frame_idx[block_x]; // 0,1,2,3
        uint8_t current_ani_step = fish_ani_steps[current_ani_frame_idx]; // ANIMATE_NONE, ANIMATE_UP, ANIMATE_DOWN

        switch (current_ani_step){
          case ANIMATE_UP:
            // draw fish at the top
            set_bkg_tile_xy(fish_top_left_x+1, fish_top_left_y, FishBlock[FISH_TILE_TL]);
            set_bkg_tile_xy(fish_top_left_x+2, fish_top_left_y, FishBlock[FISH_TILE_TR]);
            set_bkg_tile_xy(fish_top_left_x+1, fish_top_left_y+1, FishBlock[FISH_TILE_BL]);
            set_bkg_tile_xy(fish_top_left_x+2, fish_top_left_y+1, FishBlock[FISH_TILE_BR]);
            // mask bottom with empty
            set_bkg_tile_xy(fish_top_left_x+1, fish_top_left_y+2, FishBlock[FISH_TILE_EMPTY]);
            set_bkg_tile_xy(fish_top_left_x+2, fish_top_left_y+2, FishBlock[FISH_TILE_EMPTY]);
            break;
          case ANIMATE_DOWN:
            // draw fish at the bottom
            set_bkg_tile_xy(fish_top_left_x+1, fish_top_left_y+2, FishBlock[FISH_TILE_TL]);
            set_bkg_tile_xy(fish_top_left_x+2, fish_top_left_y+2, FishBlock[FISH_TILE_TR]);
            set_bkg_tile_xy(fish_top_left_x+1, fish_top_left_y+3, FishBlock[FISH_TILE_BL]);
            set_bkg_tile_xy(fish_top_left_x+2, fish_top_left_y+3, FishBlock[FISH_TILE_BR]);
            // mask top with empty
            set_bkg_tile_xy(fish_top_left_x+1, fish_top_left_y+1, FishBlock[FISH_TILE_EMPTY]);
            set_bkg_tile_xy(fish_top_left_x+2, fish_top_left_y+1, FishBlock[FISH_TILE_EMPTY]);
            break;
          case ANIMATE_NONE:
          default:
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
            break;
        }

        // set next frame
        fish_ani_frame_idx[block_x] = current_ani_frame_idx+1 < LEN(fish_ani_steps) ? (current_ani_frame_idx+1) : 0;
      }
    }
  }
}