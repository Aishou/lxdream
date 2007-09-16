/**
 * $Id: sh4trans.c,v 1.2 2007-09-04 08:40:23 nkeynes Exp $
 * 
 * SH4 translation core module. This part handles the non-target-specific
 * section of the translation.
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

#include "sh4core.h"
#include "sh4trans.h"
#include "xltcache.h"

/**
 * Execute a timeslice using translated code only (ie translate/execute loop)
 * Note this version does not support breakpoints
 */
uint32_t sh4_xlat_run_slice( uint32_t nanosecs ) 
{
    int i;
    sh4r.slice_cycle = 0;

    if( sh4r.sh4_state != SH4_STATE_RUNNING ) {
	if( sh4r.event_pending < nanosecs ) {
	    sh4r.sh4_state = SH4_STATE_RUNNING;
	    sh4r.slice_cycle = sh4r.event_pending;
	}
    }

    while( sh4r.slice_cycle < nanosecs ) {
	if( SH4_EVENT_PENDING() ) {
	    if( sh4r.event_types & PENDING_EVENT ) {
		event_execute();
	    }
	    /* Eventq execute may (quite likely) deliver an immediate IRQ */
	    if( sh4r.event_types & PENDING_IRQ ) {
		sh4_accept_interrupt();
	    }
	}

	gboolean (*code)() = xlat_get_code(sh4r.pc);
	if( code == NULL ) {
	    code = sh4_translate_basic_block( sh4r.pc );
	}
	if( !code() )
	    break;
    }

    /* If we aborted early, but the cpu is still technically running,
     * we're doing a hard abort - cut the timeslice back to what we
     * actually executed
     */
    if( sh4r.slice_cycle < nanosecs && sh4r.sh4_state == SH4_STATE_RUNNING ) {
	nanosecs = sh4r.slice_cycle;
    }
    if( sh4r.sh4_state != SH4_STATE_STANDBY ) {
	TMU_run_slice( nanosecs );
	SCIF_run_slice( nanosecs );
    }
    return nanosecs;
}

uint8_t *xlat_output;

/**
 * Translate a linear basic block, ie all instructions from the start address
 * (inclusive) until the next branch/jump instruction or the end of the page
 * is reached.
 * @return the address of the translated block
 * eg due to lack of buffer space.
 */
void * sh4_translate_basic_block( sh4addr_t start )
{
    uint32_t pc = start;
    int done;
    xlat_cache_block_t block = xlat_start_block( start );
    xlat_output = (uint8_t *)block->code;
    uint8_t *eob = xlat_output + block->size;
    sh4_translate_begin_block();

    while( (done = sh4_x86_translate_instruction( pc )) == 0 ) {
	if( eob - xlat_output < MAX_INSTRUCTION_SIZE ) {
	    uint8_t *oldstart = block->code;
	    block = xlat_extend_block();
	    xlat_output = block->code + (xlat_output - oldstart);
	    eob = block->code + block->size;
	}
	pc += 2;
    }
    sh4_translate_end_block(pc);
    xlat_commit_block( xlat_output - block->code, pc-start );
    return block->code;
}

