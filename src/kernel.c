#include <stdint.h>
#include <stddef.h>
#include "limine.h"

__attribute__((used, section(".requests")))
static volatile uint64_t base_revision[] = {
    LIMINE_BASE_REVISION(3)
};

__attribute__((used, section(".requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST_ID,
    .revision = 0
};

__attribute__((used, section(".requests_start")))
static volatile uint64_t requests_start[] = LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".requests_end")))
static volatile uint64_t requests_end[] = LIMINE_REQUESTS_END_MARKER;

static uint32_t *fb;
static uint64_t width, height, pitch;

static void putpixel(int x, int y, uint32_t c) {
    if (x < 0 || y < 0 || (uint64_t)x >= width || (uint64_t)y >= height) return;
    fb[(uint64_t)y * (pitch / 4) + (uint64_t)x] = c;
}

static void rect(int x, int y, int w, int h, uint32_t c) {
    if (w <= 0 || h <= 0) return;
    for (int yy = y; yy < y + h; ++yy)
        for (int xx = x; xx < x + w; ++xx)
            putpixel(xx, yy, c);
}

/* Small built-in 5x7 font. Each glyph is encoded as five columns. */
static const uint8_t font[37][5] = {
 {0x7e,0x11,0x11,0x11,0x7e},{0x7f,0x49,0x49,0x49,0x36},{0x3e,0x41,0x41,0x41,0x22},
 {0x7f,0x41,0x41,0x22,0x1c},{0x7f,0x49,0x49,0x49,0x41},{0x7f,0x09,0x09,0x09,0x01},
 {0x3e,0x41,0x49,0x49,0x7a},{0x7f,0x08,0x08,0x08,0x7f},{0x00,0x41,0x7f,0x41,0x00},
 {0x20,0x40,0x41,0x3f,0x01},{0x7f,0x08,0x14,0x22,0x41},{0x7f,0x40,0x40,0x40,0x40},
 {0x7f,0x02,0x04,0x02,0x7f},{0x7f,0x04,0x08,0x10,0x7f},{0x3e,0x41,0x41,0x41,0x3e},
 {0x7f,0x09,0x09,0x09,0x06},{0x3e,0x41,0x51,0x21,0x5e},{0x7f,0x09,0x19,0x29,0x46},
 {0x46,0x49,0x49,0x49,0x31},{0x01,0x01,0x7f,0x01,0x01},{0x3f,0x40,0x40,0x40,0x3f},
 {0x1f,0x20,0x40,0x20,0x1f},{0x7f,0x20,0x18,0x20,0x7f},{0x63,0x14,0x08,0x14,0x63},
 {0x07,0x08,0x70,0x08,0x07},{0x61,0x51,0x49,0x45,0x43},
 {0,0,0,0,0},{0x7e,0x81,0x81,0x81,0x7e},{0x00,0x82,0xff,0x80,0},
 {0xe2,0x91,0x91,0x91,0x8e},{0x42,0x89,0x89,0x89,0x72},{0x3e,0x41,0x41,0x41,0x22},
 {0xff,0x09,0x09,0x09,0x01},{0x7e,0x81,0x89,0x89,0x72},{0xff,0x08,0x08,0xff,0},
 {0x7f,0x49,0x49,0x49,0x41},{0x7f,0x49,0x49,0x49,0x36}
};

static int idx(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= '0' && c <= '9') return 27 + (c - '0');
    if (c == ' ') return 26;
    return 26;
}

static void text(int x, int y, const char *s, uint32_t c, int scale) {
    for (; *s; ++s) {
        int n = idx(*s);
        for (int col=0; col<5; ++col)
            for (int row=0; row<7; ++row)
                if (font[n][col] & (1u << row))
                    rect(x + col*scale, y + row*scale, scale, scale, c);
        x += 6*scale;
    }
}

static void desktop(void) {
    rect(0,0,(int)width,(int)height,0x001b2430);
    rect(0,0,(int)width,(int)height-48,0x0026323f);
    rect(0,(int)height-48,(int)width,48,0x00151c24);

    /* desktop windows */
    rect(80,70,500,330,0x000f141a);
    rect(80,70,500,32,0x002d3946);
    rect(80,102,500,298,0x00151b21);
    rect(610,100,300,230,0x000f141a);
    rect(610,100,300,32,0x002d3946);
    rect(610,132,300,198,0x00151b21);

    text(96,79,"MYOS",0x00ffffff,3);
    text(98,125,"WELCOME TO MYOS",0x00e8edf2,2);
    text(98,160,"DESKTOP READY",0x0079c2ff,2);
    text(628,109,"TERMINAL",0x00ffffff,2);
    text(628,150,"MYOS 64",0x0094d4ff,2);
    text(628,175,"SYSTEM READY",0x00d8e7d8,2);
    text(18,(int)height-35,"START",0x00ffffff,2);
    text(130,(int)height-35,"FILES",0x00d9e2ea,2);
    text(220,(int)height-35,"NOTEPAD",0x00d9e2ea,2);
    text(340,(int)height-35,"CALC",0x00d9e2ea,2);
}

void kmain(void) {
    if (!LIMINE_BASE_REVISION_SUPPORTED(base_revision)) for (;;) __asm__ volatile("hlt");

    if (!framebuffer_request.response ||
        framebuffer_request.response->framebuffer_count < 1) {
        for (;;) __asm__ volatile("hlt");
    }

    struct limine_framebuffer *f = framebuffer_request.response->framebuffers[0];
    fb = (uint32_t *)f->address;
    width = f->width;
    height = f->height;
    pitch = f->pitch;

    desktop();
    for (;;) __asm__ volatile("hlt");
}
