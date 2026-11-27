/*
 * alock - cursor_xcursor.c
 * Copyright (c) 2005 - 2007 Mathias Gumz <akira at fluxbox dot org>
 *               2014 - 2016 Arkadiusz Bokowy
 *
 * This file is a part of an alock.
 *
 * This project is licensed under the terms of the MIT license.
 *
 * This cursor module provides:
 *  -cursor xcursor:file=<file>
 *
 */

#include "alock.h"

#include <stdlib.h>
#include <string.h>
#include <X11/Xcursor/Xcursor.h>


static struct moduleData {
    Display *display;
    char *filename;
    Cursor cursor;
    XcursorImages *images;      /* kept alive; pixels owned by images[0] */
    struct aCursorImage image;
} data = { 0 };


static void module_loadargs(const char *args) {

    if (!args || strstr(args, "xcursor:") != args)
        return;

    char *arguments = strdup(&args[8]);
    char *arg;
    char *tmp;

    for (tmp = arguments; tmp; ) {
        arg = strsep(&tmp, ",");
        if (strstr(arg, "file=") == arg) {
            free(data.filename);
            data.filename = strdup(&arg[5]);
        }
    }

    free(arguments);
}


static int module_init(Display *dpy) {

    data.display = dpy;

    if (data.filename) {
        data.images = XcursorFilenameLoadImages(data.filename,
                                                XcursorGetDefaultSize(dpy));
        if (data.images && data.images->nimage > 0) {
            data.cursor = XcursorImagesLoadCursor(dpy, data.images);
            XcursorImage *img = data.images->images[0];
            data.image = (struct aCursorImage){
                img->width, img->height,
                (int)img->xhot, (int)img->yhot,
                (uint32_t *)img->pixels,  /* XcursorPixel is unsigned int = 32-bit */
            };
        }
    }

    if (data.cursor == 0) {
        fprintf(stderr, "[xcursor]: unable to load cursor file\n");
        return -1;
    }

    return 0;
}

static void module_free() {

    if (data.cursor)
        XFreeCursor(data.display, data.cursor);

    if (data.images) {
        XcursorImagesDestroy(data.images);
        data.images = NULL;
    }

    free(data.filename);
    data.filename = NULL;
}

static Cursor module_getcursor(void) {
    return data.cursor;
}

static const struct aCursorImage *module_getimage(void) {
    return (data.images && data.images->nimage > 0) ? &data.image : NULL;
}


struct aModuleCursor alock_cursor_xcursor = {
    { "xcursor",
        module_loadargs,
        module_dummy_loadxrdb,
        module_init,
        module_free,
    },
    module_getcursor,
    module_getimage,
};
