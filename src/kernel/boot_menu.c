// ==================================================================
// ARM-PRos - boot menu for ARM-PRos kernel
// Copyright (C) 2026 PRoX2011
// ==================================================================

#include <kernel/boot_menu.h>
#include <drivers/framebuffer.h>
#include <drivers/lcd/ili9486.h>
#include <drivers/input.h>
#include <drivers/uart.h>
#include <drivers/usb/keyboard.h>
#include <stdint.h>
#include <string.h>

#define MENU_BG        0xFF1A1A2Eu
#define MENU_BOX_BG    0xFF2D2D44u
#define MENU_TEXT      0xFFCCCCCCu
#define MENU_SEL_BG    0xFF4444AAu
#define MENU_SEL_TEXT  0xFFFFFFFFu
#define MENU_TITLE     0xFFFFFFFFu
#define MENU_BORDER    0xFF555577u
#define MENU_FOOTER    0xFF888899u
#define MENU_LABEL     0xFF9999BBu

#define BOOT_W      LCD_WIDTH
#define BOOT_H      LCD_HEIGHT
#define BOOT_FONT_H 16
#define FONT_W      8

typedef struct {
	const char *label;
	unsigned w;
	unsigned h;
} video_mode_t;

typedef struct {
	const char *label;
	unsigned h;
} font_size_t;

static const video_mode_t modes[] = {
    { "128x64",    128,  64   },
    { "320x240",   320,  240  },
    { "640x480",   640,  480  },
    { "800x600",   800,  600  },
    { "1024x768",  1024, 768  },
    { "1280x720",  1280, 720  },
    { "1600x900",  1600, 900  },
	{ "1600x1200", 1600, 1200 },
    { "1920x1080", 1920, 1080 },
};

#define NUM_MODES (sizeof(modes) / sizeof(modes[0]))

static const font_size_t fonts[] = {
	{ "4x6",  6  },
	{ "8x8",  8  },
	{ "8x16", 16 },
};
#define NUM_FONTS 3

#define PAGE_RESOLUTION 0
#define PAGE_FONT       1

static void px_fill_rect(int x, int y, int w, int h, uint32_t color)
{
	fb_fill_rect(x, y, w, h, color);
	if (lcd_is_ready())
		lcd_fill_rect(x, y, w, h, color);
}

static void px_string(int x, int y, const char *s, uint32_t fg, uint32_t bg)
{
	fb_put_string_at(x, y, s, fg, bg);
	if (lcd_is_ready())
		lcd_put_string_at(x, y, s, fg, bg);
}

static void draw_box(int bx, int by, int bw, int bh)
{
	px_fill_rect(0, 0, BOOT_W, BOOT_H, MENU_BG);
	px_fill_rect(bx, by, bw, bh, MENU_BOX_BG);
	px_fill_rect(bx, by, bw, 1, MENU_BORDER);
	px_fill_rect(bx, by + bh - 1, bw, 1, MENU_BORDER);
	px_fill_rect(bx, by, 1, bh, MENU_BORDER);
	px_fill_rect(bx + bw - 1, by, 1, bh, MENU_BORDER);
}

typedef struct {
	int box_x;
	int box_y;
	int box_w;
	int fh;
	int items_y;
} menu_layout_t;

static menu_layout_t layout_for(int num_items)
{
	menu_layout_t L;
	L.fh = BOOT_FONT_H;
	L.box_w = 400;
	int box_h = (num_items + 7) * L.fh;
	L.box_x = (BOOT_W - L.box_w) / 2;
	L.box_y = (BOOT_H - box_h) / 2;
	L.items_y = L.box_y + 4 * L.fh;
	return L;
}

static void draw_item(const menu_layout_t *L, int i, const char *label, int selected)
{
	int iy = L->items_y + i * L->fh;
	uint32_t fg = selected ? MENU_SEL_TEXT : MENU_TEXT;
	uint32_t bg = selected ? MENU_SEL_BG : MENU_BOX_BG;
	px_fill_rect(L->box_x + 4, iy, L->box_w - 8, L->fh, bg);
	px_string(L->box_x + 24, iy, label, fg, bg);
}

