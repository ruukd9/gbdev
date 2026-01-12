#include <gb/gb.h>
#include "Inputs.h"

#define BTN_A       0
#define BTN_B       1
#define BTN_U       2
#define BTN_R       3
#define BTN_D       4
#define BTN_L       5
#define BTN_START   6
#define BTN_SELECT  7


void set_active_sprite(uint8_t sprite, uint8_t tile){
  // tell sprite "0" to display sprite starting from pos <tile>
  // since sprite size is 8x8 this loads the entire sprite <tile>
  set_sprite_tile(sprite, tile);
  // show sprite "0" in center of screen
  move_sprite(sprite, SCREENWIDTH/2 + 4, SCREENHEIGHT/2 + 4);
}

void main(){
  // set sprites as 8x8 pixels
  SPRITES_8x8;

  // from tile 0 load 8 tiles of "inputs"
  set_sprite_data(0, 8, Inputs);

  SHOW_SPRITES;

  uint8_t is_pressing = 0;

  while(1){
    switch(joypad()) {
      case J_LEFT:
        if(!is_pressing){
          is_pressing = 1;
          set_active_sprite(0, BTN_L);
        }
        break;
      case J_RIGHT:
        if(!is_pressing){
          is_pressing = 1;
          set_active_sprite(0, BTN_R);
        }
        break;
      case J_UP:
        if(!is_pressing){
          is_pressing = 1;
          set_active_sprite(0, BTN_U);
        }
        break;
      case J_DOWN:
        if(!is_pressing){
          is_pressing = 1;
          set_active_sprite(0, BTN_D);
        }
        break;
      case J_START:
        if(!is_pressing){
          is_pressing = 1;
          set_active_sprite(0, BTN_START);
        }
        break;
      case J_SELECT:
        if(!is_pressing){
          is_pressing = 1;
          set_active_sprite(0, BTN_SELECT);
        }
        break;
      case J_A:
        if(!is_pressing){
          is_pressing = 1;
          set_active_sprite(0, BTN_A);
        }
        break;
      case J_B:
        if(!is_pressing){
          is_pressing = 1;
          set_active_sprite(0, BTN_B);
        }
        break;
      default:
        is_pressing = 0;
        hide_sprite(0);
    }

    // delay(100);
  }
}