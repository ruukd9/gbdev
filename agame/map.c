#include <gb/gb.h>
#include <rand.h>
#include <gb/metasprites.h>
#include <gb/cgb.h>

#include "variables.h"

#include "res/bg/BGTiles.h"
#include "res/bg/BGMap.h"
#include "res/fonts/FontTiles.h"

#include "res/bg/EmptyBlock.h"
#include "res/bg/CloudBlock.h"
#include "res/bg/FishBlock.h"
#include "res/bg/SnowBlock.h"
#include "res/bg/CrackedBlock.h"

// holds the entire map state (interesting flags like is it an enemy, what kind of block etc)
uint8_t current_map_state[(32/BLOCK_WIDTH_TILES)*(32/BLOCK_HEIGHT_TILES)] = {EMPTY};
// holds the y value (ground level) for every col, always starts at 0,0 regardless of scroll
uint8_t current_map_height[32/BLOCK_WIDTH_TILES] = {0};

// creates and draws a new column on the map
// also updates current_map_state
// @param target_x_tiles tile reference on the x axis of the column to generate
// @param previous_col_world_y height of the previous column (to make the next one consistent and reachable)
// @returns the height of the newly generated column
static uint8_t generate_column_for(uint8_t target_x_tile, uint8_t previous_col_world_y){
  // calc new col world_blocks_y from current
  uint8_t new_world_blocks_y = previous_col_world_y; // default as fallback in case something is wrong
  uint8_t r_height = (uint8_t)rand();

  if(previous_col_world_y == 0){
    // can only grow or stay the same
    new_world_blocks_y = r_height < 64 ? 0 : 1;                       // 1 in 4 arbitrarily
  }else if(previous_col_world_y == WORLD_MAX_Y){
    // can only shrink or stay the same
    new_world_blocks_y = r_height < 64 ? WORLD_MAX_Y : WORLD_MAX_Y-1; // 1 in 4 arbitrarily
  }else{
    // can safely do +-1 randomly
    new_world_blocks_y = r_height < 85 ?  previous_col_world_y-1      // 1/3
      : (r_height < 170 ?                 previous_col_world_y        // 1/3
      :                                   previous_col_world_y+1      // 1/3
    );
  }

  const unsigned char *block_tile; // kind of tile
  const uint8_t staring_tile_block_y = DEVICE_SCREEN_HEIGHT - STEP_HEIGHT_OFFSET_TILES; // top of the first block in the col
  const uint8_t platform_block_height  = new_world_blocks_y + 1;  // number of solid blocks in that col (or solid height of the col)
  // get a new block for every block of the col (from the bottom)
  // we have to go for more than COL_HEIGHT (based on WORLD_MAX_Y)
  // cause eventually we'll shift the whole col DOWN according to the STEP_HEIGHT and new_world_blocks_y (we have to guarantee to overwrite every "old" tile)
  // this does mean that we potentially draw out of bounds for higher levels but its not necessarily relevant since you dont see it
  // also its easy to add an if() to not draw them if it becomes an issue
  // is it the best way to do it? idk but its what i thought of
  const uint8_t shift_offset = platform_block_height*STEP_HEIGHT_OFFSET_TILES;
  // ceiling of ratio offset/tilesH to know how many tiles needed to cover the post-shift hole
  const uint8_t height_to_cover = COL_HEIGHT + ( (shift_offset+BLOCK_HEIGHT_TILES-1)/BLOCK_HEIGHT_TILES );
  uint8_t col_has_cloud = 0; // at most one cloud per col
  for(uint8_t block_i=0; block_i<height_to_cover; block_i++){
    if(block_i < platform_block_height){
      // between 0 and the solid height
      uint8_t r_block = (uint8_t)rand();

      if(block_i == platform_block_height - 1){
        // last solid one -> cracked or snow (cracked 1/3 of the time)
        block_tile = r_block < 170 ? SnowBlock : CrackedBlock;
      }else{
        // show "background" under the active platform
        block_tile = EmptyBlock;
      }
    }else if(block_i == platform_block_height){
      // first empty one -> fish or empty (fish around 1/4 of the time)
      block_tile = (uint8_t)rand() < 64 ? FishBlock : EmptyBlock;
    }else{
      if(col_has_cloud){ // max 1 cloud per col
        block_tile = EmptyBlock;
      }else{
        uint8_t r_sky = (uint8_t)rand();
        // cloud around 1/3 of the time
        if(r_sky < 85){
          block_tile = CloudBlock;
          col_has_cloud = 1;
        }else{
          block_tile = EmptyBlock;
        }
      }
    }

    uint8_t tile_y = staring_tile_block_y - (block_i*BLOCK_HEIGHT_TILES); // subtract one block height every entry (stack)
    uint8_t y_offset = new_world_blocks_y*STEP_HEIGHT_OFFSET_TILES;       // offset DOWN based on the y value  (shift assembled column down X tiles)
    set_bkg_based_tiles(target_x_tile, tile_y + y_offset, BLOCK_WIDTH_TILES, BLOCK_HEIGHT_TILES, block_tile, MAP_TILES_START);

    // update current map state with the new col
    // x_coord = target_x_tile/BLOCK_WIDTH_TILES
    // y_coord = block_i
    if      (block_tile == SnowBlock    ) { current_map_state[block_i*(32/BLOCK_HEIGHT_TILES) + (target_x_tile/BLOCK_WIDTH_TILES)] = SOLID;     }
    else if (block_tile == CrackedBlock ) { current_map_state[block_i*(32/BLOCK_HEIGHT_TILES) + (target_x_tile/BLOCK_WIDTH_TILES)] = BREAKABLE; }
    else if (block_tile == FishBlock    ) { current_map_state[block_i*(32/BLOCK_HEIGHT_TILES) + (target_x_tile/BLOCK_WIDTH_TILES)] = FISH;      }
    else                                  { current_map_state[block_i*(32/BLOCK_HEIGHT_TILES) + (target_x_tile/BLOCK_WIDTH_TILES)] = EMPTY;     }
  }

  // update height for col
  current_map_height[target_x_tile/BLOCK_WIDTH_TILES] = new_world_blocks_y;

  return new_world_blocks_y;
}

