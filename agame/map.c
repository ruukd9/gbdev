#include <gb/gb.h>
#include <rand.h>

#include "variables.h"
#include "res/bg/EmptyBlock.h"
#include "res/bg/CloudBlock.h"
#include "res/bg/FishBlock.h"
#include "res/bg/SnowBlock.h"
// #include "res/bg/IceBlock.h"
#include "res/bg/CrackedBlock.h"

uint8_t world_pixels_x = 0;
uint8_t world_blocks_x = 0;

// B0 -> target_tile = 24
// B1 -> target_tile = 28
// B2 -> target_tile =  0
// B3 -> target_tile =  4
// B4 -> target_tile =  8
// B5 -> target_tile = 12
// B6 -> target_tile = 16
// B7 -> target_tile = 20
uint8_t next_target_tile(){
  return (uint8_t)(world_blocks_x*BLOCK_WIDTH_TILES + 24) & 31; // == x % 32
}

uint8_t world_blocks_y = 0; //  0 == (SCREENHEIGHT - 16)px == (SCREENHEIGHT/8 - 2)tiles
void generate_column_for(uint8_t target_x_tile){
  // calc new world_blocks_y from current
  uint8_t new_world_blocks_y = 1; // default 1 as fallback in case something is wrong
  uint8_t r_height = (uint8_t)rand();

  if(world_blocks_y == 0){
    // can only grow or stay the same
    new_world_blocks_y = r_height < 64 ? 0 : 1; // 1 in 4 arbitrarily
  }else if(world_blocks_y == WORLD_MAX_Y){
    // can only shrink or stay the same
    new_world_blocks_y = r_height < 64 ? 1 : 2; // 1 in 4 arbitrarily
  }else{
    // can safely do +-1 randomly
    new_world_blocks_y = r_height < 85 ?  world_blocks_y-1  // 1/3
      : (r_height < 170 ?                 world_blocks_y    // 1/3
      :                                   world_blocks_y+1  // 1/3
    );
  }

  const unsigned char *block_tile;                          // kind of tile
  uint8_t platform_block_height  = new_world_blocks_y + 1;  // number of solid blocks in that col (or solid height of the col)
  // get a new block for every block of the col (from the bottom)
  // we have to go for more than COL_HEIGHT (based on WORLD_MAX_Y)
  // cause eventually we'll shift the whole col DOWN according to the STEP_HEIGHT and new_world_blocks_y (we have to guarantee to overwrite every "old" tile)
  // this does mean that we potentially draw out of bounds for higher levels but its not necessarily relevant since you dont see it
  // also its easy to add an if() to not draw them if it becomes an issue
  // is it the best way to do it? idk but its what i thought of
  uint8_t shift_offset = platform_block_height*STEP_HEIGHT_TILES;
  // ceiling of ratio to know how many tiles needed to cover the post-shift hole
  uint8_t height_to_cover = COL_HEIGHT + ( (shift_offset+BLOCK_HEIGHT_TILES-1)/BLOCK_HEIGHT_TILES );
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

    uint8_t staring_tile_block_y = DEVICE_SCREEN_HEIGHT - 2;              // top of the first block in the col
    uint8_t tile_y = staring_tile_block_y - (block_i*BLOCK_HEIGHT_TILES); // subtract one block height every entry (stack)
    uint8_t y_offset = new_world_blocks_y*STEP_HEIGHT_TILES;              // offset DOWN based on the y value  (shift assembled column down X tiles)
    set_bkg_based_tiles(target_x_tile, tile_y + y_offset, BLOCK_WIDTH_TILES, BLOCK_HEIGHT_TILES, block_tile, MAP_TILES_START);
  }

  world_blocks_y = new_world_blocks_y;
}

// scrolls right 1px
// also generates next column of the map if necessary
void update_camera(){
  scroll_bkg(1, 0);
  world_pixels_x++;
  if(world_pixels_x == BLOCK_WIDTH_PX){
    // travelled 1 block -> generate first out of bounds (next appearing)
    generate_column_for(next_target_tile());
    world_blocks_x++;
    world_pixels_x = 0;
  }
}