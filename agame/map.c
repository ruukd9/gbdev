#include <gb/gb.h>
#include <rand.h>
#include <gb/metasprites.h>
#include <gb/cgb.h>


#include "variables.h"

#include "res/bg/BGTiles.h"
#include "res/bg/BGMap.h"
#include "res/fonts/FontTiles.h"
#include "res/sprites/Postman.h"

#include "res/bg/EmptyBlock.h"
#include "res/bg/CloudBlock.h"
#include "res/bg/FishBlock.h"
#include "res/bg/SnowBlock.h"
#include "res/bg/CrackedBlock.h"

typedef struct map_position_t {
  int16_t x_px; int16_t y_px;
  uint8_t frame;
} map_position_t;

// holds the entire map state (interesting flags like is it an enemy, what kind of block etc)
uint8_t current_map_state[(32/BLOCK_WIDTH_TILES)*(32/BLOCK_HEIGHT_TILES)] = {EMPTY};

uint8_t world_is_inited = 0;

// current (last generated) world coordinates
uint8_t world_pixels_x = 0;
uint8_t world_blocks_x = 0;
uint8_t world_pixels_y = 0;
uint8_t world_blocks_y = 0;

uint8_t world_has_enemy = 0;
const uint8_t ENEMY_SPRITE_NR = 0;
const uint8_t PALETTE_ENEMY = OAMF_CGB_PAL0;
const palette_color_t Enemy_palette[] = {
  RGB_WHITE, RGB(30, 26, 11), RGB(10, 15, 24), RGB(7, 7, 7)
};
const metasprite_t Enemy_metasprite[] = {
  // reference is the origin -> we wanna place it at (W/2,H) aka where he stands
  // so top left corner is at x=-8, y=-16
  // also start(n) = n-1
  { .dtile=0, .dx=-8, .dy=-16 },  // L
  { .dtile=2, .dx=8,  .dy=0   },  // R
  METASPR_TERM
};
map_position_t Enemy_position = {
  // out of bounds init
  .x_px=-BLOCK_WIDTH_PX, .y_px=-BLOCK_HEIGHT_PX,
  .frame=ENEMY_FRAME_0
};

// B0 -> target_tile = 24
// B1 -> target_tile = 28
// B2 -> target_tile =  0
// B3 -> target_tile =  4
// B4 -> target_tile =  8
// B5 -> target_tile = 12
// B6 -> target_tile = 16
// B7 -> target_tile = 20
uint8_t next_target_tile(){
  // its not just +SCREEN bc at the start we generate one block more, so we have to generate the next next block
  return (uint8_t)(world_blocks_x*BLOCK_WIDTH_TILES + (DEVICE_SCREEN_WIDTH+BLOCK_WIDTH_TILES)) & 31; // == x % 32
}

// retruns whether or not is possible to have an enemy on the screen
uint8_t can_spawn_enemy(){
  // only possible if theres no other enemy on the map (would hit sprite limit) and we are not on the initial load (prevents headaches)
  return !world_has_enemy && world_is_inited;
}

// draws enemy sprite on screen and updates world enemy flag
// @param x_px pixels coordinate on the x axis
// @param y_px pixels coordinate on the y axis
void spawn_enemy_at(uint8_t x_px, uint8_t y_px){
  move_metasprite_ex(Enemy_metasprite, Enemy_position.frame, PALETTE_ENEMY, ENEMY_SPRITE_NR, x_px, y_px);
  Enemy_position.x_px = x_px;
  Enemy_position.y_px = y_px;
  Enemy_position.frame = (SCX_REG & 4) == 0 ? ENEMY_FRAME_0 : ENEMY_FRAME_1;
  // update world status
  world_has_enemy = 1;
}

void update_enemy(){
  if(world_has_enemy){
    Enemy_position.x_px--;
    move_metasprite_ex(Enemy_metasprite, Enemy_position.frame, PALETTE_ENEMY, ENEMY_SPRITE_NR, Enemy_position.x_px, Enemy_position.y_px);

    Enemy_position.frame = (SCX_REG & 4) == 0 ? ENEMY_FRAME_0 : ENEMY_FRAME_1;
  }
}

