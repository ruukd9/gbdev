#include <gb/gb.h>
#include <gb/cgb.h>
#include <gb/metasprites.h>

#include "res/sprites/Pino.h"

#include "res/bg/map/EmptyBlock.h"

#include "map.h"
#include "pino.h"

/* local defs */
// sprite number
#define PINO_SPRITE_NR 0
// animation duration (after how many frame to switch)
#define IDLE_FRAME_RATE 10
// idle metasprites
#define PINO_IDLE_0    0
#define PINO_IDLE_1   16
#define PINO_JUMPING  32
#define PINO_DUCKING  48

#define PINO_STARTING_BLOCK 1

static uint8_t is_pino_ok(void);
static void set_idle(void);
static void set_jumping(void);
static void set_moved(void);

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
uint8_t pino_block_x_position; // current block x position (0->7) to check for events and placement on the map
uint8_t pino_pos_ground_tile_y; // current ground tile (y) useful to have here to not recalc every time
// i have no clue why but the metasprite is drawn with 0,0 in these coordinates
const int8_t sprite_draw_px_offset[] = { -DEVICE_SPRITE_PX_OFFSET_X, -DEVICE_SPRITE_PX_OFFSET_Y };

uint8_t pino_current_state;
uint8_t pino_current_sprite;
uint8_t pino_frame_counter;
// frame-by-frame y offset while jumping, should be BLOCK_WIDTH_PX/SCROLL_SPEED long
const uint8_t jumping_curve_map[] = {
  8, 10, 12, 16, 18, 20, 20, 24,
  24, 20, 20, 18, 16, 12, 10, 8
};

uint16_t pino_score;
uint16_t pino_fishes;

// detect button presses
uint8_t current_btn;
uint8_t last_btn;

// draws pino at his currently saved xy position in his current state (IDLE/JUMPING)
void draw_pino(void){
  SPRITES_8x16; SHOW_SPRITES;

  // in case we need to adjust the position during jump
  uint8_t pino_x_jump_offset = 0;
  uint8_t pino_y_jump_offset = 0;

  switch (pino_current_state){
    case JUMPING:
      // check for how long we've been jumping
      if(pino_frame_counter < BLOCK_WIDTH_PX/SCROLL_SPEED){
        // we havent traveled enough -> x++ (and y-- for jump)
        pino_x_jump_offset = pino_frame_counter*SCROLL_SPEED;
        pino_y_jump_offset = jumping_curve_map[pino_frame_counter];
        // update overworld (SCX++) along with him
        update_camera();
      }else{
        // traveled a block -> go back to idle + update block pos
        set_idle();
        pino_block_x_position = (pino_block_x_position+1) < MAP_COLS ? (pino_block_x_position+1) : 0;
        set_moved();
      }

      break;
    case DUCKING:
      // nothing special?
      break;
    case IDLE:
    default:
      // idle animation sprite tile swap
      if(pino_frame_counter >= IDLE_FRAME_RATE){
        pino_frame_counter = 0;
        pino_current_sprite = pino_current_sprite == PINO_IDLE_0 ? PINO_IDLE_1 : PINO_IDLE_0;
      }

      break;
  }

  // calc base position (based on block grid, disregarding state)
  uint8_t pino_pos_height = current_map_height[pino_block_x_position]; // level height of the block (0,1,2...->MAX_WORLD_Y)
  pino_pos_ground_tile_y = STARING_TILE_BLOCK_Y - (pino_pos_height*STEP_HEIGHT); // y coord (tiles) of the ground for the 1st map block
  uint8_t pino_tile_y_position = pino_pos_ground_tile_y - BLOCK_HEIGHT_TILES; // y coord (tiles) from where to start drawing him

  uint8_t pino_x_px = pino_block_x_position*BLOCK_WIDTH_PX;
  uint8_t pino_y_px = pino_tile_y_position*8;

  pino_x_px += pino_x_jump_offset;
  pino_y_px -= pino_y_jump_offset;

  pino_metasprites_nr = move_metasprite_ex(
    Pino_metasprite, pino_current_sprite,
    PINO_PAL, PINO_SPRITE_NR,
    pino_x_px - sprite_draw_px_offset[0] - SCX_REG, // -SCX cause we want him still in the world frame
    pino_y_px - sprite_draw_px_offset[1]
  );

  hide_sprites_range(pino_metasprites_nr, MAX_HARDWARE_SPRITES);

  pino_frame_counter++;
}

/*************************************/
/*           STATE UPDATE            */
/*************************************/
// sets current state/frame as IDLE
static void set_idle(void){
  pino_current_state = IDLE;
  pino_current_sprite = PINO_IDLE_0;
  pino_frame_counter = 0;
}
// sets current state/frame as JUMPING
static void set_jumping(void){
  pino_current_state = JUMPING;
  pino_current_sprite = PINO_JUMPING;
  pino_frame_counter = 0;
}
// sets current state/frame as DUCKING
static void set_ducking(void){
  pino_current_state = DUCKING;
  pino_current_sprite = PINO_DUCKING;
  pino_frame_counter = 0;
}
// stuff to do after a successful move
static void set_moved(void){
  pino_score++;
  uint8_t pino_world_height = current_map_height[pino_block_x_position]; // the block on which he stands
  uint8_t lookup_index = (pino_world_height+1)*MAP_COLS + pino_block_x_position;
  if(current_map_state[lookup_index] == FISH){
    // eat the fish
    // block becomes empty -> update map/state
    set_bkg_based_tiles(
      pino_block_x_position*BLOCK_WIDTH_TILES,
      STARING_TILE_BLOCK_Y - (pino_world_height*STEP_HEIGHT) - BLOCK_HEIGHT_TILES,
      BLOCK_WIDTH_TILES,
      BLOCK_HEIGHT_TILES,
      EmptyBlock,
      MAP_TILES_START
    );
    // ... maybe some "nom" sprite appears here?
    current_map_state[lookup_index] = EMPTY;
    pino_fishes++;
  }
}

// checks whether pino is OK
// @returns 1 if yes, 0 if no
static uint8_t is_pino_ok(void){
  /* todo: its probably bad to punish going forward, need to think of something else */
  // // whats the first visible block?
  // uint8_t scx_zero_block = SCX_REG / BLOCK_WIDTH_PX;
  // uint8_t distance_from_edge = (pino_block_x_position - scx_zero_block) & (MAP_COLS-1);
  // // is he still in view?
  // return distance_from_edge < SCREENWIDTH/BLOCK_WIDTH_PX;

  // y goes backwards..
  return pino_current_state == JUMPING || pino_pos_ground_tile_y < current_sea_y_tile;
}

/**************************************/
/*             MANAGEMENT             */
/**************************************/
// sets sprite data + initial state
void init_pino(void){
  set_sprite_data(0, 64, Pino);

  // init starting position
  pino_block_x_position = PINO_STARTING_BLOCK;
  // init state
  set_idle();
  // init score
  pino_score = 0;
  pino_fishes = 0;
}

// handles pino's state (movement, position, state ecc)
// @returns current pino state (JUMPING/IDLE/KO)
int8_t update_pino(void){
  if(!is_pino_ok()) return KO;
  // if hes jumping hes not doing anything else
  if(pino_current_state == JUMPING) return JUMPING;

  // poll joypad status
  last_btn = current_btn;
  current_btn = joypad();
  // apply changes if needed
  if      (current_btn & J_B)                               { set_ducking();  }
  else if ((current_btn & J_A) && (current_btn ^ last_btn)) { set_jumping();  }
  else if (last_btn & J_B)                                  { set_idle();     }

  return pino_current_state;
}