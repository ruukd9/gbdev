#include <gb/gb.h>
#include <rand.h>
#include <gb/cgb.h>

#include "res/bg/map/BGTiles.h"
#include "res/bg/map/BGMap.h"
#include "res/bg/map/SeaMap.h"

#include "res/bg/map/EmptyBlock.h"
#include "res/bg/map/CloudBlock.h"
#include "res/bg/map/FishBlock.h"
#include "res/bg/map/SnowBlock.h"
#include "res/bg/map/CrackedBlock.h"

#include "map.h"
#include "pino.h"

/* local defs */
#define SEA_LEVEL_RATE 30 // frame for a +1 tile
#define SEA_UP    0
#define SEA_DOWN  1

// holds the entire map state (interesting flags like is it an enemy, what kind of block etc)
uint8_t current_map_state[MAP_COLS*(32/BLOCK_HEIGHT_TILES)] = {EMPTY};
// holds the y value (ground level) for every col, always starts at 0,0 regardless of scroll
uint8_t current_map_height[MAP_COLS] = {0};

// sea state helpers
uint8_t sea_motion;         // SEA_UP / SEA_DOWN
uint8_t current_sea_y_tile; // current level (top tile)
uint8_t last_sea_y_tile;    // previous frame level
uint8_t sea_frame_count;    // to increase +1 level

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
  uint8_t platform_block_height  = new_world_blocks_y + 1;  // number of solid blocks in that col (or solid height of the col)
  // get a new block for every block of the col (from the bottom)
  // we have to go for more than COL_HEIGHT (based on the new block y)
  // cause eventually we'll shift the whole col DOWN according to the STEP_HEIGHT and new_world_blocks_y (we have to guarantee to overwrite every "old" tile)
  // this does mean that we potentially draw out of bounds for higher levels but its not necessarily relevant since you dont see it
  // also its easy to add an if() to not draw them if it becomes an issue
  // is it the best way to do it? idk but its what i thought of
  uint8_t shift_offset = new_world_blocks_y*STEP_HEIGHT_OFFSET_TILES; // offset DOWN based on the y value (shift assembled column down X tiles)
  // ceiling(ratio gap/tilesH) to know how many tiles needed to cover the post-shift hole
  uint8_t height_to_cover = COL_HEIGHT + (LEFTOVER_TILES+shift_offset + BLOCK_HEIGHT_TILES - 1)/BLOCK_HEIGHT_TILES;
  uint8_t col_has_cloud = 0; // at most one cloud per col
  for(uint8_t block_i=0; block_i<height_to_cover; block_i++){
    uint8_t tile_y = STARING_TILE_BLOCK_Y - (block_i*BLOCK_HEIGHT_TILES); // subtract one block height every entry (stack)

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

    set_bkg_based_tiles(target_x_tile, tile_y + shift_offset, BLOCK_WIDTH_TILES, BLOCK_HEIGHT_TILES, block_tile, MAP_TILES_START);

    // update current map state with the new col
    // x_coord = target_x_tile/BLOCK_WIDTH_TILES
    // y_coord = block_i
    if      (block_tile == SnowBlock    ) { current_map_state[block_i*MAP_ROWS + (target_x_tile/BLOCK_WIDTH_TILES)] = SOLID;     }
    else if (block_tile == CrackedBlock ) { current_map_state[block_i*MAP_ROWS + (target_x_tile/BLOCK_WIDTH_TILES)] = BREAKABLE; }
    else if (block_tile == FishBlock    ) { current_map_state[block_i*MAP_ROWS + (target_x_tile/BLOCK_WIDTH_TILES)] = FISH;      }
    else                                  { current_map_state[block_i*MAP_ROWS + (target_x_tile/BLOCK_WIDTH_TILES)] = EMPTY;     }
  }

  // update height for col
  current_map_height[target_x_tile/BLOCK_WIDTH_TILES] = new_world_blocks_y;

  return new_world_blocks_y;
}

