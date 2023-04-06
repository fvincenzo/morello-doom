//
// Copyright(C) 1993-1996 Id Software, Inc.
// Copyright(C) 2005-2014 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//	DOOM keyboard input via linux tty
//


#include <stdlib.h>
#ifdef MORELLO
#undef WNOHANG
#undef WUNTRACED
#endif
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/keyboard.h>
#include <linux/kd.h>
#include <linux/input.h>
#ifdef UNDEF_STD_KEYS
#undef KEY_ENTER
#undef KEY_TAB
#undef KEY_F1
#undef KEY_F2
#undef KEY_F3
#undef KEY_F4
#undef KEY_F5
#undef KEY_F6
#undef KEY_F7
#undef KEY_F8
#undef KEY_F9
#undef KEY_F10
#undef KEY_F11
#undef KEY_F12
#undef KEY_BACKSPACE
#undef KEY_PAUSE
#undef KEY_MINUS
#undef KEY_CAPSLOCK
#undef KEY_NUMLOCK
#undef KEY_HOME
#undef KEY_END
#endif

#include "config.h"
#include "deh_str.h"
#include "doomtype.h"
#include "doomkeys.h"
#include "i_joystick.h"
#include "i_system.h"
#include "i_swap.h"
#include "i_timer.h"
#include "i_video.h"
#include "i_scale.h"
#include "m_argv.h"
#include "m_config.h"
#include "m_misc.h"
#include "tables.h"
#include "v_video.h"
#include "w_wad.h"
#include "z_zone.h"

int vanilla_keyboard_mapping = 1;

// Is the shift key currently down?

static int shiftdown = 0;

