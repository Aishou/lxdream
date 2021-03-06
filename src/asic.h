/**
 * $Id$
 *
 * Support for the miscellaneous ASIC functions (Primarily event multiplexing,
 * and DMA). Includes MMIO definitions for the 5f6000 and 5f7000 regions, 
 * although some functions (maple, ide) are implemented elsewhere.
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

#include "mmio.h"

/**
 * ASIC interrupts are mappable to any (or all of) 3 actual CPU IRQ lines.
 * events selected for IRQA trigger IRQ 13, IRQB => 11 and IRQC => 9.
 */

MMIO_REGION_BEGIN( 0x005F6000, ASIC, "System ASIC" )
    LONG_PORT( 0x800, PVRDMADEST, PORT_MRW, 0, "PVR DMA Dest Address" )
    LONG_PORT( 0x804, PVRDMACNT, PORT_MRW, 0, "PVR DMA Byte Count" )
    LONG_PORT( 0x808, PVRDMACTL, PORT_MRW, 0, "PVR DMA Control" )
    LONG_PORT( 0x810, SORTDMATBL, PORT_MRW, 0, "Sort DMA Table address" )
    LONG_PORT( 0x814, SORTDMADATA, PORT_MRW, 0, "Sort DMA Data base address" )
    LONG_PORT( 0x818, SORTDMATSIZ, PORT_MRW, 0, "Sort DMA Table entry size" )
    LONG_PORT( 0x81C, SORTDMAASIZ, PORT_MRW, 0, "Sort DMA Table address size" )
    LONG_PORT( 0x820, SORTDMACTL, PORT_MRW, 0, "Sort DMA Control" )
    LONG_PORT( 0x840, ASICUNK5, PORT_MRW, 0, "ASIC <unknown5>" )
    LONG_PORT( 0x844, ASICUNK6, PORT_MRW, 0, "ASIC <unknown6>" )
    LONG_PORT( 0x848, ASICUNK7, PORT_MRW, 0, "ASIC <unknown7>" )
    LONG_PORT( 0x84C, ASICUNK8, PORT_MRW, 0, "ASIC <unknown8>" )
    LONG_PORT( 0x860, SORTDMACNT, PORT_MR, 0, "Sort DMA Transfer count" )
    LONG_PORT( 0x884, PVRDMARGN1, PORT_MRW, 0, "PVR DMA Dest region 1" )
    LONG_PORT( 0x888, PVRDMARGN2, PORT_MRW, 0, "PVR DMA Dest region 2" )
    LONG_PORT( 0x88C, G2STATUS, PORT_MR|PORT_NOTRACE, 0x0E, "G2 Fifo status" )
    LONG_PORT( 0x890, SYSRESET, PORT_W, 0, "System reset port" )
    LONG_PORT( 0x89C, ASICUNKB, PORT_MRW, 0xB, "Unknown, always 0xB?" )
    LONG_PORT( 0x8A0, ASICUNKC, PORT_MRW, 0, "ASIC <unknownC>" )
    LONG_PORT( 0x8A4, ASICUNKD, PORT_MRW, 0, "ASIC <unknownD>" )
    LONG_PORT( 0x8AC, ASICUNKE, PORT_MRW, 0, "ASIC <unknownE>" )
    LONG_PORT( 0x900, PIRQ0, PORT_MRW|PORT_NOTRACE, 0, "Pending interrupts 0" )
    LONG_PORT( 0x904, PIRQ1, PORT_MRW, 0, "Pending interrupts 1" )
    LONG_PORT( 0x908, PIRQ2, PORT_MRW, 0, "Pending interrupts 2" )
    LONG_PORT( 0x910, IRQA0, PORT_MRW, 0, "IRQ A event map 0" )
    LONG_PORT( 0x914, IRQA1, PORT_MRW, 0, "IRQ A event map 1" )
    LONG_PORT( 0x918, IRQA2, PORT_MRW, 0, "IRQ A event map 2" )
    LONG_PORT( 0x920, IRQB0, PORT_MRW, 0, "IRQ B event map 0" )
    LONG_PORT( 0x924, IRQB1, PORT_MRW, 0, "IRQ B event map 1" )
    LONG_PORT( 0x928, IRQB2, PORT_MRW, 0, "IRQ B event map 2" )
    LONG_PORT( 0x930, IRQC0, PORT_MRW, 0, "IRQ C event map 0" )
    LONG_PORT( 0x934, IRQC1, PORT_MRW, 0, "IRQ C event map 1" )
    LONG_PORT( 0x938, IRQC2, PORT_MRW, 0, "IRQ C event map 2" )
    LONG_PORT( 0x940, ASIC9UNK1, PORT_MRW, 0, "Unknown 1" )
    LONG_PORT( 0x944, ASIC9UNK2, PORT_MRW, 0, "Unknown 2" )
    LONG_PORT( 0x950, ASIC9UNK3, PORT_MRW, 0, "Unknown 3" )
    LONG_PORT( 0x954, ASIC9UNK4, PORT_MRW, 0, "Unknown 4" )
