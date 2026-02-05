#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>

#include "map.h"
#include "pino.h"

#include "res/sprites/Pino.h"

/* local defs */
// sprite number
#define PINO_SPRITE_NR 0
// states
#define IDLE    0
#define JUMPING 1
#define LANDING 2
// animation duration (after how many frame to switch)
#define IDLE_FRAME_RATE 10
// idle metasprites
#define PINO_IDLE_0 0
#define PINO_IDLE_1 16

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

uint8_t pino_metasprites_nr; // hw sprites nr
uint8_t pino_block_x_position; // current block x position (0->7)
uint8_t pino_tile_y_position;  // this is in tiles cause height calculations is done in tiles
// i have no clue why but the metasprite is drawn with 0,0 in these coordinates
const int8_t sprite_draw_px_offset[] = { -DEVICE_SPRITE_PX_OFFSET_X, -DEVICE_SPRITE_PX_OFFSET_Y };

uint8_t pino_current_state = IDLE;
uint8_t pino_current_sprite = PINO_IDLE_0;
uint8_t pino_frame_counter = 0;

// detect button presses
uint8_t current_btn;
uint8_t last_btn;

// draws pino at his currently saved xy position in his current state (idle/jumping/landing)
static void draw_pino(void){
  if(pino_frame_counter >= IDLE_FRAME_RATE){
    pino_frame_counter = 0;
    pino_current_sprite = pino_current_sprite == PINO_IDLE_0 ? PINO_IDLE_1 : PINO_IDLE_0;
  }

  pino_metasprites_nr = move_metasprite_ex(
    Pino_metasprite, pino_current_sprite,
    PINO_PAL, PINO_SPRITE_NR,
    pino_block_x_position*BLOCK_WIDTH_PX - sprite_draw_px_offset[0] - SCX_REG, // -SCX cause we want him still in the world frame
    pino_tile_y_position*8 - sprite_draw_px_offset[1]
  );

  hide_sprites_range(pino_metasprites_nr, MAX_HARDWARE_SPRITES);

  pino_frame_counter++;
}

// update pino y coord based on current x
static void update_pino_height(void){
  uint8_t pino_pos_height = current_map_height[pino_block_x_position]; // level height of the block (0,1,2...->MAX_WORLD_Y)
  uint8_t pino_pos_ground_tile_y = STARING_TILE_BLOCK_Y - (pino_pos_height*STEP_HEIGHT); // y coord (tiles) of the ground for the 1st map block
  pino_tile_y_position = pino_pos_ground_tile_y - BLOCK_HEIGHT_TILES; // y coord (tiles) from where to start drawing him
}

void jump_forward(void){
  // ... do some animation in between probably

  // land him on the next block (pos++)
  pino_block_x_position = (pino_block_x_position+1) < MAP_COLS ? (pino_block_x_position+1) : 0;
}

void init_pino(void){
  SPRITES_8x16; SHOW_SPRITES;

  pino_block_x_position = 1; // init starting position
  set_sprite_data(0, 32, Pino);
}

uint8_t is_pino_ok(void){
  /* todo: its probably bad to punish going forward, need to think of something else */
  // // whats the first visible block?
  // uint8_t scx_zero_block = SCX_REG / BLOCK_WIDTH_PX;
  // uint8_t distance_from_edge = (pino_block_x_position - scx_zero_block) & (MAP_COLS-1);
  // // is he still in view?
  // return distance_from_edge < SCREENWIDTH/BLOCK_WIDTH_PX;

  return 1;
}

// handles pino's state (movement, position, state ecc)
// returns 0 if pino is ok, 1 otherwise
uint8_t update_pino(void){
  if(!is_pino_ok()) return 1;

  // poll joypad status
  last_btn = current_btn;
  current_btn = joypad();

  if(!(current_btn & last_btn) && (current_btn & J_A)){
    jump_forward();
    // update overworld
    update_camera();
  }

  // load new x/y
  update_pino_height();
  // draw him where he is
  draw_pino();

  return 0;
}