// Lookup table for mapping AT keycodes to their doom keycode
static const char at_to_doom[] =
{
    /* 0x00 */ 0x00,
    /* 0x01 */ KEY_ESCAPE,
    /* 0x02 */ '1',
    /* 0x03 */ '2',
    /* 0x04 */ '3',
    /* 0x05 */ '4',
    /* 0x06 */ '5',
    /* 0x07 */ '6',
    /* 0x08 */ '7',
    /* 0x09 */ '8',
    /* 0x0a */ '9',
    /* 0x0b */ '0',
    /* 0x0c */ '-',
    /* 0x0d */ '=',
    /* 0x0e */ KEY_BACKSPACE,
    /* 0x0f */ KEY_TAB,
    /* 0x10 */ 'q',
    /* 0x11 */ 'w',
    /* 0x12 */ 'e',
    /* 0x13 */ 'r',
    /* 0x14 */ 't',
    /* 0x15 */ 'y',
    /* 0x16 */ 'u',
    /* 0x17 */ 'i',
    /* 0x18 */ 'o',
    /* 0x19 */ 'p',
    /* 0x1a */ '[',
    /* 0x1b */ ']',
    /* 0x1c */ KEY_ENTER,
    /* 0x1d */ KEY_FIRE, /* KEY_RCTRL, */
    /* 0x1e */ 'a',
    /* 0x1f */ 's',
    /* 0x20 */ 'd',
    /* 0x21 */ 'f',
    /* 0x22 */ 'g',
    /* 0x23 */ 'h',
    /* 0x24 */ 'j',
    /* 0x25 */ 'k',
    /* 0x26 */ 'l',
    /* 0x27 */ ';',
    /* 0x28 */ '\'',
    /* 0x29 */ '`',
    /* 0x2a */ KEY_RSHIFT,
    /* 0x2b */ '\\',
    /* 0x2c */ 'z',
    /* 0x2d */ 'x',
    /* 0x2e */ 'c',
    /* 0x2f */ 'v',
    /* 0x30 */ 'b',
    /* 0x31 */ 'n',
    /* 0x32 */ 'm',
    /* 0x33 */ ',',
    /* 0x34 */ '.',
    /* 0x35 */ '/',
    /* 0x36 */ KEY_RSHIFT,
    /* 0x37 */ KEYP_MULTIPLY,
    /* 0x38 */ KEY_LALT,
    /* 0x39 */ KEY_USE,
    /* 0x3a */ KEY_CAPSLOCK,
    /* 0x3b */ KEY_F1,
    /* 0x3c */ KEY_F2,
    /* 0x3d */ KEY_F3,
    /* 0x3e */ KEY_F4,
    /* 0x3f */ KEY_F5,
    /* 0x40 */ KEY_F6,
    /* 0x41 */ KEY_F7,
    /* 0x42 */ KEY_F8,
    /* 0x43 */ KEY_F9,
    /* 0x44 */ KEY_F10,
    /* 0x45 */ KEY_NUMLOCK,
    /* 0x46 */ 0x0,
    /* 0x47 */ 0x0, /* 47 (Keypad-7/Home) */
    /* 0x48 */ 0x0, /* 48 (Keypad-8/Up) */
    /* 0x49 */ 0x0, /* 49 (Keypad-9/PgUp) */
    /* 0x4a */ 0x0, /* 4a (Keypad--) */
    /* 0x4b */ 0x0, /* 4b (Keypad-4/Left) */
    /* 0x4c */ 0x0, /* 4c (Keypad-5) */
    /* 0x4d */ 0x0, /* 4d (Keypad-6/Right) */
    /* 0x4e */ 0x0, /* 4e (Keypad-+) */
    /* 0x4f */ 0x0, /* 4f (Keypad-1/End) */
    /* 0x50 */ 0x0, /* 50 (Keypad-2/Down) */
    /* 0x51 */ 0x0, /* 51 (Keypad-3/PgDn) */
    /* 0x52 */ 0x0, /* 52 (Keypad-0/Ins) */
    /* 0x53 */ 0x0, /* 53 (Keypad-./Del) */
    /* 0x54 */ 0x0, /* 54 (Alt-SysRq) on a 84+ key keyboard */
    /* 0x55 */ 0x0,
    /* 0x56 */ 0x0,
    /* 0x57 */ 0x0,
    /* 0x58 */ 0x0,
    /* 0x59 */ 0x0,
    /* 0x5a */ 0x0,
    /* 0x5b */ 0x0,
    /* 0x5c */ 0x0,
    /* 0x5d */ 0x0,
    /* 0x5e */ 0x0,
    /* 0x5f */ 0x0,
    /* 0x60 */ 0x0,
    /* 0x61 */ 0x0,
    /* 0x62 */ 0x0,
    /* 0x63 */ 0x0,
    /* 0x64 */ 0x0,
    /* 0x65 */ 0x0,
    /* 0x66 */ 0x0,
    /* 0x67 */ KEY_UPARROW,
    /* 0x68 */ 0x0,
    /* 0x69 */ KEY_LEFTARROW,
    /* 0x6a */ KEY_RIGHTARROW,
    /* 0x6b */ 0x0,
    /* 0x6c */ KEY_DOWNARROW,
    /* 0x6d */ 0x0,
    /* 0x6e */ 0x0,
    /* 0x6f */ 0x0,
    /* 0x70 */ 0x0,
    /* 0x71 */ 0x0,
    /* 0x72 */ 0x0,
    /* 0x73 */ 0x0,
    /* 0x74 */ 0x0,
    /* 0x75 */ 0x0,
    /* 0x76 */ 0x0,
    /* 0x77 */ 0x0,
    /* 0x78 */ 0x0,
    /* 0x79 */ 0x0,
    /* 0x7a */ 0x0,
    /* 0x7b */ 0x0,
    /* 0x7c */ 0x0,
    /* 0x7d */ 0x0,
    /* 0x7e */ 0x0,
    /* 0x7f */ KEY_FIRE, //KEY_RCTRL,
};

