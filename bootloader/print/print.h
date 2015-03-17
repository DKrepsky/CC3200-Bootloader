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
 * \brief Send string trough UART A0.
 *
 * 	### Overview
 * 	Print module provides a simple way to send string through UART.
 *
 * 	### Requires
 * - Driverlib.
 *
 *	### Usage
 *	- Start the print module with PRINTInit passing a valid baud rate (check
 *	datasheet).
 *	- Use PRINT to send strings.
 *	- Use PRINTClose to power off the UARTA0 module.
 *
 * 	### Example
 *
 * \code
 *  // Initializes the PRINT with a baud rate of 115200.
 *  PRINTInit(115200);
 *
 *  // Print string.
 *  PRINT("Hello World!\n\r");
 *
 *  // Turn-off the UART module.
 *	PRINTClose();
 * \endcode
 *
 * \author David Krepsky
 * \version	1.0.0
 * \date 01/2015
 * \copyright Akenge Engenharia
 *
 * \bug None known.
 */

#ifndef _PRINT_H_
#define _PRINT_H_

/*!
 *	\file print.h
 *
 *	\brief Functions prototype for the print.c.
 *
 *	This file contains definitions used by the print.c.
 */

/*!
 *	\fn void PRINTInit(uint32_t baud)
 *
 * 	\brief Initiate the PRINT module.
 *
 *	- Enable the UARTA0 peripheral.
 *	- Configures the pin 55 (GPIO1) as UARTA0 Tx.
 *	- Set the baud rate, parity (no parity), number of bits (8) and the number
 *	  number of stop bits (one).
 *
 *	\param[in] baud UART baud rate (usually 115200 or 9600).
 */
void PRINTInit(uint32_t baud);

/*!
 *	\fn void PRINT(char *str)
 *
 * 	\brief Print a string through UARTA0.
 *
 *	Sends the null terminated string pointed by str to UARTA0.
 *
 *	\param[in] str Pointer to a null terminated string.
 *
 *	\todo Add memory scope check to the print functio.
 *
 */
void PRINT(char *str);

/*!
 *	\fn void PRINTClose(void)
 *
 * 	\brief Turn off the print module.
 *
 * 	Turn off the UARTA0 module and put the pin 55 back to type GPIO
 * 	input.
 *
 *	\todo Check for end of transmission before power off UARTA0.
 */
void PRINTClose(void);

#endif

/*!
 *	\}
 */
