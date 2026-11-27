/*
 * alock - cursor_glyph.c
 * Copyright (c) 2005 - 2007 Mathias Gumz <akira at fluxbox dot org>
 *               2014 - 2016 Arkadiusz Bokowy
 *
 * This file is a part of an alock.
 *
 * This project is licensed under the terms of the MIT license.
 *
 * This cursor module provides:
 *  -cursor glyph:name=<glyph>,fg=<color>,bg=<color>
 *
 * Used resources:
 *  ALock.Cursor.Glyph.Name
 *  ALock.Cursor.Glyph.Background
 *  ALock.Cursor.Glyph.Foreground
 *
 */

#include "alock.h"

#include <stdlib.h>
#include <string.h>
#include <X11/cursorfont.h>


struct CursorFontName {
    const char *name;
    unsigned int shape;
};

static const struct CursorFontName cursor_names[] = {
    { "X_cursor", XC_X_cursor },
    { "arrow", XC_arrow },
    { "based_arrow_down", XC_based_arrow_down },
    { "based_arrow_up", XC_based_arrow_up },
    { "boat", XC_boat },
    { "bogosity", XC_bogosity },
    { "bottom_left_corner", XC_bottom_left_corner },
    { "bottom_right_corner", XC_bottom_right_corner },
    { "bottom_side", XC_bottom_side },
    { "bottom_tee", XC_bottom_tee },
    { "box_spiral", XC_box_spiral },
    { "center_ptr", XC_center_ptr },
    { "circle", XC_circle },
    { "clock", XC_clock },
    { "coffee_mug", XC_coffee_mug },
    { "cross", XC_cross },
    { "cross_reverse", XC_cross_reverse },
    { "crosshair", XC_crosshair },
    { "diamond_cross", XC_diamond_cross },
    { "dot", XC_dot },
    { "dotbox", XC_dotbox },
    { "double_arrow", XC_double_arrow },
    { "draft_large", XC_draft_large },
    { "draft_small", XC_draft_small },
    { "draped_box", XC_draped_box },
    { "exchange", XC_exchange },
    { "fleur", XC_fleur },
    { "gobbler", XC_gobbler },
    { "gumby", XC_gumby },
    { "hand1", XC_hand1 },
    { "hand2", XC_hand2 },
    { "heart", XC_heart },
    { "icon", XC_icon },
    { "iron_cross", XC_iron_cross },
    { "left_ptr", XC_left_ptr },
    { "left_side", XC_left_side },
    { "left_tee", XC_left_tee },
    { "leftbutton", XC_leftbutton },
    { "ll_angle", XC_ll_angle },
    { "lr_angle", XC_lr_angle },
    { "man", XC_man },
    { "middlebutton", XC_middlebutton },
    { "mouse", XC_mouse },
    { "pencil", XC_pencil },
    { "pirate", XC_pirate },
    { "plus", XC_plus },
    { "question_arrow", XC_question_arrow },
    { "right_ptr", XC_right_ptr },
    { "right_side", XC_right_side },
    { "right_tee", XC_right_tee },
    { "rightbutton", XC_rightbutton },
    { "rtl_logo", XC_rtl_logo },
    { "sailboat", XC_sailboat },
    { "sb_down_arrow", XC_sb_down_arrow },
    { "sb_h_double_arrow", XC_sb_h_double_arrow },
    { "sb_left_arrow", XC_sb_left_arrow },
    { "sb_right_arrow", XC_sb_right_arrow },
    { "sb_up_arrow", XC_sb_up_arrow },
    { "sb_v_double_arrow", XC_sb_v_double_arrow },
    { "shuttle", XC_shuttle },
    { "sizing", XC_sizing },
    { "spider", XC_spider },
    { "spraycan", XC_spraycan },
    { "star", XC_star },
    { "target", XC_target },
    { "tcross", XC_tcross },
    { "top_left_arrow", XC_top_left_arrow },
    { "top_left_corner", XC_top_left_corner },
    { "top_right_corner", XC_top_right_corner },
    { "top_side", XC_top_side },
    { "top_tee", XC_top_tee },
    { "trek", XC_trek },
    { "ul_angle", XC_ul_angle },
    { "umbrella", XC_umbrella },
    { "ur_angle", XC_ur_angle },
    { "watch", XC_watch },
    { "xterm", XC_xterm },
    { NULL, 0 }
};

