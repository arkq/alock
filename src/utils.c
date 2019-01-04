/*
 * alock - utils.c
 * Copyright (c) 2005 - 2007 Mathias Gumz <akira at fluxbox dot org>
 *               2014 - 2018 Arkadiusz Bokowy
 *
 * This file is a part of an alock.
 *
 * This project is licensed under the terms of the MIT license.
 *
 */

#include "alock.h"

#include <ctype.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/Xutil.h>
#if ENABLE_IMLIB2
# include <Imlib2.h>
#endif
#if ENABLE_XRENDER
# include <X11/extensions/Xrender.h>
#endif


/* Get system time-stamp in milliseconds without discontinuities. */
unsigned long alock_mtime() {
    struct timespec t;
#ifdef CLOCK_BOOTTIME
    clock_gettime(CLOCK_BOOTTIME, &t);
#else
#warning Suspend-aware monotonic clock not available!
    clock_gettime(CLOCK_MONOTONIC, &t);
#endif
    return t.tv_sec * 1000 + t.tv_nsec / 1000000;
}

/* Determine the Endianness of the system. */
int alock_native_byte_order() {
    int x = 1;
    return (*((char *) &x) == 1) ? LSBFirst : MSBFirst;
}

/* Allocate colormap entry by the given color name. When the color_name
 * parameter is NULL, then fallback value is used right away. */
int alock_alloc_color(Display *display,
        Colormap colormap,
        const char *color_name,
        const char *fallback_name,
        XColor *result) {

    if (!display || !colormap || !fallback_name || !result)
        return 0;

    XColor tmp;

    if (!color_name || XAllocNamedColor(display, colormap, color_name, &tmp, result) == 0)
        if (XAllocNamedColor(display, colormap, fallback_name, &tmp, result) == 0)
            return 0;
    return 1;
}

/* Check if the X server supports RENDER extension. */
int alock_check_xrender(Display *display) {
#if ENABLE_XRENDER
    static int checked = 0;
    static int available = 0;

    if (checked)
        return available;

    int tmp;
    checked = 1;

    if ((available = XRenderQueryExtension(display, &tmp, &tmp)) == False)
        fprintf(stderr, "alock: missing X Render Extension support\n");

    return available;
#else
    (void)display;
    return 0;
#endif /* ENABLE_XRENDER */
}

/* Shade given source pixmap by the amount specified by the shade parameter,
 * which should be in range [0, 100]. */
int alock_shade_pixmap(Display *display,
        Visual *visual,
        const Pixmap src_pm,
        Pixmap dst_pm,
        unsigned char shade,
        int src_x, int src_y,
        int dst_x, int dst_y,
        unsigned int width,
        unsigned int height) {
#if ENABLE_XRENDER
    Picture alpha_pic = None;
    XRenderPictFormat *format;

    {
        XRenderPictFormat alpha_format;
        alpha_format.type = PictTypeDirect;
        alpha_format.depth = 8;
        alpha_format.direct.alpha = 0;
        alpha_format.direct.alphaMask = 0xff;

        format = XRenderFindFormat(display,
              PictFormatType | PictFormatDepth | PictFormatAlpha | PictFormatAlphaMask,
              &alpha_format, 0);
    }

    if (!format) {
        fprintf(stderr, "alock: couldn't find valid format for alpha\n");
        XFreePixmap(display, dst_pm);
        XFreePixmap(display, src_pm);
        return 0;
    }

    { /* fill the alpha-picture */
        Pixmap pm;
        XRenderColor color;
        XRenderPictureAttributes pa;

        if (shade > 100)
            shade = 100;

        pa.repeat = True;
        color.alpha = 0xffff * shade / 100;

        pm = XCreatePixmap(display, src_pm, 1, 1, 8);
        alpha_pic = XRenderCreatePicture(display, pm, format, CPRepeat, &pa);
        XRenderFillRectangle(display, PictOpSrc, alpha_pic, &color, 0, 0, 1, 1);

        XFreePixmap(display, pm);
    }

    { /* blend all together */
        Picture src_pic;
        Picture dst_pic;

        format = XRenderFindVisualFormat(display, visual);
        src_pic = XRenderCreatePicture(display, src_pm, format, 0, 0);
        dst_pic = XRenderCreatePicture(display, dst_pm, format, 0, 0);

        XRenderComposite(display, PictOpOver,
                         src_pic, alpha_pic, dst_pic,
                         src_x, src_y, 0, 0, dst_x, dst_y, width, height);
        XRenderFreePicture(display, src_pic);
        XRenderFreePicture(display, dst_pic);
    }

    XRenderFreePicture(display, alpha_pic);
    return 1;
#else
    (void)display;
    (void)visual;
    return 0;
#endif /* ENABLE_XRENDER */
}

/* Blur given source pixmap using a Gaussian convolution filter. Whole
 * operation is performed by the X server and when possible hardware
 * accelerated. For the best results the blur parameter should be in the
 * range [0, 100]. */
