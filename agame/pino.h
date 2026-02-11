#include <gb/cgb.h>

#ifndef __pino_h_INCLUDE
#define __pino_h_INCLUDE

// states
#define KO     -1
#define IDLE    0
#define JUMPING 1

extern const palette_color_t Pino_palette[4];

extern uint16_t pino_score;

void init_pino(void);
int8_t update_pino(void);
void draw_pino(void);

#endif