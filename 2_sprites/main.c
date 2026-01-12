#include <gb/gb.h>
#include "Smile.h"

void main(){
  // set sprites as 8x8 pixels
  SPRITES_8x8;

  // from tile 0 load 1 tile of "smile"
  set_sprite_data(0, 1, Smile);
  // tell sprite "0" to display sprite starting from pos 0
  // since sprite size is 8x8 this loads the entire sprite 0 (tile 0)
  set_sprite_tile(0, 0);
  // sprite "0" in x=50, y=50
  move_sprite(0, 50, 50);

  SHOW_SPRITES;
}