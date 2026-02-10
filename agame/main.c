#include <gb/gb.h>
#include <gb/cgb.h>

#include "title.h"
#include "map.h"
#include "animations.h"
#include "pino.h"

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
  /* SETUP */
  // init items
  init_map();
  init_pino();
  init_numbers_font();
  // init score
  // will always increase by (BLOCK_WIDTH_PX/SCROLL_SPEED) every time a jump is successful
  uint16_t score = 0;

  int8_t pino_state;

  while(1){
    /* DRAW ITEMS */
    draw_sea();
    draw_pino();  // also moves scx forward if jumping
    draw_number(0, 18, (score*SCROLL_SPEED)/BLOCK_WIDTH_PX, 1);
    animate_bg();

    /* UPDATE STATE */
    pino_state = update_pino();
    update_sea(pino_state); // idle = goes UP, jumping = goes DOWN
    if(pino_state == KO) break;
    if(pino_state == JUMPING) score++;

    vsync(); // await frame
  }

  show_endscreen();
  await_start();
}