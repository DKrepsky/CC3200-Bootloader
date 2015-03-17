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
 * \defgroup Boot Boot
 * \{
 *
 * \brief Functions to read/write the boot.cfg file and load/run an image into
 * SRAM.
 *
 * ### Overview
 * In order to manage the boot process and firmware OTA update, the
 * functions implemented here are needed.
 *
 * The bootloader will search for a valid boot.cfg file in the root
 * directory of the serial flash memory. This file is composed of a
 * bootinfo_t structure that keeps the current boot status (as described
 * in bootstatus_t) and the working image (factory or custom). The
 * bootloader then uses this information to load and run the image.
 *
 * OTA update must set the boot status to BOOT_CHECK and select the
 * IMG_CUSTOM in order to validate the new firmware.
 *
 * ### Requires
 * - Driverlib;
 * - Simplelink (Can be the TINY build).
 *
 * ### Usage
 * First start the simplelink stack with an sl_Start(NULL, NULL, NULL) in
 * order to access the serial flash memory. Then, use the desired
 * functions.
 *
 * ### Example
 *
 * \code
 *  bootinfo_t bootinfo;
 *
 *  // Start NWP to get access to flash.
 *  sl_Start(NULL, NULL, NULL);
 *
 *	if (BOOTExistCfg())
 *	{
 *		BOOTReadCfg(&bootinfo);
 *	}
 *
 *	// Stop NWP.
 *	sl_Stop(0);
 * \endcode
 *
 * \author	David Krepsky
 * \version	1.0.1
 * \date		01/2015
 * \copyright Akenge Engenharia
 *
 * \todo Add support for secure files.
 * \todo Load images different from custom.bin and factory.bin.
 * \todo Add ASSERT code to validate parameters.
 *
 * \bug None known.
 * \}
 */

#ifndef _BOOT_H_
#define _BOOT_H_

/*!
 *	\file boot.h
 *
 *	\brief Constants, functions prototypes and types used to read the boot.cfg
 *	 and load/run an image.
 *
 *	This file contains definitions used by the boot.c.
 */

/*!
 *	\def BASE_ADDR
 *
 * 	\brief SRAM base address to load images.
 *
 * 	Defines the address where an image will be loaded into SRAM.
 */
#define BASE_ADDR	0x20004000

/*!
 *	\enum bootstatus_t
 *
 *	\brief Status of the current boot.
 */
typedef enum {
	/*! Last boot was ok, boot the same image. */
	BOOT_OK,
	/*! New firmware available, boot custom.bin. */
	BOOT_CHECK,
	/*! If the bootloader finds this, something went wrong during firmware
	 *  check. */
	BOOT_CHECKING,
	/*! Error in custom.bin, load factory image. */
	BOOT_ERR
} bootstatus_t;


/*!
 *	\enum imgtype_t
 *
 *	\brief Indicates the image type (factory or custom).
 */
typedef enum {
	/*! Factory image in file /sys/factory.bin. */
	IMG_FACTORY,
	/*! Custom image in file /sys/custom.bin. */
	IMG_CUSTOM
} imgtype_t;

/*!
 *	\struct bootinfo_t
 *
 *	\brief Structure used in the boot.cfg file.
 */
typedef struct {
	/*! Status of the current boot. */
	bootstatus_t status;
	/*! Type of the image to boot. */
	imgtype_t bootimg;
} bootinfo_t;

/*!
 *	\fn int32_t BOOTExistCfg(void)
 *
 * 	\brief Check if boot.cfg exists.
 *
 *	Uses the sl_FsGetInfo function to check whether boot.cfg exists in the
 * 	flash memory;
 *
 *	\return 1 if the file exists, 0 otherwise.
 */
int32_t BOOTExistCfg(void);

/*!
 *	\fn	int32_t BOOTCreateCfg(void)
 *
 * 	\brief Function used to create a new boot.cfg file.
 *
 *	Creates a new boot.cfg file in the flash memory. This file max size will be
 *	512 for a better use of the flash space. The created file is public to read
 *	and write.
 *
 *	\return Returns the file handler if success. Returns the SL error otherwise.
 *
 * 	\warning Only use this when BOOTExistCfg(void) returns false.
 */
int32_t BOOTCreateCfg(void);

/*!
 *	\fn int32_t BOOTDeleteCfg(void)
 *
 * 	\brief Removes the boot.cfg file from flash.
 *
 * 	This function will delete the boot.cfg file from the serial flash. Useful
 * 	when the file gets corrupted.
 *
 * 	\warning The current configuration will be deleted. Use with care.
 */
int32_t BOOTDeleteCfg(void);

/*!
 *	\fn int32_t BOOTReadCfg(bootinfo_t *bootinfo)
 *
 * 	\brief Reads the boot.cfg file.
 *
 * 	Reads the file from flash and stores it in the structure pointed by
 * 	bootinfo.
 *
 * 	\param[out] bootinfo Structure to hold the boot.cfg file data.
 *
 * 	\return 0 on success, SL error code otherwise.
 */
int32_t BOOTReadCfg(bootinfo_t *bootinfo);

/*!
 *	\fn int32_t BOOTWriteCfg(bootinfo_t *bootinfo)
 *
 * 	\brief Writes the boot configuration.
 *
 * 	Writes the boot.cfg file with the contents of the bootinfo structure.
 *
 * 	\param[in] bootinfo Structure that contains the boot.cfg file data.
 *
 * 	\return 0 on success, SL error code otherwise.
 */
int32_t BOOTWriteCfg(bootinfo_t *bootinfo);

/*!
 *	\fn int32_t BOOTLoadImg(imgtype_t img)
 *
 * 	\brief Load an application image from the flash.
 *
 * 	This function will load a custom firmware (custom.bin) or the factory
 * 	firmware (factory.bin) in the SRAM at position BASE_ADDR, depending on the
 * 	parameter img.
 */
int32_t BOOTLoadImg(imgtype_t img);

/*!
 *  \fn void BOOTRun(void* BaseAddr)
 *
 *  \brief Run an binary image from another location.
 *
 *  This function will use the interrupt vector of the application that will
 *  run to set the Stack Pointer Register (SP) and to find out the application
 *  entry point (which is the ResetISR pointer).
 *
 *   \param[in] BaseAddr The application base address in the SRAM.
 */
void BOOTRun(void* BaseAddr);

#endif

/*!
 * \}
 */

