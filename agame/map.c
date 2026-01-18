#include <gb/gb.h>
#include <rand.h>

#include "variables.h"
#include "res/bg/EmptyBlock.h"
#include "res/bg/CloudBlock.h"
#include "res/bg/FishBlock.h"
#include "res/bg/SnowBlock.h"
#include "res/bg/IceBlock.h"
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
  switch (world_blocks_y){
    case 0:
      new_world_blocks_y = r_height < 64 ? 0 : 1; // 1 in 4 arbitrarily
      break;
    case 1:
      new_world_blocks_y = r_height < 85 ?  0      // 1/3
        : (r_height < 170 ?                 1      // 1/3
        :                                   2      // 1/3
      );
      break;
    case 2:
      new_world_blocks_y = r_height < 64 ? 1 : 2; // 1 in 4 arbitrarily
      break;
  }

  uint8_t platform_block_height  = new_world_blocks_y + 1;  // number of solid blocks in that col (or solid height of the col)
  uint8_t col_has_cloud = 0;                                // at most one cloud per col
  const unsigned char *block_tile;                          // kind of tile
  // get a new block for every block of the col (from the bottom)
  // we increase by 1 cause eventually we'll shift the whole col DOWN according to the STEP_HEIGHT (we have to guarantee to overwrite every "old" tile, so up to y=0)
  // "1" in this case is just roof(WORLD_MAX_Y (2) * STEP_HEIGHT_TILES (2) / BLOCK_HEIGHT_TILES (4))
  // but untill i find a clever way to make the max world level customizable (not 2) might as well just write 1
  // is it the best way to do it? idk but its what i thought of
  for(uint8_t block_i=0; block_i<COL_HEIGHT+1; block_i++){
    if(block_i < platform_block_height){
      // between 0 and the solid height (1/2/3)
      uint8_t r_block = (uint8_t)rand();

      if(block_i == platform_block_height - 1){
        // last solid one
        block_tile = r_block < 85 ? SnowBlock     // 1/3
          : (r_block < 170 ?        IceBlock      // 1/3
          :                         CrackedBlock  // 1/3
        );
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