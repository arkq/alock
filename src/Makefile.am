# alock - Makefile.am
# Copyright (c) 2014 - 2016 Arkadiusz Bokowy

bin_PROGRAMS = alock

alock_SOURCES = \
	auth_none.c \
	input_none.c \
	input_frame.c \
	bg_none.c \
	bg_blank.c \
	cursor_none.c \
	cursor_blank.c \
	cursor_glyph.c \
	utils.c \
	main.c

alock_CFLAGS = \
	@X11_CFLAGS@ \
	@XCURSOR_CFLAGS@ \
	@XEXT_CFLAGS@ \
	@XPM_CFLAGS@ \
	@XRENDER_CFLAGS@ \
	@IMLIB2_CFLAGS@

alock_LDADD = \
	@X11_LIBS@ \
	@XCURSOR_LIBS@ \
	@XEXT_LIBS@ \
	@XPM_LIBS@ \
	@XRENDER_LIBS@ \
	@IMLIB2_LIBS@

if ENABLE_PAM
alock_SOURCES += auth_pam.c
endif
if ENABLE_PASSWD
alock_SOURCES += auth_passwd.c
endif
if ENABLE_HASH
alock_SOURCES += auth_hash.c
endif

if ENABLE_XRENDER
alock_SOURCES += bg_shade.c
endif
if ENABLE_IMLIB2
alock_SOURCES += bg_image.c
if ENABLE_XRENDER
alock_SOURCES += cursor_image.c
endif
else
if ENABLE_XPM
alock_SOURCES += cursor_image.c
endif
endif
if ENABLE_XCURSOR
alock_SOURCES += cursor_xcursor.c
endif