static void draw_page_full(const menu_layout_t *L, const char *page_label,
                            int num_items, const char *const *labels, int selected)
{
	int fh = L->fh;
	int box_x = L->box_x;
	int box_y = L->box_y;
	int box_w = L->box_w;
	int box_h = (num_items + 7) * fh;

	draw_box(box_x, box_y, box_w, box_h);

	const char *title = "ARM-PRos Boot Menu";
	int title_x = box_x + (box_w - (int)strlen(title) * FONT_W) / 2;
	int title_y = box_y + fh;
	px_string(title_x, title_y, title, MENU_TITLE, MENU_BOX_BG);

	px_fill_rect(box_x + 8, title_y + fh + 4, box_w - 16, 1, MENU_BORDER);

	int label_y = title_y + fh * 2;
	px_string(box_x + 16, label_y, page_label, MENU_LABEL, MENU_BOX_BG);

	for (int i = 0; i < num_items; i++)
		draw_item(L, i, labels[i], i == selected);

	const char *footer = "Arrows: select   Enter: confirm";
	int footer_x = box_x + (box_w - (int)strlen(footer) * FONT_W) / 2;
	int footer_y = L->items_y + num_items * fh + fh;
	px_string(footer_x, footer_y, footer, MENU_FOOTER, MENU_BOX_BG);
}

static int run_page(const char *label, int num_items,
                    const char *const *labels, int default_sel)
{
	menu_layout_t L = layout_for(num_items);
	int selected = default_sel;
	draw_page_full(&L, label, num_items, labels, selected);

	for (;;) {
		int key = input_poll();
		if (key == KEY_NONE) {
			for (volatile int d = 0; d < 10000; d++)
				__asm__ volatile("yield");
			continue;
		}

		if (key == KEY_UP || key == 'k') {
			if (selected > 0) {
				draw_item(&L, selected, labels[selected], 0);
				selected--;
				draw_item(&L, selected, labels[selected], 1);
			}
		} else if (key == KEY_DOWN || key == 'j') {
			if (selected < num_items - 1) {
				draw_item(&L, selected, labels[selected], 0);
				selected++;
				draw_item(&L, selected, labels[selected], 1);
			}
		} else if (key == '\r' || key == '\n') {
			return selected;
		}
	}
}

static void graphical_menu(boot_menu_result_t *r)
{
	static const char *const out_labels[2] = { "HDMI only", "HDMI + SPI LCD" };
	int out = run_page("Display output:", 2, out_labels, 1);
	r->use_spi = (out == 1);
	if (!r->use_spi)
		lcd_set_enabled(0);

	const char *res_labels[NUM_MODES];
	for (int i = 0; i < NUM_MODES; i++)
		res_labels[i] = modes[i].label;

	int res = run_page("Select resolution:", NUM_MODES, res_labels, 2);
	r->width = modes[res].w;
	r->height = modes[res].h;

	const char *font_labels[NUM_FONTS];
	for (int i = 0; i < NUM_FONTS; i++)
		font_labels[i] = fonts[i].label;

	int font = run_page("Select font size:", NUM_FONTS, font_labels, 1);
	r->font_h = fonts[font].h;
}

static void uart_menu(boot_menu_result_t *r)
{
	uart_puts("\r\n--- Video Mode ---\r\n");
	for (int i = 0; i < NUM_MODES; i++) {
		uart_puts("  ");
		uart_putc((char)('1' + i));
		uart_puts(") ");
		uart_puts(modes[i].label);
		uart_puts("\r\n");
	}
	uart_puts("> ");
	char c = uart_getc();
	uart_putc(c);
	uart_puts("\r\n");

	int idx = c - '1';
	if (idx >= 0 && idx < NUM_MODES) {
		r->width = modes[idx].w;
		r->height = modes[idx].h;
	} else {
		r->width = 640;
		r->height = 480;
	}

	uart_puts("\r\n--- Font Size ---\r\n");
	for (int i = 0; i < NUM_FONTS; i++) {
		uart_puts("  ");
		uart_putc((char)('1' + i));
		uart_puts(") ");
		uart_puts(fonts[i].label);
		uart_puts("\r\n");
	}
	uart_puts("> ");
	c = uart_getc();
	uart_putc(c);
	uart_puts("\r\n");

	idx = c - '1';
	if (idx >= 0 && idx < NUM_FONTS)
		r->font_h = fonts[idx].h;
	else
		r->font_h = 8;
}

void boot_menu_run(boot_menu_result_t *result)
{
	result->width = 640;
	result->height = 480;
	result->font_h = 8;
	result->use_spi = 1;

	if (fb_init(BOOT_W, BOOT_H, 32u)) {
		fb_set_font_height(BOOT_FONT_H);
		fb_clear();
	}

	if (lcd_is_ready())
		lcd_set_font_height(BOOT_FONT_H);

	if (fb_is_ready() || lcd_is_ready())
		graphical_menu(result);
	else
		uart_menu(result);
}
