/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Akenge Engenharia
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/*!
 *  \file startup.asm
 *
 *  \brief Startup file for the bootloader.
 *
 *  This file contains the interrupt vector table, the function to relocate the
 *  bootloader in the SRAM and the function to run and image from another
 *  position in memory.
 *
 *  Version: 1.0.2
 *
 *  Author: David Krepsky
 */

/*!
 *  \def INITIAL_POS 0x20004000
 *
 *  \brief Bootloader image initial position.
 *
 *  Initial position of the image. By default the TI bootloader will load the
 *  binary file to 0x20004000.
 */
.set    INITIAL_POS,    0x20004000

/*!
 *  \def RELOCATED_POS 0x20000000
 *
 *  \brief Bootloader final position.
 *
 *  Position of the bootloader after the relocation process.
 */
.set    RELOCATED_POS,    0x20000000

/*
 *  Bootloader external main function.
 */
.extern main

/*
 * External BOOTRun function.
 */
.extern  BOOTRun

/*!
 *  \brief Interrupt vector
 *
 *  Interrupt vector with only the SP and the ResetISR (others aren't used).
 */
.section .intvecs
intVector: .global intVector
    /*
     *  Stack pointer start at the end of bootloader memory.
     */
    .word   0x20004004

    /*
     *  Entry point is the relocator function (added of 0x4000 because of the
     * initial load position).
     */
    .word   Relocator+0x4001


.text

/*!
 *  \fn void Relocator (void)
 *
 *  \brief Move the bootloader from 0x20004000 to 0x20000000.
 *
 *  This function will copy word by word from 0x20004000 to 0x20000000. Then it
 *  uses Run to execute the relocated bootloader.
 */
Relocator: .global  Relocator
    .align 4
    // r1 = New position in memory
    ldr        r1, =RELOCATED_POS
    // r2 = Current position in memory
    ldr        r2, =INITIAL_POS
    // r3 = End Addr for relocated bootloader
    mov        r3, r2

    // Loop to move every word from the initial position to the new position.
reloc:
    // Copy from position pointed by r2 to position pointed by r1.
    ldr        r4, [r2]
    str        r4, [r1]

    // Increment pointers.
    add        r1, r1, #4
    add        r2, r2, #4

    // Check for end.
    cmp        r1,r3
    bne        reloc

    // Load the new IntVector table address into r0.
    ldr        r0, =RELOCATED_POS

    // Load main address into the ResetISR pointer (IntVector[1]).
    ldr        r1, =main
    str        r1, [r0, #4]

    // Run the relocated main.
    b          BOOTRun

.end
