/**
 * $Id: main.c,v 1.35 2007-11-07 11:45:53 nkeynes Exp $
 *
 * Main program, initializes dreamcast and gui, then passes control off to
 * the main loop. 
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

#include <unistd.h>
#include <getopt.h>
#include "dream.h"
#include "config.h"
#include "syscall.h"
#include "mem.h"
#include "dreamcast.h"
#include "display.h"
#include "loader.h"
#include "gui.h"
#include "aica/audio.h"
#include "gdrom/gdrom.h"
#include "maple/maple.h"
#include "sh4/sh4core.h"

#define S3M_PLAYER "s3mplay.bin"

char *option_list = "a:m:s:A:V:puhbd:c:t:xD";
struct option longopts[1] = { { NULL, 0, 0, 0 } };
char *aica_program = NULL;
char *s3m_file = NULL;
const char *disc_file = NULL;
char *display_driver_name = NULL;
char *audio_driver_name = NULL;
gboolean start_immediately = FALSE;
gboolean headless = FALSE;
gboolean without_bios = FALSE;
gboolean use_xlat = TRUE;
gboolean show_debugger = FALSE;
uint32_t time_secs = 0;
uint32_t time_nanos = 0;
extern uint32_t sh4_cpu_multiplier;

int main (int argc, char *argv[])
{
    int opt, i;
    double t;
    gboolean display_ok;

    install_crash_handler();
    gdrom_get_native_devices();
#ifdef ENABLE_NLS
    bindtextdomain (PACKAGE, PACKAGE_LOCALE_DIR);
    textdomain (PACKAGE);
#endif
    display_ok = gui_parse_cmdline(&argc, &argv);

    while( (opt = getopt_long( argc, argv, option_list, longopts, NULL )) != -1 ) {
	switch( opt ) {
	case 'a': /* AICA only mode - argument is an AICA program */
	    aica_program = optarg;
	    break;
	case 'c': /* Config file */
	    lxdream_set_config_filename(optarg);
	    break;
	case 'd': /* Mount disc */
	    disc_file = optarg;
	    break;
	case 'D': /* Launch w/ debugger */
	    show_debugger = TRUE;
	    break;
	case 'm': /* Set SH4 CPU clock multiplier (default 0.5) */
	    t = strtod(optarg, NULL);
	    sh4_cpu_multiplier = (int)(1000.0/t);
	    break;
	case 's': /* AICA-only w/ S3M player */
	    aica_program = S3M_PLAYER;
	    s3m_file = optarg;
	    break;
	case 'A': /* Audio driver */
	    audio_driver_name = optarg;
	    break;
	case 'V': /* Video driver */
	    display_driver_name = optarg;
	    break;
	case 'p': /* Start immediately */
	    start_immediately = TRUE;
    	    break;
	case 'u': /* Allow unsafe dcload syscalls */
	    dcload_set_allow_unsafe(TRUE);
	    break;
    	case 'b': /* No BIOS */
    	    without_bios = TRUE;
    	    break;
        case 'h': /* Headless */
            headless = TRUE;
            break;
	case 't': /* Time limit */
	    t = strtod(optarg, NULL);
	    time_secs = (uint32_t)t;
	    time_nanos = (int)((t - time_secs) * 1000000000);
	    break;
	case 'x': /* Disable translator */
	    use_xlat = FALSE;
	    break;
	}
    }

    lxdream_load_config( );

    if( aica_program == NULL ) {
	dreamcast_init();
    } else {
	dreamcast_configure_aica_only();
	mem_load_block( aica_program, 0x00800000, 2048*1024 );
	if( s3m_file != NULL ) {
	    mem_load_block( s3m_file, 0x00810000, 2048*1024 - 0x10000 );
	}
    }

    if( without_bios ) {
    	bios_install();
	dcload_install();
    }

    audio_driver_t audio_driver = get_audio_driver_by_name(audio_driver_name);
    if( audio_driver == NULL ) {
	ERROR( "Audio driver '%s' not found, aborting.", audio_driver_name );
	exit(2);
    } else if( audio_set_driver( audio_driver, 44100, AUDIO_FMT_16ST ) == FALSE ) {
	ERROR( "Failed to initialize audio driver '%s', using null driver", 
	       audio_driver->name );
	audio_set_driver( &audio_null_driver, 44100, AUDIO_FMT_16ST );
    }

    if( headless ) {
	display_set_driver( &display_null_driver );
    } else {
	gui_init(show_debugger);

	display_driver_t display_driver = get_display_driver_by_name(display_driver_name);
	if( display_driver == NULL ) {
	    ERROR( "Video driver '%s' not found, aborting.", display_driver_name );
	    exit(2);
	} else if( display_set_driver( display_driver ) == FALSE ) {
	    ERROR( "Video driver '%s' failed to initialize (could not connect to display?)", 
		   display_driver->name );
	    exit(2);
	}
    }

    maple_reattach_all();
    INFO( "%s! ready...", APP_NAME );

    for( ; optind < argc; optind++ ) {
	gboolean ok = gdrom_menu_open_file(argv[optind]);
	if( !ok ) {
	    ok = file_load_magic( argv[optind] );
	}
	if( !ok ) {
	    ERROR( "Unrecognized file '%s'", argv[optind] );
	}
	start_immediately = ok;
    }

    if( disc_file != NULL ) {
	gdrom_menu_open_file( disc_file );
    }

    if( gdrom_get_current_disc() == NULL ) {
	disc_file = lxdream_get_config_value( CONFIG_GDROM );
	if( disc_file != NULL ) {
	    gdrom_menu_open_file( disc_file );
	}
    }

    sh4_set_use_xlat( use_xlat );

    if( start_immediately ) {
        if( dreamcast_can_run() ) {
	    if( time_nanos != 0 || time_secs != 0 ) {
	        dreamcast_run_for(time_secs, time_nanos);
		return 0;
	    } else {
	        dreamcast_run();
	    }
	} else {
	    ERROR( "Unable to start dreamcast: no program/bios loaded" );
	}
    }
    if( !headless ) {
	gui_main_loop();
    }
    return 0;
}

