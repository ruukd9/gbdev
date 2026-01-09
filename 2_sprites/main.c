#include <gb/gb.h>
#include "smile.h"

void main(){
  // set sprites as 8x8 pixels
  SPRITES_8x8;

  // from byte 0 to 8 of "smile"
  set_sprite_data(0, 8, smile);
  // label as "0" the tile in memory = 0 (the only one we have aka the sprite smile data)
  set_sprite_tile(0, 0);
  // tile "0" in x=50, y=50
  move_sprite(0, 50, 50);

  SHOW_SPRITES;
}