/* ASIC events repeats at 0x980..0x9FF, then the whole region 800..9ff
 * repeats at 000..1ff, 200..3ff, 400..5ff, 600..7ff, a00..bff.
 * The whole region 800..8ff is long-readable, but since I so far have no idea
 * what any of it means (nor have I seen any of it accessed), they're not
 * listed above.
 */
  

    LONG_PORT( 0xC04, MAPLE_DMA, PORT_MRW, UNDEFINED, "Maple DMA Address" )
    LONG_PORT( 0xC10, MAPLE_RESET2, PORT_MRW, UNDEFINED, "Maple Reset 2" )
    LONG_PORT( 0xC14, MAPLE_ENABLE, PORT_MRW, UNDEFINED, "Maple Enable" )
    LONG_PORT( 0xC18, MAPLE_STATE, PORT_MRW, 0, "Maple State" )
    LONG_PORT( 0xC70, MAPLE_UNK1, PORT_MRW, 0, "Maple unknown 1" )
    LONG_PORT( 0xC74, MAPLE_UNK2, PORT_MRW, 0, "Maple unknown 2" )
    LONG_PORT( 0xC78, MAPLE_UNK3, PORT_MRW, 0, "Maple unknown 3" )
    LONG_PORT( 0xC7C, MAPLE_UNK4, PORT_MRW, 0, "Maple unknown 4" )
    LONG_PORT( 0xC80, MAPLE_SPEED, PORT_MRW, UNDEFINED, "Maple Speed" )
    LONG_PORT( 0xC84, MAPLE_UNK5, PORT_MRW, 0, "Maple unknown 5" )
    LONG_PORT( 0xC8C, MAPLE_RESET1, PORT_MRW, UNDEFINED, "Maple Reset 1" )
    LONG_PORT( 0xCE8, MAPLE_UNK6, PORT_MRW, 0, "Maple unknown 6" )
    LONG_PORT( 0xCF4, MAPLE_SRC, PORT_MRW, 0, "Maple current source" )
    LONG_PORT( 0xCF8, MAPLE_DEST1, PORT_MRW, 0, "Maple current destination" )
    LONG_PORT( 0xCFC, MAPLE_DEST2, PORT_MRW, 0, "Maple current destination 2?" )
/* Note: Maple registers repeat at 0xD00..0xDFF,
 * 0xE00..0xEFF and 0xF00..0xFFF */
MMIO_REGION_END

