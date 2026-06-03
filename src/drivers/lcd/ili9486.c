// ==================================================================
// ARM-PRos - ILI9486 SPI LCD (480x320) driver for ARM-PRos kernel
// Copyright (C) 2026 PRoX2011
// ==================================================================

#include <drivers/lcd/ili9486.h>
#include <drivers/spi.h>
#include <drivers/timer.h>
#include <gpio.h>

extern const unsigned char font8x8_basic[128][8];
extern const unsigned char font8x16_basic[128][16];
extern const uint8_t font4x6_basic[128][3];

#define DC_PIN  24
#define RST_PIN 25

#define LCD_MADCTL 0x28
#define SPI_CLOCK_DIVIDER 8

#define MARGIN 4

static uint16_t shadow[LCD_HEIGHT][LCD_WIDTH];
static int ready;
static int present;

static unsigned font_h = 8;
static unsigned font_w = 8;
static uint16_t cur_fg;
static uint16_t cur_bg;
static int pen_x;
static int pen_y;

static inline void dc_command(void) { *GPCLR0 = (1u << DC_PIN); }
static inline void dc_data(void)    { *GPSET0 = (1u << DC_PIN); }

static uint16_t rgb565_be(uint32_t rgb)
{
	unsigned r = rgb & 0xFFu;
	unsigned g = (rgb >> 8) & 0xFFu;
	unsigned b = (rgb >> 16) & 0xFFu;
	uint16_t v = (uint16_t)(((r & 0xF8u) << 8) | ((g & 0xFCu) << 3) | (b >> 3));
	return (uint16_t)((v >> 8) | (v << 8));
}

static void write_command(uint8_t cmd)
{
	uint8_t word[2] = { 0x00, cmd };
	dc_command();
	spi0_write(word, 2);
}

static void write_data(uint8_t data)
{
	uint8_t word[2] = { 0x00, data };
	dc_data();
	spi0_write(word, 2);
}

static void set_window(int x0, int y0, int x1, int y1)
{
	write_command(0x2A);
	write_data((uint8_t)(x0 >> 8));
	write_data((uint8_t)x0);
	write_data((uint8_t)(x1 >> 8));
	write_data((uint8_t)x1);

	write_command(0x2B);
	write_data((uint8_t)(y0 >> 8));
	write_data((uint8_t)y0);
	write_data((uint8_t)(y1 >> 8));
	write_data((uint8_t)y1);

	write_command(0x2C);
}

static void blit_region(int x, int y, int w, int h)
{
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if (x + w > LCD_WIDTH)  w = LCD_WIDTH - x;
	if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
	if (w <= 0 || h <= 0)
		return;

	set_window(x, y, x + w - 1, y + h - 1);
	dc_data();
	for (int row = 0; row < h; row++)
		spi0_write((const uint8_t *)&shadow[y + row][x], (size_t)w * 2u);
}

static void hw_reset(void)
{
	gpio_set_function(DC_PIN, GPIO_OUTPUT);
	gpio_set_function(RST_PIN, GPIO_OUTPUT);

	*GPSET0 = (1u << RST_PIN);
	delay_ms(20);
	*GPCLR0 = (1u << RST_PIN);
	delay_ms(50);
	*GPSET0 = (1u << RST_PIN);
	delay_ms(150);
}

static void send_gamma(uint8_t reg, const uint8_t *g)
{
	write_command(reg);
	for (int i = 0; i < 15; i++)
		write_data(g[i]);
}

static void init_panel(void)
{
	static const uint8_t gamma_pos[15] = {
		0x0F, 0x1F, 0x1C, 0x0C, 0x0F, 0x08, 0x48, 0x98,
		0x37, 0x0A, 0x13, 0x04, 0x11, 0x0D, 0x00
	};
	static const uint8_t gamma_neg[15] = {
		0x0F, 0x32, 0x2E, 0x0B, 0x0D, 0x05, 0x47, 0x75,
		0x37, 0x06, 0x10, 0x03, 0x24, 0x20, 0x00
	};

	write_command(0xB0);
	write_data(0x00);

	write_command(0x11);
	delay_ms(250);

	write_command(0x3A);
	write_data(0x55);

	write_command(0xC2);
	write_data(0x44);

	write_command(0xC5);
	write_data(0x00);
	write_data(0x00);
	write_data(0x00);
	write_data(0x00);

	send_gamma(0xE0, gamma_pos);
	send_gamma(0xE1, gamma_neg);
	send_gamma(0xE2, gamma_neg);

	write_command(0x36);
	write_data(LCD_MADCTL);

	write_command(0x11);
	delay_ms(150);

	write_command(0x29);
}

static void flush_full(void)
{
	set_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
	dc_data();
	spi0_write((const uint8_t *)shadow, sizeof(shadow));
}

static int glyph_pixel(unsigned char c, int row, int col)
{
	if (font_h == 16)
		return (font8x16_basic[c][row] >> (7 - col)) & 1;
	if (font_h == 6) {
		int idx = row / 2;
		int nibble = (row & 1) ? (font4x6_basic[c][idx] & 0x0F)
				       : (font4x6_basic[c][idx] >> 4);
		return (nibble >> col) & 1;
	}
	return (font8x8_basic[c][row] >> col) & 1;
}