// creates and draws a new column on the map
// also updates current_map_state
// @param target_x_tiles tile reference on the x axis of the column to generate
void generate_column_for(uint8_t target_x_tile){
  // calc new world_blocks_y from current
  uint8_t new_world_blocks_y = 1; // default 1 as fallback in case something is wrong
  uint8_t r_height = (uint8_t)rand();

  if(world_blocks_y == 0){
    // can only grow or stay the same
    new_world_blocks_y = r_height < 64 ? 0 : 1;                       // 1 in 4 arbitrarily
  }else if(world_blocks_y == WORLD_MAX_Y){
    // can only shrink or stay the same
    new_world_blocks_y = r_height < 64 ? WORLD_MAX_Y : WORLD_MAX_Y-1; // 1 in 4 arbitrarily
  }else{
    // can safely do +-1 randomly
    new_world_blocks_y = r_height < 85 ?  world_blocks_y-1  // 1/3
      : (r_height < 170 ?                 world_blocks_y    // 1/3
      :                                   world_blocks_y+1  // 1/3
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
    // x_coord = target_x_tile/4
    // y_coord = block_i
    if      (block_tile == SnowBlock    ) { current_map_state[block_i*(32/BLOCK_HEIGHT_TILES) + (target_x_tile/4)] = SOLID;     }
    else if (block_tile == CrackedBlock ) { current_map_state[block_i*(32/BLOCK_HEIGHT_TILES) + (target_x_tile/4)] = BREAKABLE; }
    else if (block_tile == FishBlock    ) { current_map_state[block_i*(32/BLOCK_HEIGHT_TILES) + (target_x_tile/4)] = FISH;      }
    else{
      // block_tile == EmptyBlock
      if(block_i == platform_block_height && can_spawn_enemy()){ // maybe add some randomness to this?
        // this uses move_metasprite_x so the x,y need to be the ORIGIN of the sprite (NOT top left)
        spawn_enemy_at(target_x_tile*8 -8, (tile_y + y_offset)*8 + 32 + 16);
        current_map_state[block_i*(32/BLOCK_HEIGHT_TILES) + (target_x_tile/4)] = ENEMY;
        delay(5000);
      }else{
        current_map_state[block_i*(32/BLOCK_HEIGHT_TILES) + (target_x_tile/4)] = EMPTY;
      }
    }
  }
  world_blocks_y = new_world_blocks_y;
}

// initializes world_height
// load initial visible map (col by col)
// which in turns also inits the current map state with the visible columns (+1)
// also loads enemy sprite data
void init_map(){
  world_is_inited = 0;

  /* sprites stuff */
  SPRITES_8x16; SHOW_SPRITES;
  if(_cpu == CGB_TYPE) set_sprite_palette(0, 1, Enemy_palette);
  set_sprite_data(ENEMY_SPRITE_NR, 8, Postman);

  /* bkg stuff */
  SHOW_BKG;
  world_blocks_y = 0; //  0 == (SCREENHEIGHT - 16)px == (SCREENHEIGHT/8 - 2)tiles
  set_bkg_data(MAP_TILES_START, 17, BGTiles);
  for(uint8_t x_tile=0; x_tile<=DEVICE_SCREEN_WIDTH;x_tile+=4){ // <= so we load an extra one to buffer
    generate_column_for(x_tile);
  }

  world_is_inited = 1;
}

// scrolls everything right 1px
// also generates next column of the map if necessary
void update_camera(){
  scroll_bkg(1, 0);
  world_pixels_x++;
  if(world_pixels_x == BLOCK_WIDTH_PX){
    // travelled 1 block -> generate first out of bounds (next appearing)
    generate_column_for(next_target_tile());
    world_blocks_x = (world_blocks_x+1) & 31;
    world_pixels_x = 0;
  }

  update_enemy();
}