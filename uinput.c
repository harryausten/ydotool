/*
    This file is part of ydotool.
	Copyright (C) 2019 Harry Austen
    Copyright (C) 2018-2019 ReimuNotMoe

    This program is free software: you can redistribute it and/or modify
    it under the terms of the MIT License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/**
 * @file uinput.c
 * @author Harry Austen
 * @brief Implementation of functions for emulating input events
 */

/* Local includes */
#include "uinput.h"
/* System includes */
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <sys/stat.h>

/* Wrapper macro for errno error check */
#define CHECK(X) if (X == -1) { fprintf( stderr, "ERROR (%s:%d) -- %s\n", __FILE__, __LINE__, strerror(errno) ); return 1; }

/* uinput file descriptor */
int FD = -1;

/* All valid keycodes */
const int KEYCODES[] = {
    BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5,
    KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_MINUS, KEY_EQUAL, KEY_Q, KEY_W,
    KEY_E, KEY_R, KEY_T, KEY_Y, KEY_U, KEY_I, KEY_O, KEY_P, KEY_LEFTBRACE,
    KEY_RIGHTBRACE, KEY_A, KEY_S, KEY_D, KEY_F, KEY_G, KEY_H, KEY_J,
    KEY_K, KEY_L, KEY_SEMICOLON, KEY_APOSTROPHE, KEY_GRAVE,
    KEY_BACKSLASH, KEY_102ND, KEY_Z, KEY_X, KEY_C, KEY_V, KEY_B, KEY_N,
    KEY_M, KEY_COMMA, KEY_DOT, KEY_SLASH, KEY_SPACE, KEY_TAB, KEY_ENTER,
	KEY_LEFTALT, KEY_RIGHTALT, KEY_LEFTSHIFT, KEY_RIGHTSHIFT, KEY_LEFTCTRL,
    KEY_RIGHTCTRL, KEY_LEFTMETA, KEY_RIGHTMETA, KEY_UP, KEY_DOWN, KEY_LEFT,
    KEY_RIGHT, KEY_CAPSLOCK, KEY_NUMLOCK, KEY_SCROLLLOCK, KEY_ESC,
    KEY_BACKSPACE, KEY_DELETE, KEY_INSERT, KEY_HOME, KEY_END,
    KEY_PAGEUP, KEY_PAGEDOWN, KEY_SYSRQ, KEY_PAUSE, KEY_F1, KEY_F2, KEY_F3,
    KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10,  KEY_F11, KEY_F12
};

/* All valid event codes */
int EVCODES[] = {
    EV_KEY,
    EV_REL,
    EV_ABS,
    EV_SYN
};

struct key_char {
    char character;
    int code;
};

struct key_string {
    char string[11];
    int code;
};

/* All valid non-shifted characters */
const struct key_char NORMAL_KEYS[] = {
    {'\t', KEY_TAB},
    {'\n', KEY_ENTER},
    {' ', KEY_SPACE},
    {'#', KEY_BACKSLASH},
    {'\'', KEY_APOSTROPHE},
    {',', KEY_COMMA},
    {'-', KEY_MINUS},
    {'.', KEY_DOT},
    {'/', KEY_SLASH},
    {'0', KEY_0},
    {'1', KEY_1},
    {'2', KEY_2},
    {'3', KEY_3},
    {'4', KEY_4},
    {'5', KEY_5},
    {'6', KEY_6},
    {'7', KEY_7},
    {'8', KEY_8},
    {'9', KEY_9},
    {';', KEY_SEMICOLON},
    {'=', KEY_EQUAL},
    {'[', KEY_LEFTBRACE},
    {'\\', KEY_102ND},
    {']', KEY_RIGHTBRACE},
    {'`', KEY_GRAVE},
    {'a', KEY_A},
    {'b', KEY_B},
    {'c', KEY_C},
    {'d', KEY_D},
    {'e', KEY_E},
    {'f', KEY_F},
    {'g', KEY_G},
    {'h', KEY_H},
    {'i', KEY_I},
    {'j', KEY_J},
    {'k', KEY_K},
    {'l', KEY_L},
    {'m', KEY_M},
    {'n', KEY_N},
    {'o', KEY_O},
    {'p', KEY_P},
    {'q', KEY_Q},
    {'r', KEY_R},
    {'s', KEY_S},
    {'t', KEY_T},
    {'u', KEY_U},
    {'v', KEY_V},
    {'w', KEY_W},
    {'x', KEY_X},
    {'y', KEY_Y},
    {'z', KEY_Z}
};

