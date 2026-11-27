/*
 * alock - cursor_none.c
 * Copyright (c) 2005 - 2007 Mathias Gumz <akira at fluxbox dot org>
 *               2014 Arkadiusz Bokowy
 *
 * This file is a part of an alock.
 *
 * This project is licensed under the terms of the MIT license.
 *
 * This cursor module provides:
 *  -cursor none
 *
 */

#include "alock.h"


static Cursor module_getcursor(void) {
    return None;
}

static const struct aCursorImage *module_getimage(void) { return NULL; }


struct aModuleCursor alock_cursor_none = {
    { "none",
        module_dummy_loadargs,
        module_dummy_loadxrdb,
        module_dummy_init,
        module_dummy_free,
    },
    module_getcursor,
    module_getimage,
};
