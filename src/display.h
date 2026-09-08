#ifndef __DISPLAY_H
#define __DISPLAY_H

#define DISPLAY_CHAR_MU 0x80
#define DISPLAY_MU "\x80"

void display_init(void);
void display_clear(void);
void display_at_row(unsigned char row, const char *text, unsigned char offset);
void display_text(const char *text);

#endif
