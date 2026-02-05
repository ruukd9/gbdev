#include <gb/gb.h>
#include <gb/metasprites.h>

#include "map.h"
#include "utils.h"

#include "res/bg/map/BGTiles.h"
#include "res/bg/map/FishBlock.h"

/* local defs */
// fish animate-able tiles index in FishBlock (inner 2x2 of a 4x4 block)
#define FISH_TILE_TL     5
#define FISH_TILE_TR     6
#define FISH_TILE_BL     9
#define FISH_TILE_BR    10
// empty block to cover previous tiles
// we'll use it as the start of the mask making sure to always pick empty tiles ONLY (aka indexes NOT in FISH_TILE_XX)
#define FISH_TILE_EMPTY 0
// animation direction / position of the 2x2 inner center in a 4x4 tiles block
#define ANIMATE_NONE  0
#define ANIMATE_UP    1
#define ANIMATE_DOWN  2
#define ANIMATE_LEFT  3
#define ANIMATE_RIGHT 4
// animation "frame" (tile) index within BGTiles
// these are also referenced in FishBlock (which is the map for a fish block)
#define FISH_FRAME_0_OFFSET 0 // address is 0x0D, offset zero
#define FISH_FRAME_1_OFFSET 4 // address is 0x11, 4 tiles after the default frame

// indexes to iterate drawable tiles
const uint8_t fish_ani_tile_idx[4] = { FISH_TILE_TL, FISH_TILE_TR, FISH_TILE_BL, FISH_TILE_BR};
uint8_t fish_ani_block[4]; // the actual 2x2 tiles block to draw/move

// animation steps
const uint8_t fish_ani_steps[4] = { ANIMATE_NONE, ANIMATE_UP, ANIMATE_NONE, ANIMATE_DOWN };
uint8_t fish_ani_frame_idx[MAP_COLS] = {0}; // current index of animation, one frame per fish, max COL fishes (recycled)

uint8_t currentframe = 0;
void animate_bg(void){
  /* FISHES */

  // frame (tile) selection
  uint8_t tile_offset = currentframe == 0 ? FISH_FRAME_0_OFFSET : FISH_FRAME_1_OFFSET;
  for(uint8_t i=0; i<4; i++){
    // get the index of fishblock with the tile to draw
    uint8_t tidx = fish_ani_tile_idx[i];
    // account for tile offset in VRAM for drawable tiles based on frame
    fish_ani_block[i] = FishBlock[tidx] + tile_offset;
  }

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
            // draw fish at the top (pick tiles based on animation offset)
            set_bkg_based_tiles(fish_top_left_x+1, fish_top_left_y, 2, 2, fish_ani_block, MAP_TILES_START);
            // mask bottom with empty
            set_bkg_based_tiles(fish_top_left_x+1, fish_top_left_y+2, 2, 1, &FishBlock[FISH_TILE_EMPTY], MAP_TILES_START);
            break;
          case ANIMATE_DOWN:
            // draw fish at the bottom (pick tiles based on animation offset)
            set_bkg_based_tiles(fish_top_left_x+1, fish_top_left_y+2, 2, 2, fish_ani_block, MAP_TILES_START);
            // mask top with empty
            set_bkg_based_tiles(fish_top_left_x+1, fish_top_left_y+1, 2, 1, &FishBlock[FISH_TILE_EMPTY], MAP_TILES_START);
            break;
          case ANIMATE_NONE:
          default:
            // draw fish in the middle (pick tiles based on animation offset)
            set_bkg_based_tiles(fish_top_left_x+1, fish_top_left_y+1, 2, 2, fish_ani_block, MAP_TILES_START);
            // mask bottom/top rows with empty
            set_bkg_based_tiles(fish_top_left_x+1, fish_top_left_y, 2, 1, &FishBlock[FISH_TILE_EMPTY], MAP_TILES_START);
            set_bkg_based_tiles(fish_top_left_x+1, fish_top_left_y+3, 2, 1, &FishBlock[FISH_TILE_EMPTY], MAP_TILES_START);
            break;
        }

        // set next frame
        fish_ani_frame_idx[block_x] = current_ani_frame_idx+1 < LEN(fish_ani_steps) ? (current_ani_frame_idx+1) : 0;
        // we wont have more than 1 fish per-y-coord
        break;
      }
    }
  }

  currentframe = !currentframe;
}