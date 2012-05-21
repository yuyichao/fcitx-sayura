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

static const UT_icd ucs4_icd = {
    sizeof(uint32_t),
    NULL,
    NULL,
    NULL
};

#define ucs4_array_index(a, i)                                  \
    ({uint32_t *p = (uint32_t*)utarray_eltptr(a, i);            \
        p ? *p : 0;})

#define ucs4_array_last(a)                                  \
    ({uint32_t *p = (uint32_t*)utarray_back(a);            \
        p ? *p : 0;})

typedef struct {
    uint32_t character;
    uint32_t mahaprana;
    uint32_t sagngnaka;
    FcitxKeySym key;
} FcitxSayuraConsonant;
static const FcitxSayuraConsonant const consonants[] = {
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

typedef struct {
    uint32_t single0;
    uint32_t double0;
    uint32_t single1;
    uint32_t double1;
    FcitxKeySym key;
} FcitxSayuraVowel;
static const FcitxSayuraVowel vowels[] = {
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
    /* This one already exists in consonants[], not sure y it is also here.~ */
    {0xd8f, 0xd90, 0xd8f, 0xd90, FcitxKey_Z},
    {0, 0, 0, 0, 0}
};

#ifdef FCITX_SAYURA_DEBUG
#  define eprintf(format, args...)                      \
    fprintf(stderr, "\e[35m\e[1m"format"\e[0m", ##args)
#else
#  define eprintf(format, args...)
#endif

#  define __pfunc__() eprintf("%s\n", __func__)

static void*
FcitxSayuraCreate(FcitxInstance *instance)
{
    FcitxSayura *sayura = (FcitxSayura*)
        fcitx_utils_malloc0(sizeof(FcitxSayura));
    /* FcitxGlobalConfig *config = FcitxInstanceGetGlobalConfig(instance); */
    /* FcitxInputState *input = FcitxInstanceGetInputState(instance); */
    __pfunc__();
    bindtextdomain("fcitx-sayura", LOCALEDIR);

    sayura->hack = 0;
    sayura->owner = instance;
    utarray_init(&sayura->buff, &ucs4_icd);
    sayura->cd = iconv_open("UTF-8", "UTF-32");
    if (sayura->cd == (iconv_t)-1) {
        /* not necessary but not likely either.~ */
        utarray_done(&sayura->buff);
        free(sayura);
        return NULL;
    }

    /* A structure larger than 63byte is copied, good job. ~~ */
    FcitxInstanceRegisterIMv2(instance, sayura,
                              "sayura", _("Sinhala (Sayura)"), "sayura",
                              sayura_iface, 1, "si");
    return sayura;
}

static void
FcitxSayuraDestroy(void *arg)
{
    FcitxSayura *sayura = (FcitxSayura*)arg;
    __pfunc__();
    if (!arg)
        return;
    iconv_close(sayura->cd);
    utarray_done(&sayura->buff);
    free(arg);
}

/**
 * @brief initialize the extra input method
 *
 * @param arg
 * @return successful or not
 **/
static boolean
FcitxSayuraInit(void *arg)
{
    FcitxSayura *sayura = (FcitxSayura*)arg;
    __pfunc__();
    if (!arg)
        return false;
    sayura->hack = 0;
    FcitxInstanceSetContext(sayura->owner, CONTEXT_IM_KEYBOARD_LAYOUT, "us");
    return true;
}

static char*
FcitxSayuraBufferToUTF8(FcitxSayura *sayura)
{
    IconvStr inbuf = utarray_front(&sayura->buff);
    size_t len = utarray_len(&sayura->buff);
    size_t in_l = len * sizeof(uint32_t);
    size_t out_l = len * UTF8_MAX_LENGTH + 1;
    char *outbuf = fcitx_utils_malloc0(out_l);
    char *str = outbuf;

    size_t count = iconv(sayura->cd, &inbuf, &in_l, &outbuf, &out_l);

    eprintf("%s %d\n", str, (int)count);

    if (count == (size_t)-1) {
        /* Should never fail. */
        FcitxLog(FATAL, "Error when converting buffer string from UTF-32"
                 "to UTF-8.");
        free(str);
        return NULL;
    }
    return str;
}

static void
FcitxSayuraCommitPreedit(FcitxSayura *sayura)
{
    char *str = FcitxSayuraBufferToUTF8(sayura);
    utarray_clear(&sayura->buff);
    FcitxInstanceCommitString(sayura->owner,
                              FcitxInstanceGetCurrentIC(sayura->owner), str);
    free(str);
}

static int
FcitxSayuraFindConsonantByKey(FcitxKeySym sym)
{
    int i;
    for (i = 0;consonants[i].key;i++) {
        if (consonants[i].key == sym)
            return i;
    }
    return -1;
}

static int
FcitxSayuraFindVowelByKey(FcitxKeySym sym)
{
    int i;
    for (i = 0;vowels[i].key;i++) {
        if (vowels[i].key == sym)
            return i;
    }
    return -1;
}

static int
FcitxSayuraFindConsonant(uint32_t c)
{
    int i;
    for (i = 0;consonants[i].key;i++) {
        if (consonants[i].character == c||
            consonants[i].mahaprana == c||
            consonants[i].sagngnaka == c)
            return i;
    }
    return -1;
}

static boolean
FcitxSayuraIsConsonant(uint32_t c)
{
    return (c >= 0x0d9a) && (c <= 0x0dc6);
}

static INPUT_RETURN_VALUE
FcitxSayuraHandleConsonantPressed(FcitxSayura *sayura, int c)
{
    const FcitxSayuraConsonant consonant = consonants[c];
    int l1 = 0;
    uint32_t val = 0;

    if (utarray_len(&sayura->buff) == 0) {
        utarray_push_back(&sayura->buff, &consonant.character);
        return IRV_DISPLAY_CANDWORDS;
    }

    l1 = FcitxSayuraFindConsonant(ucs4_array_index(&sayura->buff, 0));

    if (l1 >= 0) {
        switch (consonant.key) {
        case FcitxKey_w:
            val = 0x0dca;
            utarray_push_back(&sayura->buff, &val);
            return IRV_DISPLAY_CANDWORDS;
        case FcitxKey_W:
            val = 0x0dca;
            utarray_push_back(&sayura->buff, &val);
            FcitxSayuraCommitPreedit(sayura);
            val = 0x200d;
            utarray_push_back(&sayura->buff, &val);
            return IRV_DISPLAY_CANDWORDS;
        case FcitxKey_H:
            if (!consonants[l1].mahaprana)
                break;
            if (!utarray_len(&sayura->buff))
                return IRV_DO_NOTHING;
            utarray_pop_back(&sayura->buff);
            utarray_push_back(&sayura->buff, &consonants[l1].mahaprana);
            return IRV_DISPLAY_CANDWORDS;
        case FcitxKey_G:
            if (!consonants[l1].sagngnaka)
                break;
            if (!utarray_len(&sayura->buff))
                return IRV_DO_NOTHING;
            utarray_pop_back(&sayura->buff);
            utarray_push_back(&sayura->buff, &consonants[l1].sagngnaka);
            return IRV_DISPLAY_CANDWORDS;
        case FcitxKey_R:
            val = 0x0dca;
            utarray_push_back(&sayura->buff, &val);
            val = 0x200d;
            utarray_push_back(&sayura->buff, &val);
            FcitxSayuraCommitPreedit(sayura);
            val = 0x0dbb;
            utarray_push_back(&sayura->buff, &val);
            return IRV_DISPLAY_CANDWORDS;
        case FcitxKey_Y:
            val = 0x0dca;
            utarray_push_back(&sayura->buff, &val);
            val = 0x200d;
            utarray_push_back(&sayura->buff, &val);
            FcitxSayuraCommitPreedit(sayura);
            val = 0x0dba;
            utarray_push_back(&sayura->buff, &val);
            return IRV_DISPLAY_CANDWORDS;
        default:
            break;
        }
    }

    FcitxSayuraCommitPreedit(sayura);
    utarray_push_back(&sayura->buff, &consonant.character);
    return IRV_DISPLAY_CANDWORDS;
}


static INPUT_RETURN_VALUE
FcitxSayuraHandleVowelPressed(FcitxSayura *sayura, int c)
{
    const FcitxSayuraVowel vowel = vowels[c];
    uint32_t c1 = 0;
    uint32_t val = 0;

    if (utarray_len(&sayura->buff) == 0) {
        utarray_push_back(&sayura->buff, &vowel.single0);
        return IRV_DISPLAY_CANDWORDS;
    }

    c1 = ucs4_array_last(&sayura->buff);

    if (FcitxSayuraIsConsonant(c1)) {
        utarray_push_back(&sayura->buff, &vowel.single1);
    } else if (c1 == vowel.single0) {
        utarray_pop_back(&sayura->buff);
        utarray_push_back(&sayura->buff, &vowel.double0);
    } else if (c1 == vowel.single1) {
        utarray_pop_back(&sayura->buff);
        utarray_push_back(&sayura->buff, &vowel.double1);
    } else if ((c1 == 0x0d86 || c1 == 0x0d87) && c == 0x0) {
        utarray_pop_back(&sayura->buff);
        val = vowel.single0 + 1;
        utarray_push_back(&sayura->buff, &val);
    }
    return IRV_DISPLAY_CANDWORDS;
}

static INPUT_RETURN_VALUE
FcitxSayuraDoInput(void *arg, FcitxKeySym sym, unsigned int state)
{
    FcitxSayura *sayura = (FcitxSayura*)arg;
    int c;
    __pfunc__();
    eprintf("sym: %d, state: %d\n", sym, state);

    sayura->hack = 0;
    if (FcitxHotkeyIsHotKey(sym, state, FCITX_ESCAPE))
        return IRV_FLAG_RESET_INPUT;

    if (FcitxHotkeyIsHotKey(sym, state, FCITX_BACKSPACE)) {
        if (utarray_len(&sayura->buff) > 0) {
            utarray_pop_back(&sayura->buff);
            return IRV_DISPLAY_CANDWORDS;
        }
        return IRV_TO_PROCESS;
    }

    if (FcitxHotkeyIsHotKey(sym, state, FCITX_SPACE)) {
        if (utarray_len(&sayura->buff) > 0) {
            FcitxSayuraCommitPreedit(sayura);
            /* This is ibus-sayura's behavior */
            sayura->hack |= FCITX_SAYURA_HACK_FORWARD;
            return IRV_DISPLAY_CANDWORDS;
        }
        return IRV_TO_PROCESS;
    }

    /* Hopefully this is the right thing to do~ */
    if (state)
        return IRV_TO_PROCESS;

    c = FcitxSayuraFindConsonantByKey(sym);
    if (c >= 0)
        return FcitxSayuraHandleConsonantPressed(sayura, c);

    c = FcitxSayuraFindVowelByKey(sym);
    if (c >= 0)
        return FcitxSayuraHandleVowelPressed(sayura, c);

    FcitxSayuraCommitPreedit(sayura);
    sayura->hack |= FCITX_SAYURA_HACK_FORWARD;
    return IRV_DISPLAY_CANDWORDS;
}

static INPUT_RETURN_VALUE
FcitxSayuraGetCandWords(void *arg)
{
    FcitxSayura* sayura = (FcitxSayura*) arg;
    FcitxInputState *input = FcitxInstanceGetInputState(sayura->owner);
    /* FcitxMessages *msgPreedit = FcitxInputStateGetPreedit(input); */
    FcitxMessages *clientPreedit = FcitxInputStateGetClientPreedit(input);
    __pfunc__();

    //clean up window asap
    FcitxInstanceCleanInputWindow(sayura->owner);

    char *preedit = FcitxSayuraBufferToUTF8(sayura);
    FcitxMessagesAddMessageAtLast(clientPreedit, MSG_INPUT, "%s", preedit);
    //FcitxMessagesMessageConcatLast(clientPreedit, "a");
    free(preedit);

    INPUT_RETURN_VALUE ret = IRV_DISPLAY_CANDWORDS;
    if (sayura->hack & FCITX_SAYURA_HACK_FORWARD)
        ret |= IRV_DONOT_PROCESS;

    sayura->hack = 0;
    return ret;
}

static void
FcitxSayuraReset(void *arg)
{
    FcitxSayura *sayura = (FcitxSayura*)arg;
    __pfunc__();
    sayura->hack = 0;
    if (!arg)
        return;
    utarray_clear(&sayura->buff);
}
