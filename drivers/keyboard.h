#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "../include/types.h"

void keyboard_init(void);
char keyboard_getchar(void);   /* blocks until a key is available */
bool keyboard_has_input(void);

#endif
