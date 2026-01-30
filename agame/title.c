#include <stdio.h>
#include <gb/gb.h>

#include "res/bg/title/TitleTiles.h"
#include "res/bg/title/TitleMap.h"
#include "res/bg/gg/GameOverTiles.h"
#include "res/bg/gg/GameOverMap.h"

#include "title.h"

void show_titlescreen(void){
  SHOW_BKG;

  // load title tiles to bkg
  set_bkg_data(0, 16, TitleTiles);
  set_bkg_tiles(0, 0, TitleMapWidth, TitleMapHeight, TitleMap);

  // ... maybe some animation in the future
}

void show_endscreen(void){
  SHOW_BKG;

  // load ending tiles to bkg
  set_bkg_data(0, 20, GameOverTiles);
  set_bkg_tiles(0, 0, GameOverMapWidth, GameOverMapHeight, GameOverMap);

  // ... maybe some recap/score in the future
}