// initializes world_height
// load initial visible map (col by col)
// which in turns also inits the current map state with the visible columns (+1)
// also loads enemy sprite data
void init_map(void){
  /* bkg stuff */
  SHOW_BKG;
  uint8_t previous_col_world_y = 0; //  0 == (SCREENHEIGHT - 16)px == (SCREENHEIGHT/8 - 2)tiles
  set_bkg_data(MAP_TILES_START, 17, BGTiles);
  for(uint8_t x_tile=0; x_tile<=DEVICE_SCREEN_WIDTH;x_tile+=4){ // <= so we load an extra one to buffer
    previous_col_world_y = generate_column_for(x_tile, previous_col_world_y);
  }
}

// scrolls everything right 1px
// also generates next column of the map if necessary
void update_camera(void){
  scroll_bkg(1, 0); // SCX_REG++
  if((SCX_REG & (BLOCK_WIDTH_PX-1)) == 0){ // % 32
    // travelled 1 block -> generate first out of bounds (next appearing)
    // SCX_REG is the leftmost pixel on screen, goes from 0->255
    // so we convert in block coords
    // its the previous so we do -blockW
    uint8_t last_col_x_coord = ((SCX_REG + SCREENWIDTH - BLOCK_WIDTH_PX)/BLOCK_WIDTH_PX) & 7; // % 8 == size of the array for wraparound
    // get last used height
    uint8_t last_col_world_y = current_map_height[last_col_x_coord];
    // target the next tile to show the col at
    uint8_t next_target_tile = ((SCX_REG/8) + DEVICE_SCREEN_WIDTH) & (DEVICE_SCREEN_BUFFER_WIDTH-1);

    generate_column_for(next_target_tile, last_col_world_y);
  }
}