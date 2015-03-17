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
 * 	\defgroup Print Print
 * 	\{
 */

/*!
 *	\file print.c
 *
 *	\brief Functions implementation for the print module.
 *
 *	This file contains the implementation of the functions used in by PRINT.
 */
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "hw_types.h"
#include "hw_memmap.h"
#include "rom.h"
#include "rom_map.h"
#include "pin.h"
#include "gpio.h"
#include "prcm.h"
#include "uart.h"

#include "print.h"

/*
 * Initializes UARTA0 with \ref baud baud rate, 8 bits, 1 stop and no parity.
 * Also configure pin 55 (GPIO_PIN_01) as Tx.
 */
void PRINTInit(uint32_t baud) {

	/* Enable UARTA.0 */
	MAP_PRCMPeripheralClkEnable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);

	/* Pin 55 as Tx. */
	MAP_PinTypeUART(PIN_55, PIN_MODE_3);

	/* Configure and enable UARTA0. */
	MAP_UARTConfigSetExpClk(UARTA0_BASE,
	MAP_PRCMPeripheralClockGet(PRCM_UARTA0), baud,
			(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
			UART_CONFIG_PAR_NONE));

	MAP_UARTFIFODisable(UARTA0_BASE);
}

/*
 * Print string.
 */
void PRINT(char *str) {

	/* Send char until '\0'. */
	while (*str != '\0') {
		UARTCharPut(UARTA0_BASE, *str++);
	}

}

/*
 * Turn off UARTA0 and put pin 55 in input mode (high impedance).
 */
void PRINTClose() {

	while(MAP_UARTBusy(UARTA0_BASE));

	/* Power down UARTA0. */
	MAP_PRCMPeripheralClkDisable(PRCM_UARTA0, PRCM_RUN_MODE_CLK);

	/* Pin 55 as input. */
	MAP_PinTypeGPIO(PIN_55, PIN_MODE_0, false);
}

/*!
 *	\}
 */