// initializes world_height
// load initial visible map (col by col)
// which in turns also inits the current map state
// also initializes sea level/state
void init_map(void){
  /* sea level */
  // set_win_based_tiles(0, 0, SeaMapWidth, SeaMapHeight, SeaMap, MAP_TILES_START);
  current_sea_y_tile = DEVICE_SCREEN_HEIGHT-1;
  sea_frame_count = 0;

  /* bkg stuff */
  SHOW_BKG;
  uint8_t previous_col_world_y = 0; //  0 == (SCREENHEIGHT - 16)px == (SCREENHEIGHT/8 - 2)tiles
  set_bkg_data(MAP_TILES_START, 22, BGTiles);
  for(uint8_t x_tile=0; x_tile<DEVICE_SCREEN_BUFFER_WIDTH;x_tile+=4){
    previous_col_world_y = generate_column_for(x_tile, previous_col_world_y);
  }
}

// draws sea at its current level (tile)
void draw_sea(void){
  if(last_sea_y_tile != current_sea_y_tile){
    // sea needs to move separately and be above the bkg
    // 1. move it to the correct position
    move_win(7, current_sea_y_tile*8);
    // 2. paint over the rows above the last one
    // this is so we overwrite the previous sea/window height HUD which would otherwise show after move_win
    set_win_based_tiles(0, 0, DEVICE_SCREEN_WIDTH, DEVICE_SCREEN_HEIGHT - current_sea_y_tile, SeaMap, MAP_TILES_START);
    last_sea_y_tile = current_sea_y_tile;
    SHOW_WIN;
  }
}

// updates sea framecount/level based on pino state
// JUMPING: -1, IDLE: +1
// @param pino_state JUMPING/IDLE/KO
void update_sea(int8_t pino_state){
  switch (pino_state){
    case JUMPING:
      // reverse if jumped
      if(sea_motion == SEA_UP && current_sea_y_tile < DEVICE_SCREEN_HEIGHT-1){
        current_sea_y_tile++;
        sea_frame_count = 0;
      }
      sea_motion = SEA_DOWN;
      break;
    case IDLE:
      sea_motion = SEA_UP;
      sea_frame_count++;
      if(sea_frame_count == SEA_LEVEL_RATE){
        current_sea_y_tile--;
        sea_frame_count = 0;
      }
    case KO:
    default:
      break;
  }

}

// scrolls everything right SCROLL_SPEED px
// also generates next column of the map if necessary
void update_camera(void){
  scroll_bkg(SCROLL_SPEED, 0); // SCX_REG++

  // "hidden" portion of the map -> sliding window to replace with new tiles
  uint8_t window_blocks_nr = MAP_COLS - (SCREENWIDTH/BLOCK_WIDTH_PX);

  // xtodo: this should just be x % window_blocks_nr*BLOCK_WIDTH_PX but thats too slow to calc i think
  // if(SCX_REG == 0 || SCX_REG == window_blocks_nr*BLOCK_WIDTH_PX || SCX_REG == 2*window_blocks_nr*BLOCK_WIDTH_PX){
  if((SCX_REG & (BLOCK_WIDTH_PX-1)) == 0){
    // get number of last block in view
    uint8_t reference_x_block = (SCX_REG + SCREENWIDTH - BLOCK_WIDTH_PX)/BLOCK_WIDTH_PX & (MAP_COLS-1);
    // get its height
    uint8_t reference_world_y = current_map_height[reference_x_block];
    // set first of the to-do blocks
    uint8_t next_x_block = (reference_x_block + 1) & (MAP_COLS-1);

    // for(uint8_t block_i=0; block_i<window_blocks_nr; block_i++){
    //   uint8_t x_tile = next_x_block*BLOCK_WIDTH_TILES + block_i*BLOCK_WIDTH_TILES;
    //   reference_world_y = generate_column_for(x_tile, reference_world_y);
    // }

    uint8_t x_tile = next_x_block*BLOCK_WIDTH_TILES;
    reference_world_y = generate_column_for(x_tile, reference_world_y);
  }
}