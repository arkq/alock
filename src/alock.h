/*
 * alock - alock.h
 * Copyright (c) 2005 - 2007 Mathias Gumz <akira at fluxbox dot org>
 *               2014 - 2018 Arkadiusz Bokowy
 *
 * This file is a part of an alock.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#ifndef ALOCK_ALOCK_H_
#define ALOCK_ALOCK_H_

#if HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdint.h>
#include <stdio.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>


#if DEBUG
# define debug(M, ARGS ...) fprintf(stderr, "DEBUG %s:%d: " M "\n", __FILE__, __LINE__, ## ARGS)
#else
# define debug(M, ARGS ...) do {} while (0)
#endif


/* possible states of the input module */
enum aInputState {
    AINPUT_STATE_NONE,
    AINPUT_STATE_INIT,
    AINPUT_STATE_CHECK,
    AINPUT_STATE_VALID,
    AINPUT_STATE_ERROR,
};


/* module base interface */
struct aModule {
    const char *name;
    void (*loadargs)(const char *args);
    void (*loadxrdb)(XrmDatabase database);
    int (*init)(Display *display);
    void (*free)();
};

struct aModuleAuth {
    struct aModule m;
    int (*authenticate)(const char *pass);
};

struct aModuleBackground {
    struct aModule m;
    Window (*getwindow)(int screen);
};

/* Raw ARGB pixels exposed by image-based cursor modules for the software overlay. */
struct aCursorImage {
    unsigned int width, height;
    int hot_x, hot_y;
    uint32_t *pixels;   /* pre-multiplied ARGB, native byte order, w*h entries */
};

struct aModuleCursor {
    struct aModule m;
    Cursor (*getcursor)(void);
    /* Returns pixel data for the software cursor overlay, or NULL for non-image
     * modules (none, blank, glyph).  Pointer is valid for the module's lifetime. */
    const struct aCursorImage *(*getimage)(void);
};

struct aModuleInput {
    struct aModule m;
    Window (*getwindow)(int screen);
    KeySym (*keypress)(KeySym key);
    void (*setstate)(enum aInputState state);
};


/* module container structure */
struct aModules {
    struct aModuleAuth *auth;
    struct aModuleBackground *background;
    struct aModuleCursor *cursor;
    struct aModuleInput *input;
#if WITH_XBLIGHT
    float backlight;
#endif
};


/* authentication modules */
extern struct aModuleAuth alock_auth_none;
#if ENABLE_HASH
extern struct aModuleAuth alock_auth_hash;
#endif
#if ENABLE_PASSWD
extern struct aModuleAuth alock_auth_passwd;
#endif
#if ENABLE_PAM
extern struct aModuleAuth alock_auth_pam;
#endif

/* background modules */
extern struct aModuleBackground alock_bg_none;
extern struct aModuleBackground alock_bg_blank;
#if ENABLE_IMLIB2
extern struct aModuleBackground alock_bg_image;
#endif
#if ENABLE_XRENDER
extern struct aModuleBackground alock_bg_shade;
#endif

/* cursor modules */
extern struct aModuleCursor alock_cursor_none;
extern struct aModuleCursor alock_cursor_blank;
extern struct aModuleCursor alock_cursor_glyph;
#if ENABLE_XCURSOR
extern struct aModuleCursor alock_cursor_xcursor;
#if (ENABLE_XRENDER && (ENABLE_XPM || ENABLE_IMLIB2))
extern struct aModuleCursor alock_cursor_image;
#endif
#endif

/* input modules */
extern struct aModuleInput alock_input_none;
extern struct aModuleInput alock_input_frame;


/* dummy functions for module interface */
void module_dummy_loadargs(const char *args);
void module_dummy_loadxrdb(XrmDatabase database);
int module_dummy_init(Display *display);
void module_dummy_free(void);


/* helper functions defined in utils.c */
unsigned long alock_mtime(void);
int alock_native_byte_order(void);
int alock_alloc_color(Display *display,
        Colormap colormap,
        const char *color_name,
        const char *fallback_name,
        XColor *result);
int alock_check_xrender(Display *display);
int alock_shade_pixmap(Display *display,
        Visual *visual,
        const Pixmap src_pm,
        Pixmap dst_pm,
        unsigned char shade,
        int src_x, int src_y,
        int dst_x, int dst_y,
        unsigned int width,
        unsigned int height);
int alock_blur_pixmap(Display *display,
        Visual *visual,
        const Pixmap src_pm,
        Pixmap dst_pm,
        unsigned char blur,
        int src_x, int src_y,
        int dst_x, int dst_y,
        unsigned int width,
        unsigned int height);
int alock_grayscale_image(XImage *image,
        int x, int y,
        unsigned int width,
        unsigned int height);

#endif /* ALOCK_ALOCK_H_ */