MMIO_REGION_BEGIN( 0x005F7000, EXTDMA, "ASIC External DMA" )
    BYTE_PORT( 0x018, IDEALTSTATUS, PORT_RW, 0, "IDE Device Control / Alt-status" ) /* 10110 */
    BYTE_PORT( 0x01C, IDEUNK1, PORT_MRW, 0, "IDE Unknown" )
    WORD_PORT( 0x080, IDEDATA, PORT_RW, 0, "IDE Data" )
    BYTE_PORT( 0x084, IDEFEAT, PORT_RW, 0, "IDE Feature / Error" )
    BYTE_PORT( 0x088, IDECOUNT, PORT_RW, 0, "IDE Sector Count" )
    BYTE_PORT( 0x08C, IDELBA0, PORT_RW, 0, "IDE LBA lo" ) /* AKA sector */
    BYTE_PORT( 0x090, IDELBA1, PORT_RW, 0, "IDE LBA mid" ) /* AKA Cyl lo */
    BYTE_PORT( 0x094, IDELBA2, PORT_RW, 0, "IDE LBA hi" ) /* AKA Cyl hi */
    BYTE_PORT( 0x098, IDEDEV, PORT_RW, 0, "IDE Device" )
    BYTE_PORT( 0x09C, IDECMD, PORT_RW, 0, "IDE Command/Status" )
    LONG_PORT( 0x404, IDEDMASH4, PORT_MRW, 0, "IDE DMA SH4 address" )
    LONG_PORT( 0x408, IDEDMASIZ, PORT_MRW, 0, "IDE DMA Size" )
    LONG_PORT( 0x40C, IDEDMADIR, PORT_MRW, 0, "IDE DMA Direction" )
    LONG_PORT( 0x414, IDEDMACTL1, PORT_MRW, 0, "IDE DMA Control 1" )
    LONG_PORT( 0x418, IDEDMACTL2, PORT_MRW, 0, "IDE DMA Control 2" )
    WORD_PORT( 0x480, EXTDMAUNK0, PORT_MRW, 0, "Ext DMA <unknown0>" )
    LONG_PORT( 0x484, EXTDMAUNK1, PORT_MRW, 0, "Ext DMA <unknown1>" )
    LONG_PORT( 0x488, EXTDMAUNK2, PORT_MRW, 0, "Ext DMA <unknown2>" )
    LONG_PORT( 0x48C, EXTDMAUNK3, PORT_MRW, 0, "Ext DMA <unknown3>" )
    LONG_PORT( 0x490, EXTDMAUNK4, PORT_MRW, 0, "Ext DMA <unknown4>" )
    LONG_PORT( 0x494, EXTDMAUNK5, PORT_MRW, 0, "Ext DMA <unknown5>" )
    LONG_PORT( 0x4A0, EXTDMAUNK6, PORT_MRW, 0, "Ext DMA <unknown6>" )
    LONG_PORT( 0x4A4, EXTDMAUNK7, PORT_MRW, 0, "Ext DMA <unknown7>" )
    LONG_PORT( 0x4B4, EXTDMAUNK8, PORT_MRW, 0, "Ext DMA <unknown8>" )
    LONG_PORT( 0x4B8, IDEDMACFG, PORT_MRW, 0, "IDE DMA Config" ) /* 88437F00 */
    LONG_PORT( 0x4E4, IDEACTIVATE, PORT_MRW, 0, "IDE activate" )
    LONG_PORT( 0x4F8, IDEDMATXSIZ, PORT_MRW, 0, "IDE DMA transfered size" )
    LONG_PORT( 0x800, G2DMA0EXT, PORT_MRW, 0, "G2 DMA0 External address" )
    LONG_PORT( 0x804, G2DMA0SH4, PORT_MRW, 0, "G2 DMA0 SH4-based address" )
    LONG_PORT( 0x808, G2DMA0SIZ, PORT_MRW, 0, "G2 DMA0 Size" )
    LONG_PORT( 0x80C, G2DMA0DIR, PORT_MRW, 0, "G2 DMA0 Direction" )
    LONG_PORT( 0x810, G2DMA0MOD, PORT_MRW, 0, "G2 DMA0 Mode" )
    LONG_PORT( 0x814, G2DMA0CTL1, PORT_MRW, 0, "G2 DMA0 Control 1" )
    LONG_PORT( 0x818, G2DMA0CTL2, PORT_MRW, 0, "G2 DMA0 Control 2" )
    LONG_PORT( 0x81C, G2DMA0STOP, PORT_MRW, 0x20, "G2 DMA0 Stop" )
    LONG_PORT( 0x820, G2DMA1EXT, PORT_MRW, 0, "G2 DMA1 External address" )
    LONG_PORT( 0x824, G2DMA1SH4, PORT_MRW, 0, "G2 DMA1 SH4-based address" )
    LONG_PORT( 0x828, G2DMA1SIZ, PORT_MRW, 0, "G2 DMA1 Size" )
    LONG_PORT( 0x82C, G2DMA1DIR, PORT_MRW, 0, "G2 DMA1 Direction" )
    LONG_PORT( 0x830, G2DMA1MOD, PORT_MRW, 0, "G2 DMA1 Mode" )
    LONG_PORT( 0x834, G2DMA1CTL1, PORT_MRW, 0, "G2 DMA1 Control 1" )
    LONG_PORT( 0x838, G2DMA1CTL2, PORT_MRW, 0, "G2 DMA1 Control 2" )
    LONG_PORT( 0x83C, G2DMA1STOP, PORT_MRW, 0, "G2 DMA1 Stop" )
    LONG_PORT( 0x840, G2DMA2EXT, PORT_MRW, 0, "G2 DMA2 External address" )
    LONG_PORT( 0x844, G2DMA2SH4, PORT_MRW, 0, "G2 DMA2 SH4-based address" )
    LONG_PORT( 0x848, G2DMA2SIZ, PORT_MRW, 0, "G2 DMA2 Size" )
    LONG_PORT( 0x84C, G2DMA2DIR, PORT_MRW, 0, "G2 DMA2 Direction" )
    LONG_PORT( 0x850, G2DMA2MOD, PORT_MRW, 0, "G2 DMA2 Mode" )
    LONG_PORT( 0x854, G2DMA2CTL1, PORT_MRW, 0, "G2 DMA2 Control 1" )
    LONG_PORT( 0x858, G2DMA2CTL2, PORT_MRW, 0, "G2 DMA2 Control 2" )
    LONG_PORT( 0x85C, G2DMA2STOP, PORT_MRW, 0, "G2 DMA2 Stop" )
    LONG_PORT( 0x860, G2DMA3EXT, PORT_MRW, 0, "G2 DMA3 External address" )
    LONG_PORT( 0x864, G2DMA3SH4, PORT_MRW, 0, "G2 DMA3 SH4-based address" )
    LONG_PORT( 0x868, G2DMA3SIZ, PORT_MRW, 0, "G2 DMA3 Size" )
    LONG_PORT( 0x86C, G2DMA3DIR, PORT_MRW, 0, "G2 DMA3 Direction" )
    LONG_PORT( 0x870, G2DMA3MOD, PORT_MRW, 0, "G2 DMA3 Mode" )
    LONG_PORT( 0x874, G2DMA3CTL1, PORT_MRW, 0, "G2 DMA3 Control 1" )
    LONG_PORT( 0x878, G2DMA3CTL2, PORT_MRW, 0, "G2 DMA3 Control 2" )
    LONG_PORT( 0x87C, G2DMA3STOP, PORT_MRW, 0, "G2 DMA3 Stop" )
    LONG_PORT( 0x890, G2DMAWAIT, PORT_MRW, 0, "G2 DMA wait states (?)" )
    LONG_PORT( 0x894, G2DMAUN1, PORT_MRW, 0, "G2 DMA <unknown1>" )
    LONG_PORT( 0x898, G2DMAUN2, PORT_MRW, 0, "G2 DMA <unknown2>" )
    LONG_PORT( 0x89C, G2DMAUN3, PORT_MRW, 0, "G2 DMA <unknown3>" )
    LONG_PORT( 0x8A0, G2DMAUN4, PORT_MRW, 0, "G2 DMA <unknown4>" )
    LONG_PORT( 0x8A4, G2DMAUN5, PORT_MRW, 0, "G2 DMA <unknown5>" )
    LONG_PORT( 0x8A8, G2DMAUN6, PORT_MRW, 0, "G2 DMA <unknown6>" )
    LONG_PORT( 0x8AC, G2DMAUN7, PORT_MRW, 0, "G2 DMA <unknown7>" )
    LONG_PORT( 0x8B0, G2DMAUN8, PORT_MRW, 0, "G2 DMA <unknown8>" )
    LONG_PORT( 0x8B4, G2DMAUN9, PORT_MRW, 0, "G2 DMA <unknown9>" )
    LONG_PORT( 0x8B8, G2DMAUN10, PORT_MRW, 0, "G2 DMA <unknown10>" )
    LONG_PORT( 0x8BC, G2DMACFG, PORT_MRW, 0, "G2 DMA Config" ) /* 46597F00 */
    LONG_PORT( 0xC00, PVRDMA2EXT, PORT_MRW, 0, "PVR DMA External address" )
    LONG_PORT( 0xC04, PVRDMA2SH4, PORT_MRW, 0, "PVR DMA SH4 address" )
    LONG_PORT( 0xC08, PVRDMA2SIZ, PORT_MRW, 0, "PVR DMA Size" )
    LONG_PORT( 0xC0C, PVRDMA2DIR, PORT_MRW, 0, "PVR DMA Direction" )
    LONG_PORT( 0xC10, PVRDMA2MOD, PORT_MRW, 0, "PVR DMA Mode" )
    LONG_PORT( 0xC14, PVRDMA2CTL1, PORT_MRW, 0, "PVR DMA Control 1" )
    LONG_PORT( 0xC18, PVRDMA2CTL2, PORT_MRW, 0, "PVR DMA Control 2" )
    LONG_PORT( 0xC80, PVRDMA2CFG, PORT_MRW, 0, "PVR DMA Config" ) /* 67027F00 */

