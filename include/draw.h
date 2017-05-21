#ifndef DRAW_H
#define DRAW_H

#include <stdint.h>
#include "types.h"

#define BYTES_PER_PIXEL 3
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 320

#define SCREEN_SIZE (BYTES_PER_PIXEL * SCREEN_WIDTH * SCREEN_HEIGHT)

#define RGB(r,g,b) (r<<24|b<<16|g<<8|r)

#define FB_TOP_SIZE           (400 * 240 * 3)
#define FB_BOT_SIZE           (320 * 240 * 3)

#define VRAM_BASE             (0x18000000)
#define FB_BASE_PA            (VRAM_BASE)
#define FB_TOP_LEFT1          (FB_BASE_PA)
#define FB_TOP_LEFT2          (FB_TOP_LEFT1  + FB_TOP_SIZE)
#define FB_TOP_RIGHT1         (FB_TOP_LEFT2  + FB_TOP_SIZE)
#define FB_TOP_RIGHT2         (FB_TOP_RIGHT1 + FB_TOP_SIZE)
#define FB_BOT_1              (FB_TOP_RIGHT2 + FB_TOP_SIZE)
#define FB_BOT_2              (FB_BOT_1      + FB_BOT_SIZE)

#define TOP_SCREEN0 (u8*)(FB_TOP_LEFT1)
#define TOP_SCREEN1 (u8*)(FB_TOP_LEFT2)
#define BOT_SCREEN0 (u8*)(FB_BOT_1)
#define BOT_SCREEN1 (u8*)(FB_BOT_2)
extern int current_y;

void ClearScreen(unsigned char *screen, int color);
void DrawCharacter(unsigned char *screen, int character, int x, int y, int color, int bgcolor);
void DrawHex(unsigned char *screen, unsigned int hex, int x, int y, int color, int bgcolor);
void DrawString(unsigned char *screen, const char *str, int x, int y, int color, int bgcolor);
void DrawStringF(int x, int y, const char *format, ...);
void DrawHexWithName(unsigned char *screen, const char *str, unsigned int hex, int x, int y, int color, int bgcolor);

void Debug(const char *format, ...);

void ClearBot();

#endif
