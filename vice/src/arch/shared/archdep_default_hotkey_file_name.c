/** \file   archdep_default_hotkey_file_name.c
 * \brief   Get the path to the default hotkeys file
 *
 * \author  Bas Wassink <b.wassink@ziggo.nl>
 *
 * XXX:     No longer used by the Gtk3 port, superceeded by
 *          `ui_hotkeys_vhk_filename_vice()` in arch/shared/hotkeys/uihotkeys.c.
 *          Once SDL hotkeys are also using the shared hotkeys API this file
 *          can be removed.
 */

/*
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
 */

#include "vice.h"
#include "archdep_defs.h"

#include "archdep.h"
#include "archdep_boot_path.h"
#include "archdep_home_path.h"
#include "archdep_user_config_path.h"
#include "kbd.h"
#include "machine.h"
#include "util.h"

#include "archdep_default_hotkey_file_name.h"


/** \brief  Get path to default hotkeys file
 *
 * \return  Full path to hotkeys file
 *
 * \note    Free after use with lib_free()
 */
char *archdep_default_hotkey_file_name(void)
{
    char filename[256];

    snprintf(filename,
             sizeof(filename),
             KBD_PORT_PREFIX "-hotkeys-%s.vhk",
             machine_get_name());

    return util_join_paths(archdep_user_config_path(), filename, NULL);
}
