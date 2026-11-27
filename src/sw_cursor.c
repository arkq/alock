/*
 * alock - sw_cursor.c
 * Copyright (c) 2005 - 2007 Mathias Gumz <akira at fluxbox dot org>
 *               2014 - 2018 Arkadiusz Bokowy
 *
 * This file is a part of an alock.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#include "sw_cursor.h"

#if ENABLE_XRENDER

#include <X11/extensions/Xrender.h>
#include <stdlib.h>

struct swCursor {
    Display     *display;
    Window       bg_win;
    Pixmap       src_pm;    /* cursor pixels uploaded to server (depth 32) */
    Picture      src_pic;   /* XRender picture wrapping src_pm */
    Pixmap       save_pm;   /* bg pixels under cursor before stamping */
    Picture      bg_pic;    /* picture wrapping bg_win for compositing */
    GC           gc;        /* for XCopyArea save/restore */
    unsigned int w, h;
    int          hot_x, hot_y;
    int          cur_x, cur_y; /* position of last stamp */
    int          valid;        /* non-zero once cursor has been stamped once */
};

struct swCursor *swCursorCreate(Display *dpy, Window bg_win,
                                const struct aCursorImage *img) {
    int screen = DefaultScreen(dpy);

    XRenderPictFormat *fmt32 = XRenderFindStandardFormat(dpy, PictStandardARGB32);
    if (!fmt32)
        return NULL;

    struct swCursor *swc = calloc(1, sizeof(*swc));
    if (!swc)
        return NULL;

    swc->display = dpy;
    swc->bg_win  = bg_win;
    swc->w       = img->width;
    swc->h       = img->height;
    swc->hot_x   = img->hot_x;
    swc->hot_y   = img->hot_y;

    /* Upload cursor pixels to a depth-32 server-side pixmap. */
    swc->src_pm = XCreatePixmap(dpy, bg_win, img->width, img->height, 32);
    GC gc32 = XCreateGC(dpy, swc->src_pm, 0, NULL);
    XImage ximg = {0};
    ximg.width            = (int)img->width;
    ximg.height           = (int)img->height;
    ximg.format           = ZPixmap;
    ximg.data             = (char *)img->pixels;
    ximg.byte_order       = alock_native_byte_order();
    ximg.bitmap_unit      = 32;
    ximg.bitmap_bit_order = ximg.byte_order;
    ximg.bitmap_pad       = 32;
    ximg.depth            = 32;
    ximg.bits_per_pixel   = 32;
    ximg.bytes_per_line   = (int)img->width * 4;
    ximg.red_mask         = 0x00ff0000;
    ximg.green_mask       = 0x0000ff00;
    ximg.blue_mask        = 0x000000ff;
    XInitImage(&ximg);
    XPutImage(dpy, swc->src_pm, gc32, &ximg, 0, 0, 0, 0, img->width, img->height);
    XFreeGC(dpy, gc32);
    swc->src_pic = XRenderCreatePicture(dpy, swc->src_pm, fmt32, 0, NULL);

    /* Save pixmap (same depth as bg_win) for bg save/restore under cursor. */
    int depth = DefaultDepth(dpy, screen);
    swc->save_pm = XCreatePixmap(dpy, bg_win, img->width, img->height, depth);

    XRenderPictFormat *bg_fmt = XRenderFindVisualFormat(dpy, DefaultVisual(dpy, screen));

    /* ClipByChildren (default): stamp goes to bg_win only; the frame child sits on top. */
    swc->bg_pic = XRenderCreatePicture(dpy, bg_win, bg_fmt, 0, NULL);

    swc->gc = XCreateGC(dpy, bg_win, 0, NULL);

    return swc;
}

/* Restore bg at old position, save bg at new position, stamp cursor. */
void swCursorMove(struct swCursor *swc, int x, int y) {
    Display *dpy = swc->display;
    int dst_x = x - swc->hot_x;
    int dst_y = y - swc->hot_y;

    if (swc->valid) {
        int old_x = swc->cur_x - swc->hot_x;
        int old_y = swc->cur_y - swc->hot_y;
        XCopyArea(dpy, swc->save_pm, swc->bg_win, swc->gc,
                  0, 0, swc->w, swc->h, old_x, old_y);
    }

    XCopyArea(dpy, swc->bg_win, swc->save_pm, swc->gc,
              dst_x, dst_y, swc->w, swc->h, 0, 0);

    XRenderComposite(dpy, PictOpOver,
                     swc->src_pic, None, swc->bg_pic,
                     0, 0, 0, 0,
                     dst_x, dst_y, swc->w, swc->h);

    swc->cur_x = x;
    swc->cur_y = y;
    swc->valid = 1;
}

/* Re-stamp cursor after bg repaint. save_pm is left untouched — it already holds the clean bg
 * from the last swCursorMove; overwriting it here would corrupt the restore on the next move.
 * Falls back to XQueryPointer if no prior move was recorded. */
void swCursorRestamp(struct swCursor *swc) {
    if (!swc->valid) {
        Window root_ret, child_ret;
        int root_x, root_y, win_x, win_y;
        unsigned int mask;
        if (XQueryPointer(swc->display, DefaultRootWindow(swc->display),
                          &root_ret, &child_ret,
                          &root_x, &root_y, &win_x, &win_y, &mask))
            swCursorMove(swc, root_x, root_y);
        return;
    }
    Display *dpy = swc->display;
    int dst_x = swc->cur_x - swc->hot_x;
    int dst_y = swc->cur_y - swc->hot_y;

    XRenderComposite(dpy, PictOpOver,
                     swc->src_pic, None, swc->bg_pic,
                     0, 0, 0, 0,
                     dst_x, dst_y, swc->w, swc->h);
}

void swCursorDestroy(struct swCursor *swc) {
    if (!swc) return;
    Display *dpy = swc->display;
    XRenderFreePicture(dpy, swc->src_pic);
    XFreePixmap(dpy, swc->src_pm);
    XFreePixmap(dpy, swc->save_pm);
    XRenderFreePicture(dpy, swc->bg_pic);
    XFreeGC(dpy, swc->gc);
    free(swc);
}

Window swCursorGetBgWin(struct swCursor *swc) { return swc->bg_win; }

#else /* !ENABLE_XRENDER */

struct swCursor *swCursorCreate(Display *d, Window w, const struct aCursorImage *i)
    { (void)d; (void)w; (void)i; return NULL; }
void   swCursorMove(struct swCursor *s, int x, int y) { (void)s; (void)x; (void)y; }
void   swCursorRestamp(struct swCursor *s) { (void)s; }
void   swCursorDestroy(struct swCursor *s) { (void)s; }
Window swCursorGetBgWin(struct swCursor *s) { (void)s; return None; }

#endif /* ENABLE_XRENDER */
