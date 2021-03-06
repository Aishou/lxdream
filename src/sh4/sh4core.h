/**
 * $Id$
 * 
 * This file defines the internal functions used by the SH4 core, 
 *
 * Copyright (c) 2005-2008 Nathan Keynes.
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

#ifndef lxdream_sh4core_H
#define lxdream_sh4core_H 1

#include <glib/gtypes.h>
#include <stdint.h>
#include <stdio.h>
#include "mem.h"
#include "sh4/sh4.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Breakpoint data structure */
extern struct breakpoint_struct sh4_breakpoints[MAX_BREAKPOINTS];
extern int sh4_breakpoint_count;
extern gboolean sh4_starting;
extern gboolean sh4_profile_blocks;

/**
 * Cached direct pointer to the current instruction page. If AT is on, this
 * is derived from the ITLB, otherwise this will be the entire memory region.
 * This is actually a fairly useful optimization, as we can make a lot of
 * assumptions about the "current page" that we can't make in general for
 * arbitrary virtual addresses.
 */
struct sh4_icache_struct {
    sh4ptr_t page; // Page pointer (NULL if no page)
    sh4vma_t page_vma; // virtual address of the page.
    sh4addr_t page_ppa; // physical address of the page
    uint32_t mask;  // page mask 
};
extern struct sh4_icache_struct sh4_icache;

/**
 * Test if a given address is contained in the current icache entry
 */
#define IS_IN_ICACHE(addr) (sh4_icache.page_vma == ((addr) & sh4_icache.mask))
/**
 * Return a pointer for the given vma, under the assumption that it is
 * actually contained in the current icache entry.
 */
#define GET_ICACHE_PTR(addr) (sh4_icache.page + ((addr)-sh4_icache.page_vma))
/**
 * Return the physical (external) address for the given vma, assuming that it is
 * actually contained in the current icache entry.
 */
#define GET_ICACHE_PHYS(addr) (sh4_icache.page_ppa + ((addr)-sh4_icache.page_vma))

/**
 * Return the virtual (vma) address for the first address past the end of the 
 * cache entry. Assumes that there is in fact a current icache entry.
 */
#define GET_ICACHE_END() (sh4_icache.page_vma + (~sh4_icache.mask) + 1)


/**
 * SH4 vm-exit flag - exit the current block but continue normally
 */
#define CORE_EXIT_CONTINUE 1

/**
 * SH4 vm-exit flag - exit the current block and halt immediately (eg fatal error)
 */
#define CORE_EXIT_HALT 2

/**
 * SH4 vm-exit flag - exit the current block and halt immediately for a system
 * breakpoint.
 */
#define CORE_EXIT_BREAKPOINT 3

/**
 * SH4 vm-exit flag - exit the current block and continue after performing a full
 * system reset (dreamcast_reset())
 */
#define CORE_EXIT_SYSRESET 4

/**
 * SH4 vm-exit flag - exit the current block and continue after the next IRQ.
 */
#define CORE_EXIT_SLEEP 5

/**
 * SH4 vm-exit flag - exit the current block and flush all instruction caches (ie
 * if address translation has changed)
 */
#define CORE_EXIT_FLUSH_ICACHE 6

/**
 * SH4 vm-exit flag - exit the current block following a taken exception. sh4r.spc
 * is fixed up by recovery rather than sh4r.pc.
 */
#define CORE_EXIT_EXCEPTION 7

typedef uint32_t (*sh4_run_slice_fn)(uint32_t);

/* SH4 module functions */
void sh4_init( void );
void sh4_reset( void );
void sh4_run( void );
void sh4_stop( void );
uint32_t sh4_run_slice( uint32_t nanos ); // Run single timeslice using emulator
uint32_t sh4_xlat_run_slice( uint32_t nanos ); // Run single timeslice using translator
uint32_t sh4_sleep_run_slice( uint32_t nanos ); // Run single timeslice while the CPU is asleep

/**
 * Immediately exit from the currently executing instruction with the given
 * exit code. This method does not return.
 */
void sh4_core_exit( int exit_code );

/**
 * Exit the current block at the end of the current instruction, flush the
 * translation cache (completely) and return control to sh4_xlat_run_slice.
 *
 * As a special case, if the current instruction is actually the last 
 * instruction in the block (ie it's in a delay slot), this function 
 * returns to allow normal completion of the translation block. Otherwise
 * this function never returns.
 *
 * Must only be invoked (indirectly) from within translated code.
 */
void sh4_flush_icache();

