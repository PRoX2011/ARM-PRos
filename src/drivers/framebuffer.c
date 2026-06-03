#include <drivers/framebuffer.h>
#include <drivers/mailbox.h>
#include <stddef.h>
#include <stdint.h>

extern const unsigned char font8x8_basic[128][8];
extern const unsigned char font8x16_basic[128][16];
extern const uint8_t font4x6_basic[128][3];

#define MARGIN_X 16
#define MARGIN_Y 16

#define TAG_SETPHYWH   0x00048003u
#define TAG_SETVIRTWH  0x00048004u
#define TAG_SETDEPTH   0x00048005u
#define TAG_SETPXORD   0x00048006u
#define TAG_GETPITCH   0x00040008u
#define TAG_ALLOCBUF   0x00040001u

static volatile uint32_t mbox[36] __attribute__((aligned(16)));

static uint8_t *volatile fb_base;
static unsigned fb_width;
static unsigned fb_height;
static unsigned fb_pitch;
static unsigned font_h = 8;
static unsigned font_w = 8;
#define FB_FG_DEFAULT 0xFFe8e8e8u

static uint32_t fb_fg = FB_FG_DEFAULT;
static uint32_t fb_bg = 0xFF282420u;
static int fb_ready;

static uint32_t bus_to_arm_phys(uint32_t bus)
{
	return bus & 0x3FFFFFFFu;
}

static void fill_rect_px(int x, int y, int w, int h, uint32_t color)
{
	if (!fb_base || x < 0 || y < 0)
		return;
	for (int row = 0; row < h; row++) {
		volatile uint32_t *line = (volatile uint32_t *)((uintptr_t)fb_base +
								(unsigned)(y + row) * fb_pitch);
		for (int col = 0; col < w; col++) {
			if ((unsigned)(x + col) < fb_width)
				line[x + col] = color;
		}
	}
}

static void erase_cell_at(int px, int py)
{
	fill_rect_px(px, py, (int)font_w, (int)font_h, fb_bg);
}

static int get_pixel(unsigned char c, int row, int col)
{
	if (font_h == 16) {
		return (font8x16_basic[c][row] >> (7 - col)) & 1;
	} else if (font_h == 6) {
		int idx = row / 2;
		int nibble = (row & 1) ? (font4x6_basic[c][idx] & 0x0F)
		                       : (font4x6_basic[c][idx] >> 4);
		return (nibble >> col) & 1;
	}
	return (font8x8_basic[c][row] >> col) & 1;
}
static void blit_glyph(int px, int py, unsigned char c)
{
	if (c > 127u)
		c = (unsigned char)'?';

	for (int row = 0; row < (int)font_h; row++) {
		volatile uint32_t *line = (volatile uint32_t *)((uintptr_t)fb_base +
								(unsigned)(py + row) * fb_pitch);
		for (int col = 0; col < (int)font_w; col++) {
			if ((unsigned)(px + col) < fb_width)
				line[px + col] = get_pixel(c, row, col) ? fb_fg : fb_bg;
		}
	}
}

static void scroll_one_line(void)
{
	if (!fb_base || fb_pitch == 0 || fb_height <= font_h)
		return;

	size_t move = (size_t)(fb_height - font_h) * fb_pitch;
	uint8_t *base = (uint8_t *)fb_base;
	for (size_t i = 0; i < move; i++)
		base[i] = base[i + (size_t)font_h * fb_pitch];

	for (unsigned y = fb_height - font_h; y < fb_height; y++) {
		volatile uint32_t *line =
		    (volatile uint32_t *)((uintptr_t)fb_base + y * fb_pitch);
		for (unsigned x = 0; x < fb_pitch / 4u; x++)
			line[x] = fb_bg;
	}
}

static int pen_x = MARGIN_X;
static int pen_y = MARGIN_Y;

static void newline(void)
{
	pen_x = MARGIN_X;
	pen_y += (int)font_h;
	if ((unsigned)(pen_y + (int)font_h) > fb_height) {
		scroll_one_line();
		pen_y -= (int)font_h;
	}
}

