/*******************************************************************************
*   Bootloader Memory Map
*******************************************************************************/

MEMORY
{
    /*  Bootloader starts at 0x20000000 and MUST be smaller than 16KB   */
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 16K
}

SECTIONS
{
    .text :
    {
    	. = ALIGN(4);
        _text = .;
        /*  .intvecs MUST be at position 0x20000000 */
        KEEP(*(.intvecs))
        *(.text*)
        *(.rodata*)
        _etext = .;
    } > SRAM

    .data :
    {
        _data = .;
        *(.data*)
        _edata = .;
    } > SRAM
	
    .bss :
    {
        _bss = .;
        *(.bss*)
        _ebss = .;
    } > SRAM
}

