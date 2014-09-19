/* Default linker script, for normal executables */
OUTPUT_FORMAT("elf32-msp430")
OUTPUT_ARCH("msp430")

MEMORY
{
  sfr              : ORIGIN = 0x0000, LENGTH = 0x0010
  peripheral_8bit  : ORIGIN = 0x0010, LENGTH = 0x00f0
  peripheral_16bit : ORIGIN = 0x0100, LENGTH = 0x0100

  ram (wx)         : ORIGIN = 0x0200, LENGTH = 0x4000
  rom (rx)         : ORIGIN = 0x8000, LENGTH = 0x7fe0

  vectors          : ORIGIN = 0xffe0, LENGTH = 32

  /* Remaining banks are absent */
  bsl              : ORIGIN = 0x0000, LENGTH = 0x0000
  infomem          : ORIGIN = 0x0000, LENGTH = 0x0000
  infob            : ORIGIN = 0x0000, LENGTH = 0x0000
  infoa            : ORIGIN = 0x0000, LENGTH = 0x0000
  infoc            : ORIGIN = 0x0000, LENGTH = 0x0000
  infod            : ORIGIN = 0x0000, LENGTH = 0x0000
  ram2 (wx)        : ORIGIN = 0x0000, LENGTH = 0x0000
  ram_mirror (wx)  : ORIGIN = 0x0000, LENGTH = 0x0000
  usbram (wx)      : ORIGIN = 0x0000, LENGTH = 0x0000
  far_rom          : ORIGIN = 0x00000000, LENGTH = 0x00000000
}

REGION_ALIAS("REGION_TEXT", rom);
REGION_ALIAS("REGION_DATA", ram);
REGION_ALIAS("REGION_FAR_ROM", far_rom);

__IE1 = 0x0000;
__IFG1 = 0x0002;
__ME1 = 0x0004;
__IE2 = 0x0001;
__IFG2 = 0x0003;
__ME2 = 0x0005;
__WDTCTL = 0x0120;
__MPY = 0x0130;
__MPYS = 0x0132;
__MAC = 0x0134;
__MACS = 0x0136;
__OP2 = 0x0138;
__RESLO = 0x013A;
__RESHI = 0x013C;
__SUMEXT = 0x013E;
__P1IN = 0x0020;
__P1OUT = 0x0021;
__P1DIR = 0x0022;
__P1IFG = 0x0023;
__P1IES = 0x0024;
__P1IE = 0x0025;
__P1SEL = 0x0026;
__P2IN = 0x0028;
__P2OUT = 0x0029;
__P2DIR = 0x002A;
__P2IFG = 0x002B;
__P2IES = 0x002C;
__P2IE = 0x002D;
__P2SEL = 0x002E;
__P3IN = 0x0018;
__P3OUT = 0x0019;
__P3DIR = 0x001A;
__P3SEL = 0x001B;
__P4IN = 0x001C;
__P4OUT = 0x001D;
__P4DIR = 0x001E;
__P4SEL = 0x001F;
__P5IN = 0x0030;
__P5OUT = 0x0031;
__P5DIR = 0x0032;
__P5SEL = 0x0033;
__P6IN = 0x0034;
__P6OUT = 0x0035;
__P6DIR = 0x0036;
__P6SEL = 0x0037;
__U0CTL = 0x0070;
__U0TCTL = 0x0071;
__U0RCTL = 0x0072;
__U0MCTL = 0x0073;
__U0BR0 = 0x0074;
__U0BR1 = 0x0075;
__U0RXBUF = 0x0076;
__U0TXBUF = 0x0077;
__U1CTL = 0x0078;
__U1TCTL = 0x0079;
__U1RCTL = 0x007A;
__U1MCTL = 0x007B;
__U1BR0 = 0x007C;
__U1BR1 = 0x007D;
__U1RXBUF = 0x007E;
__U1TXBUF = 0x007F;
__TAIV = 0x012E;
__TACTL = 0x0160;
__TACCTL0 = 0x0162;
__TACCTL1 = 0x0164;
__TACCTL2 = 0x0166;
__TAR = 0x0170;
__TACCR0 = 0x0172;
__TACCR1 = 0x0174;
__TACCR2 = 0x0176;
__TBIV = 0x011E;
__TBCTL = 0x0180;
__TBCCTL0 = 0x0182;
__TBCCTL1 = 0x0184;
__TBCCTL2 = 0x0186;
__TBCCTL3 = 0x0188;
__TBCCTL4 = 0x018A;
__TBCCTL5 = 0x018C;
__TBCCTL6 = 0x018E;
__TBR = 0x0190;
__TBCCR0 = 0x0192;
__TBCCR1 = 0x0194;
__TBCCR2 = 0x0196;
__TBCCR3 = 0x0198;
__TBCCR4 = 0x019A;
__TBCCR5 = 0x019C;
__TBCCR6 = 0x019E;
__DCOCTL = 0x0056;
__BCSCTL1 = 0x0057;
__BCSCTL2 = 0x0058;
__FCTL1 = 0x0128;
__FCTL2 = 0x012A;
__FCTL3 = 0x012C;
__CACTL1 = 0x0059;
__CACTL2 = 0x005A;
__CAPD = 0x005B;
__ADC12CTL0 = 0x01A0;
__ADC12CTL1 = 0x01A2;
__ADC12IFG = 0x01A4;
__ADC12IE = 0x01A6;
__ADC12IV = 0x01A8;
__ADC12MEM0 = 0x0140;
__ADC12MEM1 = 0x0142;
__ADC12MEM2 = 0x0144;
__ADC12MEM3 = 0x0146;
__ADC12MEM4 = 0x0148;
__ADC12MEM5 = 0x014A;
__ADC12MEM6 = 0x014C;
__ADC12MEM7 = 0x014E;
__ADC12MEM8 = 0x0150;
__ADC12MEM9 = 0x0152;
__ADC12MEM10 = 0x0154;
__ADC12MEM11 = 0x0156;
__ADC12MEM12 = 0x0158;
__ADC12MEM13 = 0x015A;
__ADC12MEM14 = 0x015C;
__ADC12MEM15 = 0x015E;
__ADC12MCTL0 = 0x0080;
__ADC12MCTL1 = 0x0081;
__ADC12MCTL2 = 0x0082;
__ADC12MCTL3 = 0x0083;
__ADC12MCTL4 = 0x0084;
__ADC12MCTL5 = 0x0085;
__ADC12MCTL6 = 0x0086;
__ADC12MCTL7 = 0x0087;
__ADC12MCTL8 = 0x0088;
__ADC12MCTL9 = 0x0089;
__ADC12MCTL10 = 0x008A;
__ADC12MCTL11 = 0x008B;
__ADC12MCTL12 = 0x008C;
__ADC12MCTL13 = 0x008D;
__ADC12MCTL14 = 0x008E;
__ADC12MCTL15 = 0x008F;

