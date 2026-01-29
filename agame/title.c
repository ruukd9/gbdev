#include <stdio.h>
#include <gb/gb.h>
#include <rand.h>

#include "res/fonts/FontTiles.h"
#include "res/bg/TitleMap.h"

#include "title.h"

void await_titlescreen(void){
  SHOW_BKG;

  // 1. load font tiles to bkg
  set_bkg_data(0, FONT_TILES_NB, FontTiles);
  set_bkg_tiles(0, 0, TitleMapWidth, TitleMapHeight, TitleMap);

  // 2. rand seed
  // https://github.com/gbdk-2020/gbdk-2020/blob/develop/gbdk-lib/examples/gb/rand/src/rand.c
  uint16_t seed;
  waitpad(J_START);
  seed = DIV_REG;
  waitpadup();
  seed |= (UWORD)DIV_REG << 8;

  initrand(seed);
}