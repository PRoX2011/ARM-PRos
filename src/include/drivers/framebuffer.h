#ifndef DRIVERS_FRAMEBUFFER_H
#define DRIVERS_FRAMEBUFFER_H

#include <stdint.h>

int  fb_init(unsigned width, unsigned height, unsigned depth_bits);
int  fb_is_ready(void);
void fb_set_fg(uint32_t rgba);
void fb_reset_fg(void);
void fb_set_bg(uint32_t rgba);
void fb_clear(void);
void fb_putc(int c);
void fb_puts(const char *s);

void fb_set_font_height(unsigned h);

void fb_fill_rect(int x, int y, int w, int h, uint32_t color);
void fb_put_char_at(int px, int py, char c, uint32_t fg, uint32_t bg);
void fb_put_string_at(int px, int py, const char *s, uint32_t fg, uint32_t bg);

#endif