static void draw_glyph(int px, int py, unsigned char c, uint16_t fg, uint16_t bg)
{
	if (c > 127u)
		c = (unsigned char)'?';

	for (unsigned row = 0; row < font_h; row++) {
		int y = py + (int)row;
		if (y < 0 || y >= LCD_HEIGHT)
			continue;
		for (unsigned col = 0; col < font_w; col++) {
			int x = px + (int)col;
			if (x < 0 || x >= LCD_WIDTH)
				continue;
			shadow[y][x] = glyph_pixel(c, (int)row, (int)col) ? fg : bg;
		}
	}
}

static void scroll_up(void)
{
	for (int y = 0; y < LCD_HEIGHT - (int)font_h; y++)
		for (int x = 0; x < LCD_WIDTH; x++)
			shadow[y][x] = shadow[y + (int)font_h][x];

	for (int y = LCD_HEIGHT - (int)font_h; y < LCD_HEIGHT; y++)
		for (int x = 0; x < LCD_WIDTH; x++)
			shadow[y][x] = cur_bg;

	flush_full();
}

static void newline(void)
{
	pen_x = MARGIN;
	pen_y += (int)font_h;
	if (pen_y + (int)font_h > LCD_HEIGHT) {
		scroll_up();
		pen_y -= (int)font_h;
	}
}

int lcd_init(void)
{
	cur_fg = rgb565_be(0xE8E8E8u);
	cur_bg = rgb565_be(0x202428u);

	spi0_init(SPI_CLOCK_DIVIDER);
	hw_reset();
	init_panel();

	present = 1;
	ready = 1;
	lcd_clear(0x202428u);
	return 1;
}

int lcd_is_ready(void)
{
	return ready;
}

void lcd_set_enabled(int on)
{
	ready = (on && present) ? 1 : 0;
}

void lcd_set_font_height(unsigned h)
{
	if (h == 6 || h == 8 || h == 16) {
		font_h = h;
		font_w = (h == 6) ? 4 : 8;
	}
}

void lcd_set_fg(uint32_t rgb)
{
	cur_fg = rgb565_be(rgb);
}

void lcd_set_bg(uint32_t rgb)
{
	cur_bg = rgb565_be(rgb);
}

void lcd_clear(uint32_t rgb)
{
	if (!ready)
		return;

	cur_bg = rgb565_be(rgb);
	for (int y = 0; y < LCD_HEIGHT; y++)
		for (int x = 0; x < LCD_WIDTH; x++)
			shadow[y][x] = cur_bg;

	flush_full();
	pen_x = MARGIN;
	pen_y = MARGIN;
}

void lcd_fill_rect(int x, int y, int w, int h, uint32_t rgb)
{
	if (!ready)
		return;

	uint16_t color = rgb565_be(rgb);
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if (x + w > LCD_WIDTH)  w = LCD_WIDTH - x;
	if (y + h > LCD_HEIGHT) h = LCD_HEIGHT - y;
	if (w <= 0 || h <= 0)
		return;

	for (int row = 0; row < h; row++)
		for (int col = 0; col < w; col++)
			shadow[y + row][x + col] = color;

	blit_region(x, y, w, h);
}

void lcd_putc(char c)
{
	if (!ready)
		return;

	unsigned char uc = (unsigned char)c;

	if (uc == '\r') {
		pen_x = MARGIN;
		return;
	}
	if (uc == '\n') {
		newline();
		return;
	}
	if (uc == '\b') {
		if (pen_x > MARGIN) {
			pen_x -= (int)font_w;
			for (unsigned row = 0; row < font_h; row++)
				for (unsigned col = 0; col < font_w; col++)
					shadow[pen_y + (int)row][pen_x + (int)col] = cur_bg;
			blit_region(pen_x, pen_y, (int)font_w, (int)font_h);
		}
		return;
	}
	if (uc == '\t') {
		int tab = 8 * (int)font_w;
		pen_x = MARGIN + ((pen_x - MARGIN + tab) / tab) * tab;
		if (pen_x + (int)font_w > LCD_WIDTH)
			newline();
		return;
	}

	if (pen_x + (int)font_w > LCD_WIDTH - MARGIN)
		newline();

	draw_glyph(pen_x, pen_y, uc, cur_fg, cur_bg);
	blit_region(pen_x, pen_y, (int)font_w, (int)font_h);
	pen_x += (int)font_w;
}

void lcd_puts(const char *s)
{
	if (!ready || !s)
		return;
	for (int i = 0; s[i] != '\0'; i++)
		lcd_putc(s[i]);
}

void lcd_put_char_at(int x, int y, char c, uint32_t fg, uint32_t bg)
{
	if (!ready)
		return;
	draw_glyph(x, y, (unsigned char)c, rgb565_be(fg), rgb565_be(bg));
	blit_region(x, y, (int)font_w, (int)font_h);
}

void lcd_put_string_at(int x, int y, const char *s, uint32_t fg, uint32_t bg)
{
	if (!ready || !s)
		return;

	uint16_t f = rgb565_be(fg);
	uint16_t b = rgb565_be(bg);
	int cx = x;
	for (int i = 0; s[i] != '\0'; i++) {
		draw_glyph(cx, y, (unsigned char)s[i], f, b);
		cx += (int)font_w;
	}
	blit_region(x, y, cx - x, (int)font_h);
}
