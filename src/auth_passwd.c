/*
 * alock - auth_passwd.c
 * Copyright (c) 2005 - 2007 Mathias Gumz <akira at fluxbox dot org>
 *               2014 - 2015 Arkadiusz Bokowy
 *
 * This file is a part of an alock.
 *
 * This project is licensed under the terms of the MIT license.
 *
 * This authentication module provides:
 *  -auth passwd
 *
 */

#define _XOPEN_SOURCE

#include "alock.h"

#include <string.h>
#include <crypt.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <sys/stat.h>
#include <sys/types.h>

#if __linux && HAVE_SHADOW_H
# include <shadow.h>
#endif


static struct passwd *pwd_entry = NULL;


static int module_init(Display *display) {
    (void)display;

    errno = 0;
    pwd_entry = getpwuid(getuid());
    if (!pwd_entry) {
        perror("[passwd]: password entry for uid not found");
        return -1;
    }

#if __linux && HAVE_SHADOW_H
    {
        struct spwd *sp = NULL;

        sp = getspnam(pwd_entry->pw_name);
        if (sp)
            pwd_entry->pw_passwd = sp->sp_pwdp;

    }
#endif

    if (strlen(pwd_entry->pw_passwd) < 13) {
        perror("[passwd]: password entry has no pwd");
        pwd_entry = NULL;
        return -1;
    }

    return 0;
}

static int module_authenticate(const char *pass) {

    char *cpw;

    if (pass == NULL || pwd_entry == NULL)
        return -1;

    cpw = crypt(pass, pwd_entry->pw_passwd);
    if (cpw == NULL)
        return -1;

    /* Simpler, and should work with crypt() algorithms using longer
     * salt strings (like the md5-based one on freebsd).  --marekm */
    return strcmp(cpw, pwd_entry->pw_passwd);
}


struct aModuleAuth alock_auth_passwd = {
    { "passwd",
        module_dummy_loadargs,
        module_dummy_loadxrdb,
        module_init,
        module_dummy_free,
    },
    module_authenticate,
};
