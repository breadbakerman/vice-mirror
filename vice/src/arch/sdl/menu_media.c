/*
 * menu_media.c - SDL media saving menu
 *
 * Written by
 *  Bas Wassink <b.wassink@ziggo.nl>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

/** \file   src/arch/sdl/menu_media.c
 * \brief   Media submenu for SDL
 *
 * Organizes media recording into screenshot, sound and video.
 */

#include "vice.h"

#include "machine.h"
#include "uimenu.h"

#include "lib.h"
#include "menu_common.h"
#include "menu_ffmpeg.h"
#include "menu_screenshot.h"
#include "menu_sound.h"
#include "menu_media.h"
#include "resources.h"
#include "sound.h"
#include "uifilereq.h"
#include "util.h"

static UI_MENU_CALLBACK(start_recording_callback)
{
    char *parameter = (char *)param;

    if (activated) {
        resources_set_string("SoundRecordDeviceName", "");
        if (parameter != NULL) {
            char *name = NULL;

            name = sdl_ui_file_selection_dialog("Choose audio file to record to", FILEREQ_MODE_CHOOSE_FILE);
            if (name != NULL) {
                util_add_extension(&name, parameter);
                resources_set_string("SoundRecordDeviceArg", name);
                resources_set_string("SoundRecordDeviceName", parameter);
                lib_free(name);
            }
        }
    } else {
        if (parameter != NULL) {
            const char *w;

            resources_get_string("SoundRecordDeviceName", &w);
            if (!strcmp(w, parameter)) {
                return sdl_menu_text_tick;
            }
        }
    }
    return NULL;
}

static ui_menu_entry_t sound_record_dyn_menu[SOUND_DEVICE_RECORD_MAX + 1];

static int sound_record_dyn_menu_init = 0;

static void sdl_menu_sound_record_free(void)
{
    int i;

    for (i = 0; sound_record_dyn_menu[i].string != NULL; i++) {
        lib_free(sound_record_dyn_menu[i].string);
        lib_free(sound_record_dyn_menu[i].data);
    }
}

static UI_MENU_CALLBACK(SoundRecord_dynmenu_callback)
{
    sound_desc_t *devices = sound_get_valid_devices(SOUND_RECORD_DEVICE, 1);
    int i;

    /* rebuild menu if it already exists. */
    if (sound_record_dyn_menu_init != 0) {
        sdl_menu_sound_record_free();
    } else {
        sound_record_dyn_menu_init = 1;
    }

    for (i = 0; devices[i].name; ++i) {
        sound_record_dyn_menu[i].string = (char *)util_concat("Start a ", devices[i].description, NULL);
        sound_record_dyn_menu[i].type = MENU_ENTRY_DIALOG;
        sound_record_dyn_menu[i].callback = start_recording_callback;
        sound_record_dyn_menu[i].data = (ui_callback_data_t)lib_strdup(devices[i].name);
    }

    sound_record_dyn_menu[i].string = NULL;
    sound_record_dyn_menu[i].type = 0;
    sound_record_dyn_menu[i].callback = NULL;
    sound_record_dyn_menu[i].data = NULL;

    lib_free(devices);

    return MENU_SUBMENU_STRING;
}

/** \brief  Generic media menu */
ui_menu_entry_t media_menu[] = {
    { "Create screenshot",
        MENU_ENTRY_SUBMENU,
        submenu_callback,
        NULL },             /* set by uimedia_menu_create() */
    { "Create sound recording",
        MENU_ENTRY_SUBMENU,
        SoundRecord_dynmenu_callback,
        (ui_callback_data_t)sound_record_dyn_menu },
#ifdef HAVE_FFMPEG
    { "Create video recording",
        MENU_ENTRY_SUBMENU,
        submenu_callback,
        (ui_callback_data_t)ffmpeg_menu },
#endif
    SDL_MENU_LIST_END
};


/** \brief  Set proper screenshot submenu, depending on machine
 *
 * \return  0 (OK)
 */
int uimedia_menu_create(void)
{
    switch (machine_class) {

        /* VIC/VICII/VDC */
        case VICE_MACHINE_C64:      /* fallthrough */
        case VICE_MACHINE_C64SC:    /* fallthrough */
        case VICE_MACHINE_C64DTV:   /* fallthrough */
        case VICE_MACHINE_C128:     /* fallthrough */
        case VICE_MACHINE_VIC20:    /* fallthrough */
        case VICE_MACHINE_SCPU64:   /* fallthrough */
        case VICE_MACHINE_CBM5x0:
            /* here be dragons: change the index if changing the order of the
             * items in `media_menu`
             */
            media_menu[0].data = (ui_callback_data_t)screenshot_vic_vicii_vdc_menu;
            break;

        /* CRTC */
        case VICE_MACHINE_PET:      /* fallthrough */
        case VICE_MACHINE_CBM6x0:
            /* again: nuclear missiles might launch if changing this */
            media_menu[0].data = (ui_callback_data_t)screenshot_crtc_menu;
            break;

        /* TED */
        case VICE_MACHINE_PLUS4:
            /* your C64 might change into a speccy if you change this */
            media_menu[0].data = (ui_callback_data_t)screenshot_ted_menu;
            break;
        default:
            /* VSID */
            break;
    }
    return 0;
}


/** \brief  Shutdown media menu
 *
 */
void uimedia_menu_shutdown(void)
{
    if (sound_record_dyn_menu_init) {
        sdl_menu_sound_record_free();
    }
}
