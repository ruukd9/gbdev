#include <gb/gb.h>
#include <gb/metasprites.h>
#include <gb/cgb.h>
#include <rand.h>

#include "variables.h"
#include "res/bg/BGTiles.h"
#include "res/bg/BGMap.h"

const palette_color_t Bg_palette[] = {
  RGB(28, 28, 31), RGB(19, 19, 25), RGB(1, 8, 13), RGB_BLACK
};

void main(){
  SHOW_BKG;

  // set colors if supported
  // IMPORTANT !! needs -Wm-yc flag in build
  if(_cpu == CGB_TYPE){
    set_bkg_palette(0, 1, Bg_palette);
  }else{
    // set DMG sprite palette
    OBP0_REG = DMG_PALETTE(DMG_LITE_GRAY, DMG_WHITE, DMG_DARK_GRAY, DMG_BLACK);
  }

  // setup
  initrand(DIV_REG);
  set_bkg_data(MAP_TILES_START, 36, BGTiles);
  set_bkg_based_tiles(0, 0, BGMapWidth, BGMapHeight, BGMap, MAP_TILES_START);

  uint8_t scroll_timer  = 0;

  while(1){
    scroll_timer++;
    if(scroll_timer == SCROLL_SPEED){
      update_camera();
      scroll_timer = 0;
    }

    wait_vbl_done();
  }
}