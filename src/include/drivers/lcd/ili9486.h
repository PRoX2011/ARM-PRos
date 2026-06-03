#ifndef DRIVERS_LCD_ILI9486_H
#define DRIVERS_LCD_ILI9486_H

#include <stdint.h>

#define LCD_WIDTH  480
#define LCD_HEIGHT 320

int  lcd_init(void);
int  lcd_is_ready(void);
void lcd_set_enabled(int on);

void lcd_set_font_height(unsigned h);
void lcd_set_fg(uint32_t rgb);
void lcd_set_bg(uint32_t rgb);

void lcd_clear(uint32_t rgb);
void lcd_fill_rect(int x, int y, int w, int h, uint32_t rgb);
void lcd_putc(char c);
void lcd_puts(const char *s);
void lcd_put_char_at(int x, int y, char c, uint32_t fg, uint32_t bg);
void lcd_put_string_at(int x, int y, const char *s, uint32_t fg, uint32_t bg);

#endif