/* default glyph: XC_X_cursor */
static struct moduleData {
    Display *display;
    char *colorname_bg;
    char *colorname_fg;
    unsigned int shape;
    Cursor cursor;
} data = { 0 };


static int module_set_cursor_by_name(const char *name) {

    const struct CursorFontName *cursor;

    for (cursor = cursor_names; cursor->name; cursor++) {
        if (!strcmp(cursor->name, name)) {
            data.shape = cursor->shape;
            return data.shape;
        }
    }

    return -1;
}

static void module_cmd_list(void) {

    printf("list of available cursor glyphs:\n");

    const struct CursorFontName *cursor;
    for (cursor = cursor_names; cursor->name; cursor++)
        printf("  %s\n", cursor->name);

}

static void module_loadargs(const char *args) {

    if (!args || strstr(args, "glyph:") != args)
        return;

    char *arguments = strdup(&args[6]);
    char *arg;
    char *tmp;

    for (tmp = arguments; tmp; ) {
        arg = strsep(&tmp, ",");
        if (strcmp(arg, "list") == 0) {
            module_cmd_list();
            exit(EXIT_SUCCESS);
        }
        if (strstr(arg, "name=") == arg) {
            module_set_cursor_by_name(&arg[5]);
        }
        else if (strstr(arg, "fg=") == arg) {
            free(data.colorname_fg);
            data.colorname_fg = strdup(&arg[3]);
        }
        else if (strstr(arg, "bg=") == arg) {
            free(data.colorname_bg);
            data.colorname_bg = strdup(&arg[3]);
        }
    }

    free(arguments);
}

static void module_loadxrdb(XrmDatabase xrdb) {

    XrmValue value;
    char *type;

    if (XrmGetResource(xrdb, "alock.cursor.glyph.name",
                "ALock.Cursor.Glyph.Name", &type, &value))
        module_set_cursor_by_name(value.addr);

    if (XrmGetResource(xrdb, "alock.cursor.glyph.foreground",
                "ALock.Cursor.Glyph.Foreground", &type, &value))
        data.colorname_fg = strdup(value.addr);

    if (XrmGetResource(xrdb, "alock.cursor.glyph.background",
                "ALock.Cursor.Glyph.Background", &type, &value))
        data.colorname_bg = strdup(value.addr);

}

static int module_init(Display *dpy) {

    Colormap colormap = DefaultColormap(dpy, DefaultScreen(dpy));
    XColor color_bg;
    XColor color_fg;

    data.display = dpy;
    alock_alloc_color(dpy, colormap, data.colorname_bg, "black", &color_bg);
    alock_alloc_color(dpy, colormap, data.colorname_fg, "white", &color_fg);

    /* create cursor from X11/cursorfont.h */
    if (!(data.cursor = XCreateFontCursor(dpy, data.shape))) {
        fprintf(stderr, "[glyph]: unable to create fontcursor\n");
        return -1;
    }

    XRecolorCursor(dpy, data.cursor, &color_fg, &color_bg);
    return 0;
}

static void module_free() {

    if (data.cursor)
        XFreeCursor(data.display, data.cursor);

    free(data.colorname_bg);
    data.colorname_bg = NULL;
    free(data.colorname_fg);
    data.colorname_fg = NULL;

}

static Cursor module_getcursor(void) {
    return data.cursor;
}

static const struct aCursorImage *module_getimage(void) { return NULL; }


struct aModuleCursor alock_cursor_glyph = {
    { "glyph",
        module_loadargs,
        module_loadxrdb,
        module_init,
        module_free,
    },
    module_getcursor,
    module_getimage,
};
