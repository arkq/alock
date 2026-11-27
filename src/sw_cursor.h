/*
 * alock - sw_cursor.h
 * Copyright (c) 2005 - 2007 Mathias Gumz <akira at fluxbox dot org>
 *               2014 - 2018 Arkadiusz Bokowy
 *
 * This file is a part of an alock.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#ifndef ALOCK_SW_CURSOR_H_
#define ALOCK_SW_CURSOR_H_

#include <X11/Xlib.h>
#include "alock.h"

/* Software cursor overlay state (opaque handle, defined in sw_cursor.c). */
struct swCursor;

struct swCursor *swCursorCreate(Display *dpy, Window bg_win,
                                const struct aCursorImage *img);
void   swCursorMove(struct swCursor *swc, int x, int y);
void   swCursorRestamp(struct swCursor *swc);
void   swCursorDestroy(struct swCursor *swc);
Window swCursorGetBgWin(struct swCursor *swc);

#endif /* ALOCK_SW_CURSOR_H_ */