/* All valid shifted characters */
const struct key_char SHIFTED_KEYS[] = {
    {'!', KEY_1},
    {'"', KEY_2},
    {'$', KEY_4},
    {'%', KEY_5},
    {'&', KEY_7},
    {'(', KEY_9},
    {')', KEY_0},
    {'*', KEY_8},
    {'+', KEY_EQUAL},
    {':', KEY_SEMICOLON},
    {'<', KEY_COMMA},
    {'>', KEY_DOT},
    {'?', KEY_SLASH},
    {'@', KEY_APOSTROPHE},
    {'A', KEY_A},
    {'B', KEY_B},
    {'C', KEY_C},
    {'D', KEY_D},
    {'E', KEY_E},
    {'F', KEY_F},
    {'G', KEY_G},
    {'H', KEY_H},
    {'I', KEY_I},
    {'J', KEY_J},
    {'K', KEY_K},
    {'L', KEY_L},
    {'M', KEY_M},
    {'N', KEY_N},
    {'O', KEY_O},
    {'P', KEY_P},
    {'Q', KEY_Q},
    {'R', KEY_R},
    {'S', KEY_S},
    {'T', KEY_T},
    {'U', KEY_U},
    {'V', KEY_V},
    {'W', KEY_W},
    {'X', KEY_X},
    {'Y', KEY_Y},
    {'Z', KEY_Z},
    {'^', KEY_6},
    {'_', KEY_MINUS},
    {'{', KEY_LEFTBRACE},
    {'|', KEY_102ND},
    {'}', KEY_RIGHTBRACE},
    {'~', KEY_BACKSLASH}
};

const struct key_string MODIFIER_KEYS[] = {
	{"ALT", KEY_LEFTALT},
    {"ALT_L", KEY_LEFTALT},
    {"ALT_R", KEY_RIGHTALT},
	{"CTRL", KEY_LEFTCTRL},
    {"CTRL_L", KEY_LEFTCTRL},
    {"CTRL_R", KEY_RIGHTCTRL},
	{"META", KEY_LEFTMETA},
    {"META_L", KEY_LEFTMETA},
    {"META_R", KEY_RIGHTMETA},
	{"SHIFT", KEY_LEFTSHIFT},
    {"SHIFT_L", KEY_LEFTSHIFT},
    {"SHIFT_R", KEY_RIGHTSHIFT},
	{"SUPER", KEY_LEFTMETA},
    {"SUPER_L", KEY_LEFTMETA},
    {"SUPER_R", KEY_RIGHTMETA}
};

const struct key_string FUNCTION_KEYS[] = {
	{"BACKSPACE", KEY_BACKSPACE},
    {"CAPSLOCK", KEY_CAPSLOCK},
    {"DELETE", KEY_DELETE},
    {"DOWN", KEY_DOWN},
    {"END", KEY_END},
    {"ENTER", KEY_ENTER},
    {"ESC", KEY_ESC},
	{"F1", KEY_F1},
    {"F10", KEY_F10},
    {"F11", KEY_F11},
    {"F12", KEY_F12},
    {"F2", KEY_F2},
    {"F3", KEY_F3},
    {"F4", KEY_F4},
    {"F5", KEY_F5},
    {"F6", KEY_F6},
    {"F7", KEY_F7},
    {"F8", KEY_F8},
    {"F9", KEY_F9},
    {"HOME", KEY_HOME},
    {"INSERT", KEY_INSERT},
    {"LEFT", KEY_LEFT},
    {"NUMLOCK", KEY_NUMLOCK},
    {"PAGEDOWN", KEY_PAGEDOWN},
    {"PAGEUP", KEY_PAGEUP},
    {"PAUSE", KEY_PAUSE},
    {"RIGHT", KEY_RIGHT},
    {"SCROLLLOCK", KEY_SCROLLLOCK},
    {"SYSRQ", KEY_SYSRQ},
	{"TAB", KEY_TAB},
    {"UP", KEY_UP}
};

