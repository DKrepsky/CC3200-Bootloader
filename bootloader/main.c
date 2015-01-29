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
 *  \file
 *
 *  \brief Startup file for the bootloader.
 *
 *  This file contains the interrupt vector table, the function to relocate the
 *  bootloader in the SRAM and the function to run and image from another
 *  position in memory.
 *
 *  Version: 1.0.0
 *
 *  Author: David Krepsky
 */

#include <stdint.h>

#include "hw_types.h"
#include "hw_memmap.h"
#include "rom.h"
#include "rom_map.h"
#include "interrupt.h"
#include "prcm.h"
#include "simplelink.h"

#include "nano_print.h"
#include "bootfile.h"

// Interrupt Vector from startup.asm.
extern void* intVector;

// Run function from startup.asm.
extern void Run(int32_t BaseAddr);

/*!
 *  \fn int main (void)
 *
 *  \brief Main function.
 *
 *  The main function is responsible for checking the boot.cfg file to select
 *  the adequate image to run. It then runs the appropriate image.
 */
int main() {
	int32_t RetVal; // Used to check return values.
	bootinfo_t bootinfo; // Bootinfo structure.

	// Initializes the board.
	MAP_IntVTableBaseSet((int32_t) &intVector);
	PRCMCC3200MCUInit();

	// Initializes the NANOPint with a baud rate of 115200.
	NANOPrintInit(115200);

	// Print header.
	NANOPrint("--------------------------------------------------------\n\r");
	NANOPrint("------------------ Akenge  Bootloader ------------------\n\r");
	NANOPrint("--------------------------------------------------------\n\r");
	NANOPrint("\n\r");
	NANOPrint("- Initializing Simplelink...");

	// Start NWP to get access to flash.
	sl_Start(NULL, NULL, NULL);

	NANOPrint("OK\n\r");

	// Check if boot config exists.
	if (!BOOTExistCfg()) {

		NANOPrint("- boot.cfg not found, creating new...");

		// If it doesn't exist, create the file to boot from factory.bin.
		bootinfo.bootimg = IMG_FACTORY;
		bootinfo.status = BOOT_OK;
		RetVal = BOOTWriteCfg(&bootinfo);

		// Failed to create file, Reset SOC.
		if (0 != RetVal)
			PRCMSOCReset();
		NANOPrint("OK\n\r");
	}

	NANOPrint("- Loading boot config...");

	// Read configuration.
	RetVal = BOOTReadCfg(&bootinfo);
	if (0 != RetVal)
		PRCMSOCReset();
	NANOPrint("OK\n\r");

	NANOPrint("- Boot status: ");

	// Check boot status.
	switch (bootinfo.status) {

	// Last Boot OK.
	case BOOT_OK:
		NANOPrint("BOOT_OK\n\r");
		BOOTLoadImg(bootinfo.bootimg);
		break;

		// New Firmware Available.
	case BOOT_CHECK:
		NANOPrint("BOOT_CHECK\n\r");
		bootinfo.status = BOOT_CHECKING;

		if (0 != BOOTWriteCfg(&bootinfo))
			PRCMSOCReset();

		if (0 != BOOTLoadImg(IMG_CUSTOM))
			PRCMSOCReset();
		break;

		// Something wrong during last boot, go back to factory image.
	case BOOT_CHECKING:
	case BOOT_ERR:
		NANOPrint("BOOT_ERR\n\r");
		bootinfo.bootimg = IMG_FACTORY;
		bootinfo.status = BOOT_OK;

		if (0 != BOOTWriteCfg(&bootinfo))
			PRCMSOCReset();

		if (0 != BOOTLoadImg(IMG_FACTORY))
			PRCMSOCReset();
		break;

		// Unknow status (corrupted file maybe?).
	default:
		NANOPrint("BOOT_UNKNOWN\n\r");
		BOOTDeleteCfg();
		PRCMSOCReset();
		break;
	}

	NANOPrint("- Stop NWP...");

	// Stop NWP.
	sl_Stop(0);

	NANOPrint("OK\n\r");

	// Print the selected image.
	NANOPrint("Running ");

	if (bootinfo.bootimg == IMG_FACTORY)
		NANOPrint("Factory Image\n\r");
	else
		NANOPrint("Custom Image\n\r");

	// Turn-off the UART module.
	NANOPrintClose();

	// Run loaded image.
	Run(BASE_ADDR);

	// Should never reach here. If so, reset soc
	PRCMSOCReset();

	return -1;
}

// Simplelink Hooks, not used by the bootloader but required by the simplelink.
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent) {

}

void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
		SlHttpServerResponse_t *pHttpResponse) {

}
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent) {

}
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock) {

}
