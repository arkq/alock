# alock - configure.ac
# Copyright (c) 2014 - 2021 Arkadiusz Bokowy

AC_PREREQ([2.59])
AC_INIT([alock], [2.3.1], [arkadiusz.bokowy@gmail.com])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_CONFIG_HEADERS([config.h])

AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

# support for debugging
AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug], [enable debugging support])])
AM_CONDITIONAL([ENABLE_DEBUG], [test "x$enable_debug" = "xyes"])
AM_COND_IF([ENABLE_DEBUG], [
	AC_DEFINE([DEBUG], [1], [Define to 1 if debugging is enabled.])
])

AC_SEARCH_LIBS([clock_gettime], [rt])
PKG_CHECK_MODULES([X11], [x11])

# check for the Misc X Extension library
PKG_CHECK_MODULES([XEXT], [xext],
	[AC_DEFINE([HAVE_XEXT], [1], [Define to 1 if you have X Ext library.])],
	[#skip])

# support for the PAM library
AC_ARG_ENABLE([pam],
	[AS_HELP_STRING([--enable-pam], [enable PAM support])])
AM_CONDITIONAL([ENABLE_PAM], [test "x$enable_pam" = "xyes"])
AM_COND_IF([ENABLE_PAM], [
	AC_CHECK_HEADERS([security/pam_appl.h],
		[], [AC_MSG_ERROR([pam_appl.h header not found])])
	AC_SEARCH_LIBS([pam_start], [pam],
		[], [AC_MSG_ERROR([pam library not found])])
	AC_DEFINE([ENABLE_PAM], [1], [Define to 1 if PAM is enabled.])
])

# support for default (build-in) PAM service customization
AC_ARG_VAR([PAM_DEFAULT_SERVICE], [default (build-in) PAM service])
AS_IF([test "x$PAM_DEFAULT_SERVICE" = "x"], [PAM_DEFAULT_SERVICE="system-auth"])
AC_DEFINE_UNQUOTED([PAM_DEFAULT_SERVICE], ["$PAM_DEFAULT_SERVICE"], [Default PAM service.])

# support for the passwd (/etc/shadow)
AC_ARG_ENABLE([passwd],
	[AS_HELP_STRING([--enable-passwd], [enable passwd support])])
AM_CONDITIONAL([ENABLE_PASSWD], [test "x$enable_passwd" = "xyes"])
AM_COND_IF([ENABLE_PASSWD], [
	AC_CHECK_HEADERS([shadow.h],
		[], [AC_MSG_ERROR([shadow.h header not found])])
	AC_SEARCH_LIBS([crypt], [crypt],
		[], [AC_MSG_ERROR([crypt library not found])])
	AC_DEFINE([ENABLE_PASSWD], [1], [Define to 1 if passwd is enabled.])
])

# support for the gcrypt library
AC_ARG_ENABLE([hash],
	[AS_HELP_STRING([--enable-hash], [enable hash support])])
AM_CONDITIONAL([ENABLE_HASH], [test "x$enable_hash" = "xyes"])
AM_COND_IF([ENABLE_HASH], [
	AC_CHECK_HEADERS([gcrypt.h],
		[], [AC_MSG_ERROR([gcrypt.h header not found])])
	AC_SEARCH_LIBS([gcry_md_open], [gcrypt],
		[], [AC_MSG_ERROR([gcrypt library not found])])
	AC_DEFINE([ENABLE_HASH], [1], [Define to 1 if hash is enabled.])
])

# support for the X Render library
AC_ARG_ENABLE([xrender],
	[AS_HELP_STRING([--enable-xrender], [enable Xrender support])])
AM_CONDITIONAL([ENABLE_XRENDER], [test "x$enable_xrender" = "xyes"])
AM_COND_IF([ENABLE_XRENDER], [
	PKG_CHECK_MODULES([XRENDER], [xrender])
	AC_SEARCH_LIBS([exp], [m],
		[], [AC_MSG_ERROR([math library not found])])
	AC_DEFINE([ENABLE_XRENDER], [1], [Define to 1 if Xrender is enabled.])
])

# support for the X Cursor library
AC_ARG_ENABLE([xcursor],
	[AS_HELP_STRING([--enable-xcursor], [enable Xcursor support])])
AM_CONDITIONAL([ENABLE_XCURSOR], [test "x$enable_xcursor" = "xyes"])
AM_COND_IF([ENABLE_XCURSOR], [
	PKG_CHECK_MODULES([XCURSOR], [xcursor])
	AC_DEFINE([ENABLE_XCURSOR], [1], [Define to 1 if Xcursor is enabled.])
])

# support for the X Pixmap library
AC_ARG_ENABLE([xpm],
	[AS_HELP_STRING([--enable-xpm], [enable Xpm support])])
AM_CONDITIONAL([ENABLE_XPM], [test "x$enable_xpm" = "xyes"])
AM_COND_IF([ENABLE_XPM], [
	PKG_CHECK_MODULES([XPM], [xpm])
	AC_DEFINE([ENABLE_XPM], [1], [Define to 1 if Xpm is enabled.])
])

# support for the Imlib2 library
AC_ARG_ENABLE([imlib2],
	[AS_HELP_STRING([--enable-imlib2], [enable Imlib2 support])])
AM_CONDITIONAL([ENABLE_IMLIB2], [test "x$enable_imlib2" = "xyes"])
AM_COND_IF([ENABLE_IMLIB2], [
	PKG_CHECK_MODULES([IMLIB2], [imlib2])
	AC_DEFINE([ENABLE_IMLIB2], [1], [Define to 1 if Imlib2 is enabled.])
])

# disable dunst notification-daemon while screen is locked
AC_ARG_WITH([dunst],
	[AS_HELP_STRING([--with-dunst], [notification-daemon integration])])
AM_CONDITIONAL([WITH_DUNST], [test "x$with_dunst" = "xyes"])
AM_COND_IF([WITH_DUNST], [
	AC_DEFINE([WITH_DUNST], [1], [Define to 1 if dunst integration is enabled.])
])

# dim display using xbacklight while screen is locked
AC_ARG_WITH([xbacklight],
	[AS_HELP_STRING([--with-xbacklight], [xbacklight integration])])
AM_CONDITIONAL([WITH_XBLIGHT], [test "x$with_xbacklight" = "xyes"])
AM_COND_IF([WITH_XBLIGHT], [
	AC_DEFINE([WITH_XBLIGHT], [1], [Define to 1 if xbacklight integration is enabled.])
])

AC_CONFIG_FILES([Makefile doc/Makefile src/Makefile])
AC_OUTPUT

# warn user when the debugging mode is enabled
AM_COND_IF([ENABLE_DEBUG], [
	AC_MSG_WARN([              *** debug mode enabled ***])
	AC_MSG_WARN([In the debug mode for every pressed key a corresponding])
	AC_MSG_WARN([character is printed to the stderr. This behavior might])
	AC_MSG_WARN([compromise your password!])
	AC_MSG_WARN([Do NOT treat this configuration as a release one!])
])
