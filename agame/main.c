#include <gb/gb.h>
#include <gb/cgb.h>
#include <rand.h>

#include "title.h"
#include "map.h"
#include "animations.h"
#include "pino.h"

/* local defs */
// how many frames for +1px?
#define SCROLL_SPEED 2

static void await_start(void);
static void game_loop(void);

const palette_color_t Bg_palette[] = {
  // v1
  // RGB(28, 28, 31), RGB(19, 19, 25), RGB(1, 8, 13), RGB_BLACK
  // v2
  // RGB(28, 31, 28), RGB(14, 18, 31), RGB(7, 11, 15), RGB(7, 7, 7)
  // v3
  RGB_WHITE, RGB(11, 24, 31), RGB(5, 8, 31), RGB(3, 3, 3)
};

void main(void){
  // set colors if supported
  // IMPORTANT !! needs -Wm-yc flag in build
  if(_cpu == CGB_TYPE){
    set_bkg_palette(0, 1, Bg_palette);
    set_sprite_palette(0, 1, Pino_palette);
  }else{
    // set DMG palette
    OBP0_REG = DMG_PALETTE(DMG_WHITE, DMG_LITE_GRAY, DMG_DARK_GRAY, DMG_BLACK);
  }

  show_titlescreen();
  // wait for start to begin
  await_start();
}

// inits the random seed and then starts the main game loop
static void await_start(void){
  // init rand seed
  // https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/rand/src/rand.c
  uint16_t seed;
  waitpad(J_START);
  seed = DIV_REG;
  waitpadup();
  seed |= (UWORD)DIV_REG << 8;
  initrand(seed);

  game_loop();
}

static void game_loop(void){
  // setup
  init_map();
  init_pino();

  uint8_t scroll_timer  = 0;

  while(1){
    scroll_timer++;
    if(scroll_timer == SCROLL_SPEED){
      // update overworld
      update_camera();
      // bg tiles animation (fish and stuff)
      // every 8px (px per tiles, sperimentally a decent framerate)
      if((SCX_REG & 7) == 0) animate_bg();

      scroll_timer = 0;
    }

    if(update_pino()) break;

    vsync();
  }

  // reset camera
  SCX_REG = 0;

  show_endscreen();
  // wait for start press to go again
  await_start();
}