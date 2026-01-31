#include <stdio.h>
#include <gb/gb.h>
#include <rand.h>

#include "res/bg/title/TitleTiles.h"
#include "res/bg/title/TitleMap.h"
#include "res/bg/gg/GameOverTiles.h"
#include "res/bg/gg/GameOverMap.h"

#include "title.h"

#define IDLE_TITLE_FRAMES 45

// draw/animate titlescreen
// wait for start button to init rand seed and return to caller
void show_titlescreen(void){
  // reset camera
  SCX_REG = 0;
  // remove sprites
  HIDE_SPRITES;

  SHOW_BKG;

  // set starting tiles
  set_bkg_data(0, 16, TitleTiles);
  set_bkg_tiles(0, 0, TitleMapWidth, TitleMapHeight, TitleMap);

  // to animate the "press start"
  const unsigned char BlankMask[16*8] = {0x00};
  uint16_t idle_frames = IDLE_TITLE_FRAMES;
  uint8_t should_show_text = 1;

  while(1){
    // animate "press start"
    if(idle_frames == IDLE_TITLE_FRAMES){
      idle_frames = 0;
      should_show_text ? 
        set_bkg_data(0, 16, TitleTiles)
        : set_bkg_data(8, 8, BlankMask);

      should_show_text = !should_show_text;
    }

    if(joypad() & J_START){
      // init rand seed
      // https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/rand/src/rand.c
      uint16_t seed;
      waitpad(J_START);
      seed = DIV_REG;
      waitpadup();
      seed |= (UWORD)DIV_REG << 8;
      initrand(seed);
      break;
    }

    idle_frames++;
    vsync(); // wait next frame
  }
}

// draw/animate end screen
// wait for start or A button return to caller
void show_endscreen(void){
  // reset camera
  SCX_REG = 0;
  // remove sprites
  HIDE_SPRITES;

  SHOW_BKG;

  // load ending tiles to bkg
  set_bkg_data(0, 17, GameOverTiles);
  set_bkg_tiles(0, 0, GameOverMapWidth, GameOverMapHeight, GameOverMap);

  // ... maybe some recap/score in the future

  waitpad(J_A | J_START);
}