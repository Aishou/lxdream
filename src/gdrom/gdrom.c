/**
 * $Id: gdrom.c,v 1.1 2006-04-30 01:51:08 nkeynes Exp $
 *
 * GD-Rom  access functions.
 *
 * Copyright (c) 2005 Nathan Keynes.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <stdio.h>

#include "gdrom/ide.h"
#include "gdrom/gdrom.h"
#include "dream.h"

static void gdrom_image_destroy( gdrom_disc_t );
static uint32_t gdrom_image_read_sectors( gdrom_disc_t, uint32_t, uint32_t, char * );


gdrom_disc_t gdrom_disc = NULL;

char *gdrom_mode_names[] = { "Mode1", "Mode2", "XA 1", "XA2", "Audio", "GD-Rom" };
uint32_t gdrom_sector_size[] = { 2048, 2336, 2048, 2324, 2352, 2336 };

gdrom_disc_t gdrom_image_open( const gchar *filename )
{
    return nrg_image_open( filename );
}


gdrom_disc_t gdrom_image_new( FILE *file )
{
    struct gdrom_disc *disc = (struct gdrom_disc *)calloc(1, sizeof(struct gdrom_disc));
    if( disc == NULL )
	return NULL;
    disc->read_sectors = gdrom_image_read_sectors;
    disc->close = gdrom_image_destroy;
    disc->disc_type = IDE_DISC_CDROM;
    disc->file = file;
    return disc;
}

static void gdrom_image_destroy( gdrom_disc_t disc )
{
    if( disc->file != NULL ) {
	fclose(disc->file);
	disc->file = NULL;
    }
    free( disc );
}

static uint32_t gdrom_image_read_sectors( gdrom_disc_t disc, uint32_t sector,
				   uint32_t sector_count, char *buf )
{
    int i, track = -1, track_offset, read_len;

    for( i=0; i<disc->track_count; i++ ) {
	if( disc->track[i].lba <= sector && 
	    disc->track[i].lba + disc->track[i].sector_count <= sector + sector_count ) {
	    track = i;
	    break;
	}
    }
    if( track == -1 )
	return 0;
    
    track_offset = disc->track[track].sector_size * (sector - disc->track[track].lba);
    read_len = disc->track[track].sector_size * sector_count;
    fseek( disc->file, disc->track[track].offset + track_offset, SEEK_SET );
    fread( buf, disc->track[track].sector_size, sector_count, disc->file );
    return read_len;
}

uint32_t gdrom_read_sectors( uint32_t sector, uint32_t sector_count,
			     char *buf )
{
    if( gdrom_disc == NULL )
	return 0; /* No media */
    return gdrom_disc->read_sectors( gdrom_disc, sector, sector_count, buf );
}


void gdrom_dump_disc( gdrom_disc_t disc ) {
    int i;
    INFO( "Disc ID: %s, %d tracks in %d sessions", disc->mcn, disc->track_count, 
	  disc->track[disc->track_count-1].session + 1 );
    for( i=0; i<disc->track_count; i++ ) {
	INFO( "Sess %d Trk %d: Start %06X Length %06X, %s",
	      disc->track[i].session+1, i+1, disc->track[i].lba,
	      disc->track[i].sector_count, gdrom_mode_names[disc->track[i].mode] );
    }
}

gboolean gdrom_get_toc( char *buf ) 
{
    if( gdrom_disc == NULL )
	return FALSE;
    return TRUE;
}

void gdrom_mount_disc( gdrom_disc_t disc ) 
{
    gdrom_unmount_disc();
    gdrom_disc = disc;
    idereg.disc = disc->disc_type | IDE_DISC_READY;
    gdrom_dump_disc( disc );
}

gdrom_disc_t gdrom_mount_image( const gchar *filename )
{
    gdrom_disc_t disc = gdrom_image_open(filename);
    if( disc != NULL )
	gdrom_mount_disc( disc );
    return disc;
}

void gdrom_unmount_disc( ) 
{
    if( gdrom_disc != NULL ) {
	gdrom_disc->close(gdrom_disc);
    }
    gdrom_disc = NULL;
    idereg.disc = IDE_DISC_NONE;
}

gboolean gdrom_is_mounted( void ) 
{
    return gdrom_disc != NULL;
}