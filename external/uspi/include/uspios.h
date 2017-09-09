//
// uspios.h
//
// External functions used by the USPi library
//
// USPi - An USB driver for Raspberry Pi written in C
// Copyright (C) 2014-2016  R. Stange <rsta2@o2online.de>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#ifndef _uspios_h
#define _uspios_h

#ifdef __cplusplus
extern "C" {
#endif

//
// System configuration
//
// (Change this before you build the USPi library!)
//
#define GPU_L2_CACHE_ENABLED		// normally enabled (can be disabled in config.txt)

#define HZ	100			// timer ticks / second (set this to your timer interrupt frequency)

// Default keyboard map (enable only one)
//#define USPI_DEFAULT_KEYMAP_DE
//#define USPI_DEFAULT_KEYMAP_ES
//#define USPI_DEFAULT_KEYMAP_FR
//#define USPI_DEFAULT_KEYMAP_IT
#define USPI_DEFAULT_KEYMAP_US
//#define USPI_DEFAULT_KEYMAP_US

#include <liblox/memory.h>

//
// Timer
//
void UspiMsDelay(unsigned nMilliSeconds);
void UspiUsDelay(unsigned nMicroSeconds);

typedef void TKernelTimerHandler (unsigned hTimer, void *pParam, void *pContext);

// returns the timer handle (hTimer)
unsigned UspiStartKernelTimer(unsigned nHzDelay,    // in HZ units (see "system configuration" above)
                              TKernelTimerHandler* pHandler,
                              void* pParam, void* pContext);	// handed over to the timer handler

void UspiCancelKernelTimer(unsigned hTimer);

//
// Interrupt handling
//
typedef void TInterruptHandler (void *pParam);

// USPi uses USB IRQ 9
void UspiConnectInterrupt(unsigned nIRQ, TInterruptHandler* pHandler, void* pParam);

//
// Property tags (ARM -> VC)
//
// See: https://github.com/raspberrypi/firmware/wiki/Mailboxes
//

// returns 0 on failure
int UspiSetPowerStateOn(unsigned nDeviceId);	// "set power state" to "on", wait until completed

// returns 0 on failure
int UspiGetMACAddress(unsigned char* Buffer);	// "get board MAC address"

//
// Logging
//

// Severity (change this before building if you want different values)
#define LOG_ERROR	1
#define LOG_WARNING	2
#define LOG_NOTICE	3
#define LOG_DEBUG	4

void UspiLogWrite(const char* pSource,        // short name of module
                  unsigned Severity,        // see above
                  const char* pMessage, ...);	// uses printf format options

#ifdef __cplusplus
}
#endif

#endif
