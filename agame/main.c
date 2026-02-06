#include <gb/gb.h>
#include <gb/cgb.h>

#include "title.h"
#include "map.h"
#include "animations.h"
#include "pino.h"

/* local defs */
// main loop phases
#define MAP_PHASE       0
#define ANIMATION_PHASE 1
// how many frames to idle animations for (aka animate every N)
#define IDLE_ANI_FRAMES 15

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

  await_start();
}

// shows the titlescreen and then starts the main loop
static void await_start(void){
  show_titlescreen();
  game_loop();
}

static void game_loop(void){
  // setup
  init_map();
  init_pino();

  // animation helper
  uint8_t framecount = IDLE_ANI_FRAMES;
  // distribute load across frames
  // uint8_t game_phase = MAP_PHASE;

  while(1){
    vsync(); // await frame

    // if(game_phase == ANIMATION_PHASE){
      // bg tiles animation (fish and stuff)
      if(framecount == IDLE_ANI_FRAMES){
        framecount = 0;
        animate_bg();
      }

      framecount++;
    // }else if(game_phase == MAP_PHASE){
      // try update player pos
      if(update_pino()) break;

      update_sea();
    // }

    // next render phase
    // game_phase = !game_phase;
  }

  show_endscreen();
  await_start();
}