SECTIONS
{
  /* Read-only sections, merged into text segment.  */
  .hash            : { *(.hash)          }
  .dynsym          : { *(.dynsym)        }
  .dynstr          : { *(.dynstr)        }
  .gnu.version     : { *(.gnu.version)   }
  .gnu.version_d   : { *(.gnu.version_d) }
  .gnu.version_r   : { *(.gnu.version_r) }
  .rel.init      : { *(.rel.init)  }
  .rela.init     : { *(.rela.init) }
  .rel.fini      : { *(.rel.fini)  }
  .rela.fini     : { *(.rela.fini) }
  .rel.text      : { *(.rel.text .rel.text.* .rel.gnu.linkonce.t.*)        }
  .rela.text     : { *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*)     }
  .rel.rodata    : { *(.rel.rodata .rel.rodata.* .rel.gnu.linkonce.r.*)    }
  .rela.rodata   : { *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*) }
  .rel.data      : { *(.rel.data .rel.data.* .rel.gnu.linkonce.d.*)        }
  .rela.data     : { *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*)     }
  .rel.bss       : { *(.rel.bss .rel.bss.* .rel.gnu.linkonce.b.*)          }
  .rela.bss      : { *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*)       }
  .rel.ctors     : { *(.rel.ctors)  }
  .rela.ctors    : { *(.rela.ctors) }
  .rel.dtors     : { *(.rel.dtors)  }
  .rela.dtors    : { *(.rela.dtors) }
  .rel.got       : { *(.rel.got)    }
  .rela.got      : { *(.rela.got)   }
  .rel.plt       : { *(.rel.plt)    }
  .rela.plt      : { *(.rela.plt)   }
  .text :
  {
     . = ALIGN(2);
     KEEP(*(.init .init.*))
     KEEP(*(.init0))  /* Start here after reset.               */
     KEEP(*(.init1))  /* User definable.                       */
     KEEP(*(.init2))  /* Initialize stack.                     */
     KEEP(*(.init3))  /* Initialize hardware, user definable.  */
     KEEP(*(.init4))  /* Copy data to .data, clear bss.        */
     KEEP(*(.init5))  /* User definable.                       */
     KEEP(*(.init6))  /* C++ constructors.                     */
     KEEP(*(.init7))  /* User definable.                       */
     KEEP(*(.init8))  /* User definable.                       */
     KEEP(*(.init9))  /* Call main().                          */
     KEEP(*(.fini9))  /* Falls into here after main(). User definable.  */
     KEEP(*(.fini8))  /* User definable.                           */
     KEEP(*(.fini7))  /* User definable.                           */
     KEEP(*(.fini6))  /* C++ destructors.                          */
     KEEP(*(.fini5))  /* User definable.                           */
     KEEP(*(.fini4))  /* User definable.                           */
     KEEP(*(.fini3))  /* User definable.                           */
     KEEP(*(.fini2))  /* User definable.                           */
     KEEP(*(.fini1))  /* User definable.                           */
     KEEP(*(.fini0))  /* Infinite loop after program termination.  */
     KEEP(*(.fini .fini.*))
     . = ALIGN(2);
     __ctors_start = . ;
     KEEP(*(.ctors))
     __ctors_end = . ;
     __dtors_start = . ;
     KEEP(*(.dtors))
     __dtors_end = . ;
     . = ALIGN(2);
    *(.text .text.* .gnu.linkonce.t.*)
     . = ALIGN(2);
  }  > REGION_TEXT
  .rodata   :
  {
     . = ALIGN(2);
    *(.rodata .rodata.* .gnu.linkonce.r.*)
     . = ALIGN(2);
  }  > REGION_TEXT
   _etext = .; /* Past last read-only (loadable) segment */
  .data   :
  {
     . = ALIGN(2);
     PROVIDE (__data_start = .) ;
    *(.data .data.* .gnu.linkonce.d.*)
     . = ALIGN(2);
  }  > REGION_DATA AT > REGION_TEXT
   _edata = . ;  /* Past last read-write (loadable) segment */
   PROVIDE (__data_load_start = LOADADDR(.data) );
   PROVIDE (__data_size = _edata - __data_start );
  .bss   :
  {
     PROVIDE (__bss_start = .) ;
    *(.bss .bss.*)
    *(COMMON)
     . = ALIGN(2);
     PROVIDE (__bss_end = .) ;
  }  > REGION_DATA
   PROVIDE (__bss_size = SIZEOF(.bss) );
  .noinit   :
  {
     PROVIDE (__noinit_start = .) ;
    *(.noinit .noinit.*)
     . = ALIGN(2);
     PROVIDE (__noinit_end = .) ;
  }  > REGION_DATA
   . = ALIGN(2);
   _end = . ;   /* Past last write (loadable) segment */
  .infomem   :
  {
    *(.infomem)
     . = ALIGN(2);
    *(.infomem.*)
  }  > infomem
  .infomemnobits   :
  {
    *(.infomemnobits)
     . = ALIGN(2);
    *(.infomemnobits.*)
  }  > infomem
  .infoa   :
  {
    *(.infoa .infoa.*)
  }  > infoa
  .infob   :
  {
    *(.infob .infob.*)
  }  > infob
  .infoc   :
  {
    *(.infoc .infoc.*)
  }  > infoc
  .infod   :
  {
    *(.infod .infod.*)
  }  > infod
  .vectors  :
  {
     PROVIDE (__vectors_start = .) ;
    KEEP(*(.vectors*))
     _vectors_end = . ;
  }  > vectors
  .fartext :
  {
     . = ALIGN(2);
    *(.fartext)
     . = ALIGN(2);
    *(.fartext.*)
     _efartext = .;
  }  > REGION_FAR_ROM
  /* Stabs for profiling information*/
  .profiler 0 : { *(.profiler) }
  /* Stabs debugging sections.  */
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .stab.excl 0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment 0 : { *(.comment) }
  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo  0 : { *(.debug_srcinfo) }
  .debug_sfnames  0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  /* DWARF 2 */
  .debug_info     0 : { *(.debug_info) *(.gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_macinfo  0 : { *(.debug_macinfo) }
  /* DWARF 3 */
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }
   PROVIDE (__stack = ORIGIN(ram) + LENGTH(ram));
   PROVIDE (__data_start_rom = _etext);
   PROVIDE (__data_end_rom   = _etext + SIZEOF (.data));
}