MMIO_REGION_END

#define EVENT_PVR_RENDER_DONE 2
#define EVENT_SCANLINE2 3
#define EVENT_SCANLINE1 4
#define EVENT_HPOS   5
#define EVENT_PVR_YUV_DONE 6
#define EVENT_PVR_OPAQUE_DONE 7
#define EVENT_PVR_OPAQUEMOD_DONE 8
#define EVENT_PVR_TRANS_DONE 9
#define EVENT_PVR_TRANSMOD_DONE 10
#define EVENT_PVR_DMA2 11
#define EVENT_MAPLE_DMA 12
#define EVENT_MAPLE_ERR 13 /* ??? */
#define EVENT_IDE_DMA 14
#define EVENT_G2_DMA0  15
#define EVENT_G2_DMA1  16
#define EVENT_G2_DMA2  17
#define EVENT_G2_DMA3  18
#define EVENT_PVR_DMA   19
#define EVENT_SORT_DMA  20
#define EVENT_PVR_PUNCHOUT_DONE 21
#define EVENT_CASCADE1  30 /* Set if something in the second word is active */
#define EVENT_CASCADE2  31 /* Set if something in the third word is active */

#define EVENT_IDE       32
#define EVENT_AICA      33

#define EVENT_PVR_PRIM_ALLOC_FAIL 66
#define EVENT_PVR_MATRIX_ALLOC_FAIL 67
#define EVENT_PVR_BAD_INPUT 68
#define EVENT_SORT_DMA_ERR  92

#define IS_IDE_REGISTER(x) ( (x) <= IDEDMACTL2 )

/**
 * Raise an ASIC event 
 */
void asic_event( int event );

/**
 * Clear an ASIC event. Currently only the IDE controller is known to use
 * this functionality.
 */
void asic_clear_event( int event );

void asic_g2_write_word( );

gboolean asic_enable_ide_interface( gboolean enable );