/* SH4 peripheral module functions */
void CPG_reset( void );
void DMAC_reset( void );
void DMAC_run_slice( uint32_t );
void DMAC_save_state( FILE * );
int DMAC_load_state( FILE * );
void INTC_reset( void );
void INTC_save_state( FILE *f );
int INTC_load_state( FILE *f );
void MMU_init( void );
void MMU_reset( void );
void MMU_save_state( FILE *f );
int MMU_load_state( FILE *f );
void MMU_ldtlb();
void CCN_reset();
void CCN_set_cache_control( int reg );
void CCN_save_state( FILE *f );
int CCN_load_state( FILE *f );
void SCIF_reset( void );
void SCIF_run_slice( uint32_t );
void SCIF_save_state( FILE *f );
int SCIF_load_state( FILE *f );
void SCIF_update_line_speed(void);
void TMU_init( void );
void TMU_reset( void );
void TMU_run_slice( uint32_t );
void TMU_save_state( FILE * );
int TMU_load_state( FILE * );
void TMU_update_clocks( void );
void PMM_reset( void );
void PMM_write_control( int, uint32_t );
void PMM_save_state( FILE * );
int PMM_load_state( FILE * );
uint32_t PMM_run_slice( uint32_t );
uint32_t sh4_translate_run_slice(uint32_t);
uint32_t sh4_emulate_run_slice(uint32_t);

/* SH4 instruction support methods */
mem_region_fn_t FASTCALL sh7750_decode_address( sh4addr_t address );
void FASTCALL sh7750_decode_address_copy( sh4addr_t address, mem_region_fn_t result );
void FASTCALL sh4_sleep( void );
void FASTCALL sh4_fsca( uint32_t angle, float *fr );
void FASTCALL sh4_ftrv( float *fv );
uint32_t FASTCALL sh4_read_sr(void);
void FASTCALL sh4_write_sr(uint32_t val);
void FASTCALL sh4_write_fpscr(uint32_t val);
void FASTCALL sh4_switch_fr_banks(void);
void FASTCALL signsat48(void);
gboolean sh4_has_page( sh4vma_t vma );

/* SH4 Memory */
#define MMU_VMA_ERROR 0x80000000
/**
 * Update the sh4_icache structure to contain the specified vma. If the vma
 * cannot be resolved, an MMU exception is raised and the function returns
 * FALSE. Otherwise, returns TRUE and updates sh4_icache accordingly.
 * Note: If the vma resolves to a non-memory area, sh4_icache will be 
 * invalidated, but the function will still return TRUE.
 * @return FALSE if an MMU exception was raised, otherwise TRUE.
 */
gboolean FASTCALL mmu_update_icache( sh4vma_t addr );

int64_t FASTCALL sh4_read_quad( sh4addr_t addr );
int32_t FASTCALL sh4_read_long( sh4addr_t addr );
int32_t FASTCALL sh4_read_word( sh4addr_t addr );
int32_t FASTCALL sh4_read_byte( sh4addr_t addr );
void FASTCALL sh4_write_quad( sh4addr_t addr, uint64_t val );
void FASTCALL sh4_write_long( sh4addr_t addr, uint32_t val );
void FASTCALL sh4_write_word( sh4addr_t addr, uint32_t val );
void FASTCALL sh4_write_byte( sh4addr_t addr, uint32_t val );
int32_t sh4_read_phys_word( sh4addr_t addr );
void FASTCALL sh4_flush_store_queue( sh4addr_t addr );
void FASTCALL sh4_flush_store_queue_mmu( sh4addr_t addr, void *exc );

/* SH4 Exceptions */
#define EXC_POWER_RESET     0x000 /* reset vector */
#define EXC_MANUAL_RESET    0x020 /* reset vector */
#define EXC_TLB_MISS_READ   0x040 /* TLB vector */
#define EXC_TLB_MISS_WRITE  0x060 /* TLB vector */
#define EXC_INIT_PAGE_WRITE 0x080
#define EXC_TLB_PROT_READ   0x0A0
#define EXC_TLB_PROT_WRITE  0x0C0
#define EXC_DATA_ADDR_READ  0x0E0
#define EXC_DATA_ADDR_WRITE 0x100
#define EXC_TLB_MULTI_HIT   0x140
#define EXC_SLOT_ILLEGAL    0x1A0
#define EXC_ILLEGAL         0x180
#define EXC_TRAP            0x160
#define EXC_FPU_DISABLED    0x800
#define EXC_SLOT_FPU_DISABLED 0x820

#define EXV_EXCEPTION    0x100  /* General exception vector */
#define EXV_TLBMISS      0x400  /* TLB-miss exception vector */
#define EXV_INTERRUPT    0x600  /* External interrupt vector */

void FASTCALL sh4_raise_exception( int );
void FASTCALL sh4_raise_reset( int );
void FASTCALL sh4_raise_trap( int );
void FASTCALL sh4_raise_tlb_exception( int, sh4vma_t );
void FASTCALL sh4_raise_tlb_multihit( sh4vma_t );
void FASTCALL sh4_accept_interrupt( void );