int uinput_test() {
    for (size_t i = 1; i != sizeof(NORMAL_KEYS)/sizeof(struct key_char); ++i) {
        if (NORMAL_KEYS[i].character < NORMAL_KEYS[i-1].character) {
            printf("%c < %c\n", NORMAL_KEYS[i].character, NORMAL_KEYS[i-1].character);
        }
    }

    for (size_t i = 1; i != sizeof(SHIFTED_KEYS)/sizeof(struct key_char); ++i) {
        if (SHIFTED_KEYS[i].character < SHIFTED_KEYS[i-1].character) {
            printf("%c < %c\n", SHIFTED_KEYS[i].character, SHIFTED_KEYS[i-1].character);
        }
    }

    for (size_t i = 1; i != sizeof(MODIFIER_KEYS)/sizeof(struct key_string); ++i) {
        if (strcmp(MODIFIER_KEYS[i].string, MODIFIER_KEYS[i-1].string) < 0) {
            printf("%s < %s\n", MODIFIER_KEYS[i].string, MODIFIER_KEYS[i-1].string);
        }
    }

    for (size_t i = 1; i != sizeof(FUNCTION_KEYS)/sizeof(struct key_string); ++i) {
        if (strcmp(FUNCTION_KEYS[i].string, FUNCTION_KEYS[i-1].string) < 0) {
            printf("%s < %s\n", FUNCTION_KEYS[i].string, FUNCTION_KEYS[i-1].string);
        }
    }

    return 0;
}

/* Initialise the input device */
int uinput_init() {
    /* Check write access to uinput driver device */
    if (access("/dev/uinput", W_OK)) {
        fprintf(stderr, "Do not have access to write to /dev/uinput!\n"
            "Try running as root\n");
        return 1;
    }

    /* Confirm availability of uinput kernel module */
    struct utsname uname_buffer;
    CHECK( uname(&uname_buffer) );
    char kernel_mod_dir[50] = "/lib/modules/";
    strcat(kernel_mod_dir, uname_buffer.release);
    struct stat stats;
    stat(kernel_mod_dir, &stats);
    if (!S_ISDIR(stats.st_mode)) {
        fprintf(stderr, "Dir (%s) doesn't exist!\n"
            "Have you recently updated your kernel version?\n"
            "Restart your system to use new kernel modules\n", kernel_mod_dir);
        return 1;
    }

    /* Open uinput driver device */
    CHECK( (FD = open("/dev/uinput", O_WRONLY|O_NONBLOCK)) );

    /* Events/Keys setup */
    for (int i = 0; i != sizeof(KEYCODES)/sizeof(KEYCODES[0]); ++i) {
        CHECK( ioctl(FD, UI_SET_KEYBIT, KEYCODES[i]) );
    }
    for (int i = 0; i != sizeof(EVCODES)/sizeof(EVCODES[0]); ++i) {
        CHECK( ioctl(FD, UI_SET_EVBIT, EVCODES[i]) );
    }

    /* uinput device setup */
    struct uinput_setup usetup = {
        {
            BUS_USB,
            0x1234,
            0x5678,
            0
        },
        "ydotool virtual device",
        0
    };

    CHECK( ioctl(FD, UI_DEV_SETUP, &usetup) );
    CHECK( ioctl(FD, UI_DEV_CREATE) );

    /* Wait for device to come up */
    usleep(1000000);

    return 0;
}

/* Delete the input device */
int uinput_destroy() {
    if (FD != -1) {
        CHECK( ioctl(FD, UI_DEV_DESTROY) );
        close(FD);
        FD = -1;
    }
    return 0;
}

/* Character pointer comparer for use with bsearch */
int cmp_chars(const void * a, const void * b) {
    return (*(char *)a - *(char *)b);
}

