/**
 * $Id$
 * 
 * This file defines the public functions and definitions exported by the SH4
 * modules.
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

#ifndef lxdream_sh4_H
#define lxdream_sh4_H 1

#include "lxdream.h"
#include "mem.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * SH4 is running normally 
 */
#define SH4_STATE_RUNNING 1
/**
 * SH4 is not executing instructions but all peripheral modules are still
 * running
 */
#define SH4_STATE_SLEEP 2
/**
 * SH4 is not executing instructions, DMAC is halted, but all other peripheral
 * modules are still running
 */
#define SH4_STATE_DEEP_SLEEP 3
/**
 * SH4 is not executing instructions and all peripheral modules are also
 * stopped. As close as you can get to powered-off without actually being
 * off.
 */
#define SH4_STATE_STANDBY 4

/**
 * sh4r.event_types flag indicating a pending IRQ
 */
#define PENDING_IRQ 1

/**
 * sh4r.event_types flag indicating a pending event (from the event queue)
 */
#define PENDING_EVENT 2

/**
 * SH4 register structure
 */
struct sh4_registers {
    uint32_t r[16];
    uint32_t sr, pr, pc;
    union {
        int32_t i;
        float f;
    } fpul;
    uint32_t t, m, q, s; /* really boolean - 0 or 1 */
    float fr[2][16]; /* Must be aligned on 16-byte boundary */
    uint32_t fpscr;
    uint32_t pad; /* Pad up to 64-bit boundaries */
    uint64_t mac;
    uint32_t gbr, ssr, spc, sgr, dbr, vbr;

    uint32_t r_bank[8]; /* hidden banked registers */
    int32_t store_queue[16]; /* technically 2 banks of 32 bytes */

    uint32_t new_pc; /* Not a real register, but used to handle delay slots */
    uint32_t event_pending; /* slice cycle time of the next pending event, or FFFFFFFF
                             when no events are pending */
    uint32_t event_types; /* bit 0 = IRQ pending, bit 1 = general event pending */
    int in_delay_slot; /* flag to indicate the current instruction is in
     * a delay slot (certain rules apply) */
    uint32_t slice_cycle; /* Current nanosecond within the timeslice */
    uint32_t bus_cycle; /* Nanosecond within the timeslice that the bus will be free */
    int sh4_state; /* Current power-on state (one of the SH4_STATE_* values ) */
    
    /* Not saved */
    int xlat_sh4_mode; /* Collection of execution mode flags (derived) from fpscr, sr, etc */
};

extern struct sh4_registers sh4r;

extern const struct cpu_desc_struct sh4_cpu_desc;

typedef enum {
    SH4_INTERPRET,
    SH4_TRANSLATE,
    SH4_SHADOW
} sh4core_t;

/**
 * Switch between translation and emulation execution modes. Note that this
 * should only be used while the system is stopped. If the system was built
 * without translation support, this method has no effect.
 *
 * @param use TRUE for translation mode, FALSE for emulation mode.
 */
void sh4_set_core( sh4core_t core );

/**
 * Test if system is currently using the translation engine.
 */
gboolean sh4_translate_is_enabled();

/**
 * Explicitly set the SH4 PC to the supplied value - this will be the next
 * instruction executed. This should only be called while the system is stopped.
 */
void sh4_set_pc( int pc );

/**
 * Set the time of the next pending event within the current timeslice.
 */
void sh4_set_event_pending( uint32_t cycles );

/**
 * Handle an event that's due (note caller is responsible for ensuring that the
 * event is in fact due).
 */
void sh4_handle_pending_events();

/**
 * Execute (using the emulator) a single instruction (in other words, perform a
 * single-step operation). 
 */
gboolean sh4_execute_instruction( void );

/* SH4 breakpoints */
void sh4_set_breakpoint( uint32_t pc, breakpoint_type_t type );
gboolean sh4_clear_breakpoint( uint32_t pc, breakpoint_type_t type );
int sh4_get_breakpoint( uint32_t pc );

/** Dump current SH4 core state (for crashdump purposes) */
void sh4_crashdump();

/** Dump a translated block with SH4 and target assembly side by side. */
void sh4_translate_dump_block( uint32_t pc );

/**
 * Enable/disable basic block profiling (Note only supported by translation cores)
 */
void sh4_set_profile_blocks( gboolean flag );

/**
 * Get the boolean flag indicating whether block profiling is on.
 */
gboolean sh4_get_profile_blocks();



#ifdef __cplusplus
}
#endif
#endif /* !lxdream_sh4_H */
