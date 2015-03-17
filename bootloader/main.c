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
 *  \file main.c
 *
 *  \brief Bootloader.
 *
 *  This is the Akenge bootloader, used to boot an custom image and do a roll
 *  back when needed.
 *
 * \author David Krepsky
 * \version	1.0.3
 * \date 01/2015
 * \copyright Akenge Engenharia
 */

#include <stdint.h>

#include "unused.h"

#include "hw_types.h"
#include "hw_memmap.h"
#include "interrupt.h"
#include "prcm.h"
#include "simplelink.h"

#include "boot.h"

#include "rom.h"
#include "rom_map.h"
#include "print.h"

// Interrupt Vector from startup.asm.
extern void* intVector;

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

	// Initializes the PRINT with a baud rate of 115200.
	PRINTInit(115200);

	// Print header.
	PRINT("--------------------------------------------------------\r\n");
	PRINT("------------------ Akenge  Bootloader ------------------\r\n");
	PRINT("--------------------------------------------------------\r\n");
	PRINT("\r\n");
	PRINT("- Initializing Simplelink ...");

	// Start NWP to get access to flash.
	if (0 > sl_Start(NULL, NULL, NULL)) {
		PRINT("FAIL\r\n");
		PRCMSOCReset();
	}

	PRINT("OK\r\n");

	// Check if boot configuration exists.
	if (!BOOTExistCfg()) {

		PRINT("- boot.cfg not found, creating new ...");

		// If it doesn't exist, create the file to boot from factory.bin.
		bootinfo.bootimg = IMG_FACTORY;
		bootinfo.status = BOOT_OK;
		RetVal = BOOTWriteCfg(&bootinfo);

		// Failed to create file, Reset SOC.
		if (0 != RetVal) {
			PRINT("FAIL\r\n");
			PRCMSOCReset();
		}
		PRINT("OK\r\n");
	}

	PRINT("- Loading boot config ...");

	// Read configuration.
	RetVal = BOOTReadCfg(&bootinfo);
	if (0 != RetVal) {
		PRINT("FAIL\r\n");
		PRCMSOCReset();
	}
	PRINT("OK\r\n");

	PRINT("- Boot status: ");

	// Check boot status.
	switch (bootinfo.status) {

	// Last Boot OK.
	case BOOT_OK:
		PRINT("BOOT_OK\r\n");
		BOOTLoadImg(bootinfo.bootimg);
		break;

		// New Firmware Available.
	case BOOT_CHECK:
		PRINT("BOOT_CHECK\r\n");
		bootinfo.status = BOOT_CHECKING;

		if (0 != BOOTWriteCfg(&bootinfo))
			PRCMSOCReset();

		if (0 != BOOTLoadImg(IMG_CUSTOM))
			PRCMSOCReset();
		break;

		// Something wrong during last boot, go back to factory image.
	case BOOT_CHECKING:
	case BOOT_ERR:
		PRINT("BOOT_ERR\r\n");
		bootinfo.bootimg = IMG_FACTORY;
		bootinfo.status = BOOT_OK;

		if (0 != BOOTWriteCfg(&bootinfo))
			PRCMSOCReset();

		if (0 != BOOTLoadImg(IMG_FACTORY))
			PRCMSOCReset();
		break;

		// Unknow status (corrupted file maybe?).
	default:
		PRINT("BOOT_UNKNOWN\r\n");
		BOOTDeleteCfg();
		PRCMSOCReset();
		break;
	}

	PRINT("- Stop NWP...");

	// Stop NWP.
	sl_Stop(0);

	PRINT("OK\r\n");

	// Print the selected image.
	PRINT("Running ");

	if (bootinfo.bootimg == IMG_FACTORY)
		PRINT("Factory Image\r\n");
	else
		PRINT("Custom Image\r\n");

	// Turn-off the UART module.
	PRINTClose();

	// Run loaded image.
	BOOTRun((void*) BASE_ADDR);

	// Should never reach here. If so, reset soc
	PRCMSOCReset();

	return -1;
}

// Simplelink Hooks, not used by the bootloader but required by the simplelink.
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent) {

	UNUSED(pWlanEvent);
}

void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pHttpEvent,
		SlHttpServerResponse_t *pHttpResponse) {

	UNUSED(pHttpEvent);
	UNUSED(pHttpResponse);
}
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent) {

	UNUSED(pNetAppEvent);
}
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock) {

	UNUSED(pSock);
}
