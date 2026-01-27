#include <gb/gb.h>
#include <gb/metasprites.h>

#include "map.h"
#include "res/bg/BGTiles.h"

#define ANI_FISH_TILE_LOC   0x0E  // location in VRAM of the animatable tile for fish
#define ANI_FISH_TILE_0_IDX 0x0E  // index in BGTiles for frame 0 of the animation
#define ANI_FISH_TILE_1_IDX 0x11  // index in BGTiles for frame 1 of the animation

uint8_t current_fish_ani_frame;
const unsigned char fish_frames[] = { ANI_FISH_TILE_0_IDX, ANI_FISH_TILE_1_IDX };

void animate_bg(void){
  /* FISHES */
  uint8_t this_frame_idx = fish_frames[current_fish_ani_frame];
  const unsigned char* BGTile_frame_reference = &BGTiles[this_frame_idx * 16]; 
  set_bkg_data(ANI_FISH_TILE_LOC, 1, BGTile_frame_reference); // 1 tile (idx) is 16B
  // set next frame
  current_fish_ani_frame = (current_fish_ani_frame+1) & 1; // limit to fish_frames size
}

void init_animations(void){
  current_fish_ani_frame = 0;
}