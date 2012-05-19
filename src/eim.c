/***************************************************************************
 *   Copyright (C) 2012~2012 by Yichao Yu                                  *
 *   yyc1992@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.              *
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcitx/ime.h>
#include <fcitx-config/fcitx-config.h>
#include <fcitx-config/xdg.h>
#include <fcitx-config/hotkey.h>
#include <fcitx-utils/log.h>
#include <fcitx-utils/utils.h>
#include <fcitx-utils/utf8.h>
#include <fcitx/instance.h>
#include <fcitx/context.h>
#include <fcitx/keys.h>
#include <fcitx/ui.h>
#include <libintl.h>

#include "eim.h"
#include "config.h"

static void *FcitxSayuraCreate(FcitxInstance *instance);
static void FcitxSayuraDestroy(void *arg);
static boolean FcitxSayuraInit(void *arg);
static INPUT_RETURN_VALUE FcitxSayuraDoInput(void *arg, FcitxKeySym sym,
                                             unsigned int state);
static INPUT_RETURN_VALUE FcitxSayuraGetCandWords(void *arg);
static void FcitxSayuraReset(void *arg);

FCITX_EXPORT_API
FcitxIMClass ime = {
    .Create = FcitxSayuraCreate,
    .Destroy = FcitxSayuraDestroy
};
FCITX_EXPORT_API
int ABI_VERSION = FCITX_ABI_VERSION;

static FcitxIMIFace sayura_iface = {
    .Init = FcitxSayuraInit,
    .ResetIM = FcitxSayuraReset,
    .DoInput = FcitxSayuraDoInput,
    .GetCandWords = FcitxSayuraGetCandWords,
    .PhraseTips = NULL,
    .Save = NULL,
    .ReloadConfig = NULL,
    .KeyBlocker = NULL,
    .UpdateSurroundingText = NULL,
    .DoReleaseInput = NULL,
};

/**
 * @brief initialize the extra input method
 *
 * @param arg
 * @return successful or not
 **/
static void*
FcitxSayuraCreate(FcitxInstance *instance)
{
    static boolean initialized = false;
    FcitxSayura *sayura = (FcitxSayura*)
        fcitx_utils_malloc0(sizeof(FcitxSayura));
    FcitxGlobalConfig *config = FcitxInstanceGetGlobalConfig(instance);
    FcitxInputState *input = FcitxInstanceGetInputState(instance);

    if (!initialized) {
        bindtextdomain("fcitx-sayura", LOCALEDIR);
        initialized = true;
    }

    sayura->owner = instance;
    FcitxInstanceRegisterIMv2(instance, sayura,
                              "sayura", _("Sayura"), "sayura",
                              sayura_iface, 1, "si");
    return sayura;
}

static void
FcitxSayuraDestroy(void *arg)
{
    FcitxSayura *sayura = (FcitxSayura*)arg;
    free(arg);
}

static boolean
FcitxSayuraInit(void *arg)
{
    return true;
}

static INPUT_RETURN_VALUE
FcitxSayuraDoInput(void *arg, FcitxKeySym sym, unsigned int state)
{
    return IRV_TO_PROCESS;
}

static INPUT_RETURN_VALUE
FcitxSayuraGetCandWords(void *arg)
{
    return IRV_TO_PROCESS;
}

static void
FcitxSayuraReset(void *arg)
{

}
