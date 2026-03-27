#ifndef _DRIVERS_KEYBOARD_H
#define _DRIVERS_KEYBOARD_H

void keyboard_init(void);
unsigned char keyboard_get_scancode(void);
char keyboard_getc(void);

#endif