// Lookup table for mapping ASCII characters to their equivalent when
// shift is pressed on an American layout keyboard:
static const char shiftxform[] =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
    31, ' ', '!', '"', '#', '$', '%', '&',
    '"', // shift-'
    '(', ')', '*', '+',
    '<', // shift-,
    '_', // shift--
    '>', // shift-.
    '?', // shift-/
    ')', // shift-0
    '!', // shift-1
    '@', // shift-2
    '#', // shift-3
    '$', // shift-4
    '%', // shift-5
    '^', // shift-6
    '&', // shift-7
    '*', // shift-8
    '(', // shift-9
    ':',
    ':', // shift-;
    '<',
    '+', // shift-=
    '>', '?', '@',
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '[', // shift-[
    '!', // shift-backslash - OH MY GOD DOES WATCOM SUCK
    ']', // shift-]
    '"', '_',
    '\'', // shift-`
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
    'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    '{', '|', '}', '~', 127
};

/* Checks whether or not the given file descriptor is associated
   with a local keyboard.
   Returns 1 if it is, 0 if not (or if something prevented us from
   checking). */

int tty_is_kbd(int fd)
{
    int data = 0;

    if (ioctl(fd, KDGKBTYPE, &data) != 0)
        return 0;

    if (data == KB_84) {
        printf("84-key keyboard found.\n");
        return 1;
    } else if (data == KB_101) {
        printf("101-key keyboard found.\n");
        return 1;
    } else {
        printf("KDGKBTYPE = 0x%x.\n", data);
        return 0;
    }
}

static int old_mode = -1;
static struct termios old_term;
static int kb = -1; /* keyboard file descriptor */

void kbd_shutdown(void)
{
    /* Shut down nicely. */

    printf("Cleaning up.\n");
    fflush(stdout);

    printf("Exiting normally.\n");
    if (old_mode != -1) {
        ioctl(kb, KDSKBMODE, old_mode);
        tcsetattr(kb, 0, &old_term);
    }

    if (kb > 3)
        close(kb);

    exit(0);
}

static int kbd_init(void)
{
    struct termios new_term;
    char *files_to_try[] = {"/dev/tty", "/dev/tty0", "/dev/console", NULL};
    int i;
    int flags;
    int found = 0;

    /* First we need to find a file descriptor that represents the
       system's keyboard. This should be /dev/tty, /dev/console,
       stdin, stdout, or stderr. We'll try them in that order.
       If none are acceptable, we're probably not being run
       from a VT. */
    for (i = 0; files_to_try[i] != NULL; i++) {
        /* Try to open the file. */
        kb = open(files_to_try[i], O_RDONLY);
        if (kb < 0) continue;
        /* See if this is valid for our purposes. */
        if (tty_is_kbd(kb)) {
            printf("Using keyboard on %s.\n", files_to_try[i]);
            found = 1;
            break;
        }
        close(kb);
    }

    /* If those didn't work, not all is lost. We can try the
       3 standard file descriptors, in hopes that one of them
       might point to a console. This is not especially likely. */
    if (files_to_try[i] == NULL) {
        for (kb = 0; kb < 3; kb++) {
            if (tty_is_kbd(i)) {
                found = 1;
                break;
            }
        }
    }

    if (!found) {
        printf("Unable to find a file descriptor associated with "\
                "the keyboard.\n" \
                "Perhaps you're not using a virtual terminal?\n");
        return 1;
    }

    /* Find the keyboard's mode so we can restore it later. */
    if (ioctl(kb, KDGKBMODE, &old_mode) != 0) {
        printf("Unable to query keyboard mode.\n");
        kbd_shutdown();
    }

    /* Adjust the terminal's settings. In particular, disable
       echoing, signal generation, and line buffering. Any of
       these could cause trouble. Save the old settings first. */
    if (tcgetattr(kb, &old_term) != 0) {
        printf("Unable to query terminal settings.\n");
        kbd_shutdown();
    }

    new_term = old_term;
    new_term.c_iflag = 0;
    new_term.c_lflag &= ~(ECHO | ICANON | ISIG);

    /* TCSAFLUSH discards unread input before making the change.
       A good idea. */
    if (tcsetattr(kb, TCSAFLUSH, &new_term) != 0) {
        printf("Unable to change terminal settings.\n");
    }
    
    /* Put the keyboard in mediumraw mode. */
    if (ioctl(kb, KDSKBMODE, K_MEDIUMRAW) != 0) {
        printf("Unable to set mediumraw mode.\n");
        kbd_shutdown();
    }

    /* Put in non-blocking mode */
    flags = fcntl(kb, F_GETFL, 0);
    fcntl(kb, F_SETFL, flags | O_NONBLOCK);

    printf("Ready to read keycodes. Press Backspace to exit.\n");

    return 0;
}