int alock_blur_pixmap(Display *display,
        Visual *visual,
        const Pixmap src_pm,
        Pixmap dst_pm,
        unsigned char blur,
        int src_x, int src_y,
        int dst_x, int dst_y,
        unsigned int width,
        unsigned int height) {

    if (!blur)
        /* TODO: copy source pixmap to the destination one */
        return 1;

#if ENABLE_IMLIB2

    Imlib_Context ctx = imlib_context_new();

    imlib_context_push(ctx);
    imlib_context_set_display(display);
    imlib_context_set_visual(visual);

    imlib_context_set_drawable(src_pm);
    imlib_context_set_image(imlib_create_image_from_drawable(None, src_x, src_y, width, height, 0));

    debug("Blur size: %d", blur / 10);
    imlib_image_blur(blur / 10);

    imlib_context_set_drawable(dst_pm);
    imlib_render_image_on_drawable_at_size(dst_x, dst_y, width, height);

    imlib_context_pop();
    imlib_context_free(ctx);
    return 1;

#elif ENABLE_XRENDER

    /* NOTE: It seems that reasonable sigma value is between 0.5 and 4. This
     *       will translate into the blur up to 12 x 12 pixels wide - radius
     *       is like 3x times the sigma. */
    double sigma = (double)blur / 30 + 0.5;
    int radius = sigma * sqrt(2 * -log(1.0 / 255));
    int size = radius * 2 + 1;

    debug("Gaussian kernel size: %dx%d", size, size);
    XFixed *params = malloc(sizeof(XFixed) * (2 + size * size));

    { /* calculate sampled Gaussian kernel */
        double *kernel = malloc(sizeof(double) * size * size);
        double scale1 = - 1.0 / (2 * sigma * sigma);
        double scale2 = - scale1 / M_PI;
        double vsum = 0;
        int i, x, y;

        for (i = 0, x = -radius; x <= radius; x++)
            for (y = -radius; y <= radius; y++, i++) {
                kernel[i] = scale2 * exp(scale1 * (x * x + y * y));
                vsum += kernel[i];
            }

        params[0] = params[1] = XDoubleToFixed(size);
        for (i = 0; i < size * size; i++)
            params[i + 2] = XDoubleToFixed(kernel[i] / vsum);

        free(kernel);
    }

    { /* 2D blur using convolution filter */
        XRenderPictFormat *format;
        Picture src_pic;
        Picture dst_pic;

        format = XRenderFindVisualFormat(display, visual);
        src_pic = XRenderCreatePicture(display, src_pm, format, 0, NULL);
        dst_pic = XRenderCreatePicture(display, dst_pm, format, 0, NULL);

        XRenderSetPictureFilter(display, src_pic, FilterConvolution,
                                params, 2 + size * size);
        XRenderComposite(display, PictOpSrc, src_pic, None, dst_pic,
                         src_x, src_y, 0, 0, dst_x, dst_y, width, height);

        XRenderFreePicture(display, src_pic);
        XRenderFreePicture(display, dst_pic);
    }

    free(params);
    return 1;

#else
    (void)display;
    (void)visual;
    return 0;
#endif
}

/* Convert given color image to the grayscale intensity one. Note, that this
 * function performs in-place conversion. */
int alock_grayscale_image(XImage *image,
        int x, int y,
        unsigned int width,
        unsigned int height) {

    union {
        struct __attribute__ ((packed)) {
            uint16_t red   : 5;
            uint16_t green : 6;
            uint16_t blue  : 5;
        } v16;
        struct __attribute__ ((packed)) {
            uint8_t red;
            uint8_t green;
            uint8_t blue;
        } v24;
        unsigned long value;
    } pixel;

    int depth = image->depth;
    int _x, _y;

    if (depth != 16 && depth != 24) {
        fprintf(stderr, "alock: screen depth %d is not supported\n", depth);
        return 0;
    }

    /* NOTE: Color conversion is based on the colorimetric strategy. The
     *       principle is, that the luminance of the grayscale image should
     *       match the luminance of the original color image. */

    for (_x = x; _x < (signed)width; _x++)
        for (_y = y; _y < (signed)height; _y++) {
            pixel.value = XGetPixel(image, _x, _y);

            if (depth == 24)
                pixel.v24.red = pixel.v24.green = pixel.v24.blue =
                        0.2126 * pixel.v24.red +
                        0.7152 * pixel.v24.green +
                        0.0722 * pixel.v24.blue;
            else {
                pixel.v16.green =
                        0.2126 * (pixel.v16.red << 1) +
                        0.7152 * pixel.v16.green +
                        0.0722 * (pixel.v16.blue << 1);
                pixel.v16.red = pixel.v16.blue = pixel.v16.green >> 1;
            }

            XPutPixel(image, _x, _y, pixel.value);
        }

    return 1;
}

/* Dummy function for module interface. */
void module_dummy_loadargs(const char *args) {
    (void)args;
    debug("dummy loadargs: %s", args);
}

/* Dummy function for module interface. */
void module_dummy_loadxrdb(XrmDatabase database) {
    (void)database;
    debug("dummy loadxrdb");
}

/* Dummy function for module interface. */
int module_dummy_init(Display *display) {
    (void)display;
    debug("dummy init");
    return 0;
}

/* Dummy function for module interface. */
void module_dummy_free(void) {
    debug("dummy free");
}
