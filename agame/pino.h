#include <gb/cgb.h>

#ifndef __pino_h_INCLUDE
#define __pino_h_INCLUDE

// states
#define IDLE    0
#define JUMPING 1

extern const palette_color_t Pino_palette[4];

extern uint8_t pino_current_state;

void init_pino(void);
uint8_t update_pino(void);

#endif