int fb_init(unsigned width, unsigned height, unsigned depth_bits)
{
	fb_ready = 0;
	fb_base = 0;

	mbox[0] = 30u * 4u;
	mbox[1] = 0u;
	mbox[2] = TAG_SETPHYWH;
	mbox[3] = 8u;
	mbox[4] = 0u;
	mbox[5] = width;
	mbox[6] = height;
	mbox[7] = TAG_SETVIRTWH;
	mbox[8] = 8u;
	mbox[9] = 0u;
	mbox[10] = width;
	mbox[11] = height;
	mbox[12] = TAG_SETDEPTH;
	mbox[13] = 4u;
	mbox[14] = 0u;
	mbox[15] = depth_bits;
	mbox[16] = TAG_SETPXORD;
	mbox[17] = 4u;
	mbox[18] = 0u;
	mbox[19] = 1u; /* RGB */
	mbox[20] = TAG_GETPITCH;
	mbox[21] = 4u;
	mbox[22] = 0u;
	mbox[23] = 0u;
	mbox[24] = TAG_ALLOCBUF;
	mbox[25] = 8u;
	mbox[26] = 0u;
	mbox[27] = 16u; /* alignment */
	mbox[28] = 0u;
	mbox[29] = 0u;

	if (!mbox_call(mbox))
		return 0;

	fb_pitch = mbox[23];
	if (fb_pitch == 0 || (fb_pitch % 4u) != 0u)
		return 0;

	uint32_t gpu_ptr = mbox[27];
	if (gpu_ptr == 0u)
		return 0;

	fb_base = (uint8_t *)(uintptr_t)bus_to_arm_phys(gpu_ptr);
	fb_width = mbox[5];
	fb_height = mbox[6];
	fb_ready = 1;

	pen_x = MARGIN_X;
	pen_y = MARGIN_Y;
	return 1;
}

int fb_is_ready(void)
{
	return fb_ready;
}

void fb_set_font_height(unsigned h)
{
	if (h == 6 || h == 8 || h == 16) {
		font_h = h;
		font_w = (h == 6) ? 4 : 8;
	}
}

unsigned fb_get_width(void)  { return fb_width; }
unsigned fb_get_height(void) { return fb_height; }
unsigned fb_get_pitch(void)  { return fb_pitch; }
unsigned fb_get_font_h(void) { return font_h; }
unsigned fb_get_font_w(void) { return font_w; }

void fb_set_fg(uint32_t rgba)
{
	fb_fg = rgba;
}

void fb_reset_fg(void)
{
	fb_fg = FB_FG_DEFAULT;
}

void fb_set_bg(uint32_t rgba)
{
	fb_bg = rgba;
}

void fb_clear(void)
{
	if (!fb_ready || !fb_base)
		return;

	for (unsigned y = 0; y < fb_height; y++) {
		volatile uint32_t *line =
		    (volatile uint32_t *)((uintptr_t)fb_base + y * fb_pitch);
		for (unsigned x = 0; x < fb_pitch / 4u; x++)
			line[x] = fb_bg;
	}
	pen_x = MARGIN_X;
	pen_y = MARGIN_Y;
}

void fb_putc(int c)
{
	if (!fb_ready || !fb_base)
		return;

	unsigned char uc = (unsigned char)c;

	if (uc == '\r') {
		pen_x = MARGIN_X;
		return;
	}
	if (uc == '\n') {
		newline();
		return;
	}
	if (uc == '\b') {
		if (pen_x > MARGIN_X) {
			pen_x -= (int)font_w;
			erase_cell_at(pen_x, pen_y);
		}
		return;
	}
	if (uc == '\t') {
		int tab = 8 * (int)font_w;
		pen_x = MARGIN_X + ((pen_x - MARGIN_X + tab) / tab) * tab;
		if ((unsigned)pen_x + font_w > fb_width)
			newline();
		return;
	}

	if ((unsigned)pen_x + font_w > fb_width - MARGIN_X)
		newline();

	blit_glyph(pen_x, pen_y, uc);
	pen_x += (int)font_w;
}

void fb_puts(const char *s)
{
	if (!s)
		return;
	for (int i = 0; s[i] != '\0'; i++)
		fb_putc((unsigned char)s[i]);
}

void fb_set_pen(int x, int y)
{
	pen_x = x;
	pen_y = y;
}

int fb_get_pen_x(void) { return pen_x; }
int fb_get_pen_y(void) { return pen_y; }

void fb_fill_rect(int x, int y, int w, int h, uint32_t color)
{
	fill_rect_px(x, y, w, h, color);
}

void fb_put_char_at(int px, int py, char c, uint32_t fg, uint32_t bg_color)
{
	if (!fb_ready || !fb_base)
		return;
	unsigned char uc = (unsigned char)c;
	if (uc > 127u)
		uc = (unsigned char)'?';

	for (int row = 0; row < (int)font_h; row++) {
		volatile uint32_t *line = (volatile uint32_t *)((uintptr_t)fb_base +
								(unsigned)(py + row) * fb_pitch);
		for (int col = 0; col < (int)font_w; col++) {
			if ((unsigned)(px + col) < fb_width)
				line[px + col] = get_pixel(uc, row, col) ? fg : bg_color;
		}
	}
}

void fb_put_string_at(int px, int py, const char *s, uint32_t fg, uint32_t bg_color)
{
	if (!fb_ready || !fb_base || !s)
		return;
	int x = px;
	for (int i = 0; s[i] != '\0'; i++) {
		fb_put_char_at(x, py, s[i], fg, bg_color);
		x += (int)font_w;
	}
}
