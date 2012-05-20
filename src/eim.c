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
const FcitxIMClass ime = {
    .Create = FcitxSayuraCreate,
    .Destroy = FcitxSayuraDestroy
};
FCITX_EXPORT_API
const int ABI_VERSION = FCITX_ABI_VERSION;

static const FcitxIMIFace sayura_iface = {
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

const UT_icd ucs4_icd = {
    sizeof(uint32_t),
    NULL,
    NULL,
    NULL
};


struct {
    uint32_t character;
    uint32_t mahaprana;
    uint32_t sagngnaka;
    FcitxKeySym key;
} consonents[] = {
    {0xda4, 0x00, 0x00, FcitxKey_z},
    {0xda5, 0x00, 0x00, FcitxKey_Z},
    {0xdc0, 0x00, 0x00, FcitxKey_w},
    {0x200c, 0x00, 0x00, FcitxKey_W},
    {0xdbb, 0x00, 0x00, FcitxKey_r},
    {0xdbb, 0x00, 0x00, FcitxKey_R},
    {0xdad, 0xdae, 0x00, FcitxKey_t},
    {0xda7, 0xda8, 0x00, FcitxKey_T},
    {0xdba, 0x00, 0x00, FcitxKey_y},
    {0xdba, 0x00, 0x00, FcitxKey_Y},
    {0xdb4, 0xdb5, 0x00, FcitxKey_p},
    {0xdb5, 0xdb5, 0x00, FcitxKey_P},
    {0xdc3, 0xdc2, 0x00, FcitxKey_s},
    {0xdc1, 0xdc2, 0x00, FcitxKey_S},
    {0xdaf, 0xdb0, 0xdb3, FcitxKey_d},
    {0xda9, 0xdaa, 0xdac, FcitxKey_D},
    {0xdc6, 0x00, 0x00, FcitxKey_f},
    {0xdc6, 0x00, 0x00, FcitxKey_F},
    {0xd9c, 0xd9d, 0xd9f, FcitxKey_g},
    {0xd9f, 0xd9d, 0x00, FcitxKey_G},
    {0xdc4, 0xd83, 0x00, FcitxKey_h},
    {0xdc4, 0x00, 0x00, FcitxKey_H},
    {0xda2, 0xda3, 0xda6, FcitxKey_j},
    {0xda3, 0xda3, 0xda6, FcitxKey_J},
    {0xd9a, 0xd9b, 0x00, FcitxKey_k},
    {0xd9b, 0xd9b, 0x00, FcitxKey_K},
    {0xdbd, 0x00, 0x00, FcitxKey_l},
    {0xdc5, 0x00, 0x00, FcitxKey_L},
    {0xd82, 0x00, 0x00, FcitxKey_x},
    {0xd9e, 0x00, 0x00, FcitxKey_X},
    {0xda0, 0xda1, 0x00, FcitxKey_c},
    {0xda1, 0xda1, 0x00, FcitxKey_C},
    {0xdc0, 0x00, 0x00, FcitxKey_v},
    {0xdc0, 0x00, 0x00, FcitxKey_V},
    {0xdb6, 0xdb7, 0xdb9, FcitxKey_b},
    {0xdb7, 0xdb7, 0xdb9, FcitxKey_B},
    {0xdb1, 0x00, 0xd82, FcitxKey_n},
    {0xdab, 0x00, 0xd9e, FcitxKey_N},
    {0xdb8, 0x00, 0x00, FcitxKey_m},
    {0xdb9, 0x00, 0x00, FcitxKey_M},
    {0, 0, 0, 0}
};

struct {
    uint32_t single0;
    uint32_t double0;
    uint32_t single1;
    uint32_t double1;
    FcitxKeySym key;
} vowels[] = {
    {0xd85, 0xd86, 0xdcf, 0xdcf, FcitxKey_a},
    {0xd87, 0xd88, 0xdd0, 0xdd1, FcitxKey_A},
    {0xd87, 0xd88, 0xdd0, 0xdd1, FcitxKey_q},
    {0xd91, 0xd92, 0xdd9, 0xdda, FcitxKey_e},
    {0xd91, 0xd92, 0xdd9, 0xdda, FcitxKey_E},
    {0xd89, 0xd8a, 0xdd2, 0xdd3, FcitxKey_i},
    {0xd93, 0x00, 0xddb, 0xddb, FcitxKey_I},
    {0xd94, 0xd95, 0xddc, 0xddd, FcitxKey_o},
    {0xd96, 0x00, 0xdde, 0xddf, FcitxKey_O},
    {0xd8b, 0xd8c, 0xdd4, 0xdd6, FcitxKey_u},
    {0xd8d, 0xd8e, 0xdd8, 0xdf2, FcitxKey_U},
    {0xd8f, 0xd90, 0xd8f, 0xd90, FcitxKey_Z},
    {0, 0, 0, 0, 0}
};


#ifdef FCITX_SAYURA_DEBUG
#  define __pfunc__() printf("%s\n", __func__)
#else
#  define __pfunc__()
#endif

/**
 * @brief initialize the extra input method
 *
 * @param arg
 * @return successful or not
 **/
static void*
FcitxSayuraCreate(FcitxInstance *instance)
{
    FcitxSayura *sayura = (FcitxSayura*)
        fcitx_utils_malloc0(sizeof(FcitxSayura));
    /* FcitxGlobalConfig *config = FcitxInstanceGetGlobalConfig(instance); */
    /* FcitxInputState *input = FcitxInstanceGetInputState(instance); */
    __pfunc__();
    bindtextdomain("fcitx-sayura", LOCALEDIR);

    sayura->owner = instance;
    utarray_new(sayura->buff, &ucs4_icd);
    FcitxInstanceRegisterIMv2(instance, sayura,
                              "sayura", _("Sinhala Input Method"), "sayura",
                              sayura_iface, 1, "si");
    return sayura;
}

static void
FcitxSayuraDestroy(void *arg)
{
    FcitxSayura *sayura = (FcitxSayura*)arg;
    __pfunc__();
    utarray_free(sayura->buff);
    free(arg);
}

static boolean
FcitxSayuraInit(void *arg)
{
    FcitxSayura *sayura = (FcitxSayura*)arg;
    __pfunc__();
    FcitxInstanceSetContext(sayura->owner, CONTEXT_IM_KEYBOARD_LAYOUT, "us");
    return true;
}

static INPUT_RETURN_VALUE
FcitxSayuraDoInput(void *arg, FcitxKeySym sym, unsigned int state)
{
    __pfunc__();
    return IRV_TO_PROCESS;
}

static INPUT_RETURN_VALUE
FcitxSayuraGetCandWords(void *arg)
{
    __pfunc__();
    return IRV_TO_PROCESS;
}

static void
FcitxSayuraReset(void *arg)
{
    FcitxSayura *sayura = (FcitxSayura*)arg;
    __pfunc__();
    utarray_clear(sayura->buff);
}
