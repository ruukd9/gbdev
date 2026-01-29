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
// curren block x position (0->7)
uint8_t pino_x_position;
// i have no clue why but the metasprite is drawn with 0,0 in these coordinates
const int8_t sprite_draw_tiles_offset[] = { -1, -2 };

// draws pino at his currently saved x position
static void draw_pino(void){
  uint8_t pino_pos_height = current_map_height[pino_x_position]; // level height of the block
  uint8_t pino_pos_ground_tile_y = STARING_TILE_BLOCK_Y - (pino_pos_height*STEP_HEIGHT); // y coord (tiles) of the ground for the 1st map block
  uint8_t pino_block_y_pos = pino_pos_ground_tile_y - BLOCK_HEIGHT_TILES;
  set_sprite_data(0, 16, Pino);
  move_metasprite_ex(
    Pino_metasprite,
    0, PINO_PAL, PINO_SPRITE_NR,
    (pino_x_position - sprite_draw_tiles_offset[0])*8 - SCX_REG, // -SCX cause we want him still in the world frame
    (pino_block_y_pos - sprite_draw_tiles_offset[1])*8
  );
}

void init_pino(void){
  SPRITES_8x16; SHOW_SPRITES;

  pino_x_position = 0; // init starting position
  draw_pino();
}

// handles pino's state (movement, position, state ecc)
// returns 0 if pino is ok, 1 otherwise
uint8_t update_pino(void){
  // is he out of the screen?
  if((pino_x_position*BLOCK_WIDTH_PX) + BLOCK_WIDTH_PX < SCX_REG) return 1;

  // draw him where he is
  draw_pino();

  return 0;
}