/*
 * alock - cursor_blank.c
 * Copyright (c) 2016 Arkadiusz Bokowy
 *
 * This file is a part of an alock.
 *
 * This project is licensed under the terms of the MIT license.
 *
 * This cursor module provides:
 *  -cursor blank
 *
 */

#include "alock.h"


static struct moduleData {
    Display *display;
    Cursor cursor;
} data = { 0 };

static int module_init(Display *dpy) {

    Screen *screen = DefaultScreenOfDisplay(dpy);
    char no_data[8] = { 0 };
    XColor black;
    Pixmap blank;

    data.display = dpy;
    alock_alloc_color(dpy, DefaultColormapOfScreen(screen), NULL, "black", &black);

    blank = XCreateBitmapFromData(dpy, RootWindowOfScreen(screen), no_data, 8, 8);
    data.cursor = XCreatePixmapCursor(dpy, blank, blank, &black, &black, 0, 0);

    return 0;
}

static void module_free() {

    Display *dpy = data.display;
    Screen *screen = DefaultScreenOfDisplay(dpy);

    if (data.cursor)
        XFreeCursor(dpy, data.cursor);

    /* Cursor was not visible, so it can be anywhere by now. Help to locate
     * it, by placing it in a center of the screen. */
    XWarpPointer(dpy, None, RootWindowOfScreen(screen), 0, 0, 0, 0,
            WidthOfScreen(screen) / 2, HeightOfScreen(screen) / 2);

}

static Cursor module_getcursor(void) {
    return data.cursor;
}

static const struct aCursorImage *module_getimage(void) { return NULL; }


struct aModuleCursor alock_cursor_blank = {
    { "blank",
        module_dummy_loadargs,
        module_dummy_loadxrdb,
        module_init,
        module_free,
    },
    module_getcursor,
    module_getimage,
};
