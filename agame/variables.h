#define COL_HEIGHT                5 // 32x32 blocks in a col
#define STEP_HEIGHT_OFFSET_TILES  2 // tile height of the overlap between two different (consecutive) height levels
#define WORLD_MAX_Y               3 // max height of the solid blocks in a col, NEEDS to be < COL_HEIGHT to make sense

#define BLOCK_WIDTH_PX     32
#define BLOCK_WIDTH_TILES   4
#define BLOCK_HEIGHT_PX    32
#define BLOCK_HEIGHT_TILES  4

#define FONT_TILES_NB 50

#define SCROLL_SPEED 2

#define MAP_TILES_START 0

// flags
#define EMPTY     0
#define SOLID     1
#define BREAKABLE 2
#define ENEMY     3
#define FISH      4