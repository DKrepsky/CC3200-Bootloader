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
 * \addtogroup Boot
 * \{
 */

/*!
 * 	\file boot.c
 *
 * 	\brief Implementation of the boot functions.
 *
 * 	This file implements the functions used by the bootloader and the OTA
 * 	update.
 */

#include <stdint.h>
#include "unused.h"
#include "simplelink.h"
#include "boot.h"

/*!
 * 	\var static unsigned char bootfile[]
 *
 * 	\brief Path of the boot configuration file.
 *
 * 	The file is saved as boot.cfg in the root dir of the serial flash.
 */
static unsigned char bootfile[] = "boot.cfg";

/*!
 * 	\var static unsigned char IMG_FACTORY_NAME[]
 *
 * 	\brief Path to the factory image file.
 *
 * 	The file is saved as /sys/factory.bin.
 */
static unsigned char IMG_FACTORY_NAME[] = "/sys/factory.bin";

/*!
 * 	\var static unsigned char IMG_CUSTOM_NAME[]
 *
 * 	\brief Path to the custom image file.
 *
 * 	The file is saved as /sys/custom.bin.
 */
static unsigned char IMG_CUSTOM_NAME[] = "/sys/custom.bin";

/*
 * Check if the configuration file exists.
 */
int32_t BOOTExistCfg() {
	int32_t RetVal;
	SlFsFileInfo_t FileInfo;

	/* If file info exists, the file exists. */
	RetVal = sl_FsGetInfo(bootfile, 0, &FileInfo);

	return (0 == RetVal) ? 1 : 0;
}

/*
 * Allocate space for the configuration file.
 */
int32_t BOOTCreateCfg() {
	int32_t RetVal;
	int32_t hFile;

	/* Create a public file with max size of 512 bytes. */
	RetVal = sl_FsOpen(bootfile,
			FS_MODE_OPEN_CREATE(512,
					_FS_FILE_PUBLIC_WRITE | _FS_FILE_PUBLIC_READ), NULL,
			&hFile);

	/* Return the file handler if success. */
	return (0 != RetVal) ? -1 : hFile;
}

/*
 * Delete the configuration.
 */
int32_t BOOTDeleteCfg() {
	/* Delete the configuration file. */
	return sl_FsDel(bootfile, 0);
}

/*
 * Read the configuration into a bootinfo_t structure.
 */
int32_t BOOTReadCfg(bootinfo_t *bootinfo) {
	int32_t RetVal;
	int32_t hFile;

	/* If boot.cfg doesn't exists, return error. */
	if (!BOOTExistCfg())
		return -1;

	/* Open the file in read mode. */
	RetVal = sl_FsOpen(bootfile, FS_MODE_OPEN_READ, NULL, &hFile);
	if (RetVal != 0)
		return RetVal;

	/* Reat it. */
	RetVal = sl_FsRead(hFile, 0, (unsigned char*) bootinfo, sizeof(bootinfo_t));

	/* Close it. */
	sl_FsClose(hFile, NULL, NULL, 0);
	return (RetVal < 0) ? RetVal : 0;
}

/*
 * Write the configuration if the file does exists or create and then write if
 * no file is found.
 */
int32_t BOOTWriteCfg(bootinfo_t *bootinfo) {
	int32_t RetVal;
	int32_t hFile;

	/* Check whether file exists or not. */
	if (BOOTExistCfg()) {

		/* If exists, open it. */
		RetVal = sl_FsOpen(bootfile, FS_MODE_OPEN_WRITE, NULL, &hFile);
		if (0 != RetVal)
			return -1;
	} else {
		/* Create a new one otherwise. */
		hFile = BOOTCreateCfg();
		if (-1 == hFile)
			return -1;
	}

	/* Write the configuration. */
	RetVal = sl_FsWrite(hFile, 0, (unsigned char*) bootinfo,
			sizeof(bootinfo_t));

	/* Close the file. */
	sl_FsClose(hFile, NULL, NULL, 0);

	/* Return 0 case everything is ok. */
	return (0 > RetVal) ? -1 : 0;
}

/*
 * Load an image from the serial flash to the SRAM.
 * The image type must be IMG_FACTORY or IMG_CUSTOM.
 */
int32_t BOOTLoadImg(imgtype_t img) {
	int32_t hFile;
	int32_t RetVal;
	SlFsFileInfo_t FileInfo;

	/* Pointer to the SRAM position where the image will be loaded. */
	unsigned char *BaseAddr = (unsigned char*) BASE_ADDR;

	switch (img) {

	/*
	 * Open the correct file according to the image type.
	 * Return error if wrong image type is passed.
	 */
	case IMG_FACTORY:
		RetVal = sl_FsOpen(IMG_FACTORY_NAME, FS_MODE_OPEN_READ, 0, &hFile);
		if (0 != RetVal)
			return RetVal;
		sl_FsGetInfo(IMG_FACTORY_NAME, 0, &FileInfo);
		break;

	case IMG_CUSTOM:
		RetVal = sl_FsOpen(IMG_CUSTOM_NAME, FS_MODE_OPEN_READ, 0, &hFile);
		if (0 != RetVal)
			return RetVal;
		sl_FsGetInfo(IMG_CUSTOM_NAME, 0, &FileInfo);
		break;

	default:
		return -1;
	}

	/* Load the image to the SRAM. */
	RetVal = sl_FsRead(hFile, 0, BaseAddr, FileInfo.FileLen);
	if (0 > RetVal)
		return RetVal;

	/* Close the handler. */
	sl_FsClose(hFile, 0, 0, 0);

	/* Return success. */
	return 0;
}

/*
 * Run an binary image file located at BaseAddr, in SRAM.
 */
void BOOTRun(void* BaseAddr) {
	/*
	 * Here, BaseAddr is passed through register r0.
	 * This is the default behavior of GCC EABI.
	 *
	 * BaseAddr is treated as a Interrupt Vector Table, so, the 1st position is
	 * the Stack Pointer position and the second is the ResetISR location.
	 */

	UNUSED(BaseAddr);

	// Load SP.
	__asm(
			"	ldr        r1, [r0]\n\r"
			"	mov        sp, r1\n\r"
	);

	// Jump to ResetISR.
	__asm(
			"ldr	r1, [r0, #4]\n\r"
			"bx		r1\n\r"
	);
}

/*!
 * \}
 */
