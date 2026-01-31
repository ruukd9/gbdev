#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>

#include "map.h"
#include "pino.h"

#include "res/sprites/Pino.h"

/* local defs */
// sprite number
#define PINO_SPRITE_NR 0

// palette stuff
const uint8_t PINO_PAL = OAMF_CGB_PAL0;
const palette_color_t Pino_palette[] = {
  RGB(11, 24, 31), RGB_WHITE, RGB(5, 8, 31), RGB(3, 3, 3)
};
const metasprite_t Pino_metasprite[] = {
  // start(n) = n-1, 0,0 = reference -> top right
  { .dtile=0,   .dx=0,  .dy=0 },    // TL
  { .dtile=2,   .dx=8,  .dy=0 },    // TML
  { .dtile=4,   .dx=-8, .dy=16 },   // BL
  { .dtile=6,   .dx=8,  .dy=0 },    // BML
  { .dtile=8,   .dx=8,  .dy=-16 },  // TMR
  { .dtile=10,  .dx=8,  .dy=0 },    // TR
  { .dtile=12,  .dx=-8, .dy=16 },   // BMR
  { .dtile=14,  .dx=8,  .dy=0 },    // BR
  METASPR_TERM
};

uint8_t pino_block_x_position; // current block x position (0->7)
uint8_t pino_tile_y_position;  // this is in tiles cause height calculations is done in tiles
// i have no clue why but the metasprite is drawn with 0,0 in these coordinates
const int8_t sprite_draw_px_offset[] = { -DEVICE_SPRITE_PX_OFFSET_X, -DEVICE_SPRITE_PX_OFFSET_Y };

// detect button presses
uint8_t current_btn;
uint8_t last_btn;

// draws pino at his currently saved x position
static void draw_pino(void){
  uint8_t pino_pos_height = current_map_height[pino_block_x_position]; // level height of the block (0,1,2...->MAX_WORLD_Y)
  uint8_t pino_pos_ground_tile_y = STARING_TILE_BLOCK_Y - (pino_pos_height*STEP_HEIGHT); // y coord (tiles) of the ground for the 1st map block
  pino_tile_y_position = pino_pos_ground_tile_y - BLOCK_HEIGHT_TILES; // y coord (tiles) from where to start drawing him
  move_metasprite_ex(
    Pino_metasprite,
    0, PINO_PAL, PINO_SPRITE_NR,
    pino_block_x_position*BLOCK_WIDTH_PX - sprite_draw_px_offset[0] - SCX_REG, // -SCX cause we want him still in the world frame
    pino_tile_y_position*8 - sprite_draw_px_offset[1]
  );
}

void jump_forward(void){
  // ... do some animation in between probably

  // land him on the next block (pos++)
  pino_block_x_position = (pino_block_x_position+1) < MAP_COLS ? (pino_block_x_position+1) : 0;
}

void init_pino(void){
  SPRITES_8x16; SHOW_SPRITES;

  pino_block_x_position = 1; // init starting position
  set_sprite_data(0, 16, Pino);
  draw_pino();
}

uint8_t is_pino_ok(void){
  // whats the first visible block?
  uint8_t scx_zero_block = SCX_REG / BLOCK_WIDTH_PX;
  uint8_t last_valid_block = (scx_zero_block + (MAP_COLS-1)) & (MAP_COLS-1);
  // is he after it?
  return pino_block_x_position >= scx_zero_block || pino_block_x_position < last_valid_block;
}

// handles pino's state (movement, position, state ecc)
// returns 0 if pino is ok, 1 otherwise
uint8_t update_pino(void){
  if(!is_pino_ok()) return 1;

  // poll joypad status
  last_btn = current_btn;
  current_btn = joypad();

  if(!(current_btn & last_btn) && (current_btn & J_A)) jump_forward();

  // draw him where he is
  draw_pino();

  return 0;
}