#define RAISE_TLB_ERROR(code, vpn) sh4_raise_tlb_exception(code, vpn)
#define RAISE_MEM_ERROR(code, vpn) \
    MMIO_WRITE(MMU, TEA, vpn); \
    MMIO_WRITE(MMU, PTEH, ((MMIO_READ(MMU, PTEH) & 0x000003FF) | (vpn&0xFFFFFC00))); \
    sh4_raise_exception(code);
#define RAISE_TLB_MULTIHIT_ERROR(vpn) sh4_raise_tlb_multihit(vpn)

#ifdef HAVE_FRAME_ADDRESS
#define SH4_EXCEPTION_EXIT() do{ *(((void * volatile *)__builtin_frame_address(0))+1) = exc; } while(0)
#else
#define SH4_EXCEPTION_EXIT() sh4_core_exit(CORE_EXIT_EXCEPTION)
#endif

/**
 * Helper method to update the SH4 registers for an exception, without
 * touching the MMU registers. Mainly for use in shadow mode.
 */
void FASTCALL sh4_reraise_exception( sh4addr_t exception_pc );
/**
 * Complete the current instruction as part of a core exit. Prevents the 
 * system from being left in an inconsistent state when an exit is 
 * triggered during a memory write. 
 */  
void sh4_finalize_instruction( void );

/* Status Register (SR) bits */
#define SR_MD    0x40000000 /* Processor mode ( User=0, Privileged=1 ) */ 
#define SR_RB    0x20000000 /* Register bank (priviledged mode only) */
#define SR_BL    0x10000000 /* Exception/interupt block (1 = masked) */
#define SR_FD    0x00008000 /* FPU disable */
#define SR_M     0x00000200
#define SR_Q     0x00000100
#define SR_IMASK 0x000000F0 /* Interrupt mask level */
#define SR_S     0x00000002 /* Saturation operation for MAC instructions */
#define SR_T     0x00000001 /* True/false or carry/borrow */
#define SR_MASK  0x700083F3
#define SR_MQSTMASK 0xFFFFFCFC /* Mask to clear the flags we're keeping separately */
#define SR_MDRB  0x60000000 /* MD+RB mask for convenience */

#define IS_SH4_PRIVMODE() (sh4r.sr&SR_MD)
#define SH4_INTMASK() ((sh4r.sr&SR_IMASK)>>4)
#define SH4_EVENT_PENDING() (sh4r.event_pending <= sh4r.slice_cycle && !sh4r.in_delay_slot)

#define FPSCR_FR     0x00200000 /* FPU register bank */
#define FPSCR_SZ     0x00100000 /* FPU transfer size (0=32 bits, 1=64 bits) */
#define FPSCR_PR     0x00080000 /* Precision (0=32 bites, 1=64 bits) */
#define FPSCR_DN     0x00040000 /* Denormalization mode (1 = treat as 0) */
#define FPSCR_CAUSE  0x0003F000
#define FPSCR_ENABLE 0x00000F80
#define FPSCR_FLAG   0x0000007C
#define FPSCR_RM     0x00000003 /* Rounding mode (0=nearest, 1=to zero) */
#define FPSCR_MASK   0x003FFFFF

#define IS_FPU_DOUBLEPREC() (sh4r.fpscr&FPSCR_PR)
#define IS_FPU_DOUBLESIZE() (sh4r.fpscr&FPSCR_SZ)
#define IS_FPU_ENABLED() ((sh4r.sr&SR_FD)==0)

#define FR(x) sh4r.fr[0][(x)^1]
#define DRF(x) *((double *)&sh4r.fr[0][(x)<<1])
#define XF(x) sh4r.fr[1][(x)^1]
#define XDR(x) *((double *)&sh4r.fr[1][(x)<<1])
#define DRb(x,b) *((double *)&sh4r.fr[b][(x)<<1])
#define DR(x) *((double *)&sh4r.fr[x&1][x&0x0E])
#define FPULf    (sh4r.fpul.f)
#define FPULi    (sh4r.fpul.i)

/**************** SH4 internal memory regions *****************/
extern struct mem_region_fn p4_region_itlb_addr;
extern struct mem_region_fn p4_region_itlb_data;
extern struct mem_region_fn p4_region_utlb_addr;
extern struct mem_region_fn p4_region_utlb_data;
extern struct mem_region_fn p4_region_icache_addr;
extern struct mem_region_fn p4_region_icache_data;
extern struct mem_region_fn p4_region_ocache_addr;
extern struct mem_region_fn p4_region_ocache_data;

#define OC_ENABLED 1

#ifdef __cplusplus
}
#endif

#endif /* !lxdream_sh4core_H */