int kbd_read(int *pressed, unsigned char *key)
{
    unsigned char data;

    if (read(kb, &data, 1) < 1) {
        return 0;
    }

    *pressed = (data & 0x80) == 0x80;
    *key = data & 0x7F;

    return 1;
}

static int mouse = -1;

int mouse_init(void)
{
    char *files_to_try[] = {"/dev/input/mouse0", "/dev/input/mouse1", NULL};

    for (int i = 0; files_to_try[i] != NULL; i++) {
        if((mouse = open(files_to_try[i], O_RDONLY | O_NONBLOCK)) == -1) {
            printf("Open %s: ERROR\n", files_to_try[i]);
            continue;
        } else {
            printf("Open %s: OK\n", files_to_try[i]);
        }
    }

    return 0;
}

int mouse_read(int *_status, unsigned char *_button, int *_abs_x, int *_abs_y)
{
    struct input_event ievent;
    unsigned char *ptr = (unsigned char*)&ievent;
    
    unsigned char button;
    char x, y;
    int abs_x = 0, abs_y = 0;

    int status = -1;

    status = read(mouse, &ievent, sizeof(struct input_event));

    button=ptr[0];
    
    x = (char)ptr[1];
    y = (char)ptr[2];
    
    abs_x += x;
    abs_y -= y;

    *_status = status;
    *_button = button;
    *_abs_x = abs_x;
    *_abs_y = abs_y;

    return status;
}

static unsigned char TranslateKey(unsigned char key)
{
    if (key < sizeof(at_to_doom))
        return at_to_doom[key];
    else
        return 0x0;
}

// Get the equivalent ASCII (Unicode?) character for a keypress.

static unsigned char GetTypedChar(unsigned char key)
{
    key = TranslateKey(key);

    // Is shift held down?  If so, perform a translation.

    if (shiftdown > 0)
    {
        if (key >= 0 && key < arrlen(shiftxform))
        {
            key = shiftxform[key];
        }
        else
        {
            key = 0;
        }
    }

    return key;
}

static void UpdateShiftStatus(int pressed, unsigned char key)
{
    int change;

    if (pressed) {
        change = 1;
    } else {
        change = -1;
    }

    if (key == 0x2a || key == 0x36) {
        shiftdown += change;
    }
}

void I_GetEvent(void)
{
    event_t event;
    int pressed;
    unsigned char key;
    unsigned char button = 0;
    int abs_x = 0, abs_y = 0;
    int status = -1;

    // put event-grabbing stuff in here
    
    while (kbd_read(&pressed, &key) || mouse_read(&status, &button, &abs_x, &abs_y))
    {
        if (key == 0x0E) {
            kbd_shutdown();
            I_Quit();
        }

        UpdateShiftStatus(pressed, key);

        // process event
        
        if (!pressed)
        {
            // data1 has the key pressed, data2 has the character
            // (shift-translated, etc)
            event.type = ev_keydown;
            event.data1 = TranslateKey(key);
            event.data2 = GetTypedChar(key);

            if (event.data1 != 0)
            {
                D_PostEvent(&event);
            }
        }
        else
        {
            event.type = ev_keyup;
            event.data1 = TranslateKey(key);

            // data2 is just initialized to zero for ev_keyup.
            // For ev_keydown it's the shifted Unicode character
            // that was typed, but if something wants to detect
            // key releases it should do so based on data1
            // (key ID), not the printable char.

            event.data2 = 0;

            if (event.data1 != 0)
            {
                D_PostEvent(&event);
            }
            break;
        }

        if (status != -1)
        {
            event.type = ev_mouse;
            event.data1 = button;
            event.data2 = abs_x;
            event.data3 = abs_y;
            D_PostEvent(&event);

            break;
        }
    }
}

void I_InitInput(void)
{
    kbd_init();
    mouse_init();
}