/* TODO: Implement binary search */
int uinput_enter_key(const char * key_string) {
    /* Search modifier keys */
    for (int i = 0; i != sizeof(MODIFIER_KEYS)/sizeof(struct key_string); ++i) {
        if (!strcmp(key_string, MODIFIER_KEYS[i].string)) {
            if (uinput_send_keypress(MODIFIER_KEYS[i].code)) {
                return 1;
            }
            return 0;
        }
    }

    /* Search function keys */
    for (int i = 0; i != sizeof(FUNCTION_KEYS)/sizeof(struct key_string); ++i) {
        if (!strcmp(key_string, FUNCTION_KEYS[i].string)) {
            if (uinput_send_keypress(FUNCTION_KEYS[i].code)) {
                return 1;
            }
            return 0;
        }
    }

    /* If string is a single character */
    if (strlen(key_string) == 1) {
        /* Search normal keys */
        for (int i = 0; i != sizeof(NORMAL_KEYS)/sizeof(struct key_char); ++i) {
            if (key_string[0] == NORMAL_KEYS[i].character) {
                if (uinput_send_keypress(NORMAL_KEYS[i].code)) {
                    return 1;
                }
                return 0;
            }
        }

        /* Search shifted keys */
        for (int i = 0; i != sizeof(SHIFTED_KEYS)/sizeof(struct key_char); ++i) {
            if (key_string[0] == SHIFTED_KEYS[i].character) {
                if (uinput_send_shifted_keypress(SHIFTED_KEYS[i].code)) {
                    return 1;
                }
                return 0;
            }
        }
    }
    return 1;
}

/* Emulate typing the given character on the vitual device */
int uinput_enter_char(char c) {
/* This should be re-enabled once binary searching the key arrays has been implemented
    void * found;
    void * begin_norm = (void *)&NORMAL_KEYS;
    void * begin_shift = (void *)&SHIFTED_KEYS;

    if ((found = bsearch(&c, begin_norm, sizeof(NORMAL_KEYS)/sizeof(char), sizeof(char), cmp_chars))) {
        if (uinput_send_keypress(NORMAL_KEYCODES[((char *)found - (char *)begin_norm)/sizeof(char)])) {
            return 1;
        }
    } else if ((found = bsearch(&c, begin_shift, sizeof(SHIFTED_KEYS)/sizeof(char), sizeof(char), cmp_chars))) {
        if (uinput_send_shifted_keypress(SHIFTED_KEYCODES[((char *)found - (char *)begin_shift)/sizeof(char)])) {
            return 1;
        }
    } else {
        fprintf(stderr, "Unsupported character (%d:%c) cannot be entered!\n", c, c);
        return 1;
    }

*/
    return 0;
}

/* Trigger an input event */
int uinput_emit(uint16_t type, uint16_t code, int32_t value) {
    struct input_event ie = {
        /* Ignore timestamp values */
        {0,0},
        type,
        code,
        value
    };

    if (FD == -1) {
        if (uinput_init()) {
            return 1;
        }
    }

    CHECK( write(FD, &ie, sizeof(ie)) );

    /* Allow processing time for uinput before sending next event */
    usleep( 50 );

    return 0;
}

/* Single key event and report */
int uinput_send_key(uint16_t code, int32_t value) {
    if (uinput_emit(EV_KEY, code, value)) {
        return 1;
    }
    if (uinput_emit(EV_SYN, SYN_REPORT, 0)) {
        return 1;
    }
    return 0;
}

/* Emulate a quick key press */
int uinput_send_keypress(uint16_t code) {
    /* send press */
    if (uinput_send_key(code, 1)) {
        return 1;
    }
    /* send release */
    if (uinput_send_key(code, 0)) {
        return 1;
    }
    return 0;
}

/* Emulate a shifted key press */
int uinput_send_shifted_keypress(uint16_t code) {
    /* Send shift press */
    if (uinput_send_key(KEY_LEFTSHIFT, 1)) {
        return 1;
    }
    /* Emulate keypress */
    if (uinput_send_keypress(code)) {
        return 1;
    }
    /* Send shift release */
    if (uinput_send_key(KEY_LEFTSHIFT, 0)) {
        return 1;
    }
    return 0;
}

/* Move the cursor to a given (x,y) position */
int uinput_move_mouse(int32_t x, int32_t y) {
    if (uinput_emit(EV_ABS, ABS_X, x)) {
        return 1;
    }
    if (uinput_emit(EV_ABS, ABS_Y, y)) {
        return 1;
    }
    if (uinput_emit(EV_SYN, SYN_REPORT, 0)) {
        return 1;
    }
    return 0;
}

/* Move the cursor a given (x,y) relative to the current position */
int uinput_relative_move_mouse(int32_t x, int32_t y) {
    if (x) {
        if (uinput_emit(EV_REL, REL_X, x)) {
            return 1;
        }
    }
    if (y) {
        if (uinput_emit(EV_REL, REL_Y, y)) {
            return 1;
        }
    }
    if (uinput_emit(EV_SYN, SYN_REPORT, 0)) {
        return 1;
    }
    return 0;
}
