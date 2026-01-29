#ifndef __map_h_INCLUDE
#define __map_h_INCLUDE

#define MAP_TILES_START 0
// block size
#define BLOCK_WIDTH_PX     32
#define BLOCK_WIDTH_TILES   4
#define BLOCK_HEIGHT_PX    32
#define BLOCK_HEIGHT_TILES  4
// world vars
#define COL_HEIGHT                6 // visible 32x32 blocks in a col
#define STEP_HEIGHT               1 // tile height of the overlap between two different (consecutive) height levels
#define WORLD_MAX_Y               4 // max height of the solid blocks in a col, NEEDS to be < COL_HEIGHT to make sense
#define STEP_HEIGHT_OFFSET_TILES (BLOCK_HEIGHT_TILES - STEP_HEIGHT)             // offset down for a single unit of "step" (height)
#define STARING_TILE_BLOCK_Y (DEVICE_SCREEN_HEIGHT - STEP_HEIGHT)  // top of the first (bottomest) visible block in the col
// flags
#define EMPTY     0
#define SOLID     1
#define BREAKABLE 2
#define ENEMY     3
#define FISH      4
// utils
#define MAP_COLS (32/BLOCK_WIDTH_TILES)
#define MAP_ROWS (32/BLOCK_HEIGHT_TILES)

extern uint8_t current_map_state[MAP_COLS*MAP_ROWS];
extern uint8_t current_map_height[MAP_COLS];

void init_map(void);
void update_camera(void);

#endif