#include <gb/gb.h>
#include "Penguin.h"

void main(){
  // set sprites as 8x16 pixels (max value)
  SPRITES_8x16;

  // from tile 0 to 4 of "penguin"
  set_sprite_data(0, 4, Penguin);
  // tell sprite "0" to display sprite staring from pos 0
  // since sprite size is 8x16 this loads sprite 0 as tiles 0 and 1 (left half)
  set_sprite_tile(0, 0);
  // tell sprite "1" to display sprite staring from pos 2
  // since sprite size is 8x16 this loads sprite 2 as tiles 2 and 3 (right half)
  set_sprite_tile(1, 2);
  // sprite "0" in x=50, y=50
  move_sprite(0, 50, 50);
  // sprite "1" in x=50 (+ sprite width), y=50
  move_sprite(1, 50 + 8, 50);

  SHOW_SPRITES;
}