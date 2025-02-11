
/*
 * attach.h - File system attach management.
 *
 * Written by
 *  Andreas Boose <viceteam@t-online.de>
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

#ifndef VICE_ATTACH_H
#define VICE_ATTACH_H

#include "types.h"

/*
 * FIXME: Note about ATTACH_DEVICE_FS and ATTACH_DEVICE_VIRT:
 * Attaching a disk image also uses _FS even though you would expect _VIRT.
 * The value _VIRT seems to be unused in practice.
 * One would expect _FS for the fsdevice, and _VIRT for vdrive images.
 */
#define ATTACH_DEVICE_NONE 0
#define ATTACH_DEVICE_FS   1 /* filesystem */
#define ATTACH_DEVICE_REAL 2 /* real IEC device (opencbm) */
/* 3 was raw device */
#define ATTACH_DEVICE_VIRT 4 /* non-tde drive/image */

struct vdrive_s;

void file_system_init(void);
void file_system_shutdown(void);
int file_system_resources_init(void);
int file_system_cmdline_options_init(void);

const char *file_system_get_disk_name(unsigned int unit, unsigned int drive);
int file_system_attach_disk(unsigned int unit, unsigned int drive, const char *filename);
void file_system_detach_disk(unsigned int unit, unsigned int drive);
void file_system_detach_disk_shutdown(void);
struct vdrive_s *file_system_get_vdrive(unsigned int unit);
struct disk_image_s *file_system_get_image(unsigned int unit, unsigned int drive);
int file_system_bam_get_disk_id(unsigned int unit, unsigned int drive, uint8_t *id);
int file_system_bam_set_disk_id(unsigned int unit, unsigned int drive, uint8_t *id);
void file_system_event_playback(unsigned int unit, unsigned int drive, const char *filename);

#endif
