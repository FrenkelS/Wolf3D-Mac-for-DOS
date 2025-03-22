/*
Copyright (C) 1994-1995 Apogee Software, Ltd.
Copyright (C) 2023-2025 Frenkel Smeijers

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
/**********************************************************************
   module: DMA.C

   author: James R. Dose
   date:   February 4, 1994

   Low level routines to for programming the DMA controller for 8 bit
   and 16 bit transfers.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <conio.h>
#include <dos.h>

#include "i_system.h"
#include "a_dma.h"

#define DMA_MaxChannel 7

#define VALID   ( 1 == 1 )
#define INVALID ( !VALID )

#define BYTE 0
#define WORD 1

typedef struct
{
	int32_t Valid;
	int32_t Width;
	int32_t Mask;
	int32_t Mode;
	int32_t Clear;
	int32_t Page;
	int32_t Address;
	int32_t Length;
} DMA_PORT;

static const DMA_PORT DMA_PortInfo[DMA_MaxChannel + 1] =
{
	{  VALID, BYTE,  0xA,  0xB,  0xC, 0x87,  0x0,  0x1},
	{  VALID, BYTE,  0xA,  0xB,  0xC, 0x83,  0x2,  0x3},
	{INVALID, BYTE,  0xA,  0xB,  0xC, 0x81,  0x4,  0x5},
	{  VALID, BYTE,  0xA,  0xB,  0xC, 0x82,  0x6,  0x7},
	{INVALID, WORD, 0xD4, 0xD6, 0xD8, 0x8F, 0xC0, 0xC2},
	{  VALID, WORD, 0xD4, 0xD6, 0xD8, 0x8B, 0xC4, 0xC6},
	{  VALID, WORD, 0xD4, 0xD6, 0xD8, 0x89, 0xC8, 0xCA},
	{  VALID, WORD, 0xD4, 0xD6, 0xD8, 0x8A, 0xCC, 0xCE}
};


/*---------------------------------------------------------------------
   Function: DMA_VerifyChannel

   Verifies whether a DMA channel is available to transfer data.
---------------------------------------------------------------------*/

int32_t DMA_VerifyChannel(int32_t channel)
{
	if (!(0 <= channel && channel <= DMA_MaxChannel))
		return DMA_Error;
	else if (DMA_PortInfo[channel].Valid == INVALID)
		return DMA_Error;
	else
		return DMA_Ok;
}


/*---------------------------------------------------------------------
   Function: DMA_SetupTransfer

   Programs the specified DMA channel to transfer data.
---------------------------------------------------------------------*/

int32_t DMA_SetupTransfer(int32_t channel, uint8_t *address, int32_t length)
{
	const DMA_PORT *Port;

	int32_t addr;
	int32_t ChannelSelect;
	int32_t Page;
	int32_t HiByte;
	int32_t LoByte;
	int32_t TransferLength;
	int32_t status;

	status = DMA_VerifyChannel(channel);

	if (status == DMA_Ok)
	{
		Port = &DMA_PortInfo[channel];
		ChannelSelect = channel & 0x3;

		addr = ((int32_t)address) - __djgpp_conventional_base;

		if (Port->Width == WORD)
		{
			Page   = (addr >> 16) & 255;
			HiByte = (addr >> 9)  & 255;
			LoByte = (addr >> 1)  & 255;

			// Convert the length in bytes to the length in words
			TransferLength = (length + 1) >> 1;

			// The length is always one less the number of bytes or words
			// that we're going to send
			TransferLength--;
		} else {
			Page   = (addr >> 16) & 255;
			HiByte = (addr >> 8)  & 255;
			LoByte = addr & 255;

			// The length is always one less the number of bytes or words
			// that we're going to send
			TransferLength = length - 1;
		}

		// Mask off DMA channel
		outp(Port->Mask, 4 | ChannelSelect);

		// Clear flip-flop to lower byte with any data
		outp(Port->Clear, 0);

		// Set DMA mode to AutoInitRead
		outp(Port->Mode, 0x58 | ChannelSelect);

		// Send address
		outp(Port->Address, LoByte);
		outp(Port->Address, HiByte);

		// Send page
		outp(Port->Page, Page);

		// Send length
		outp(Port->Length, LOBYTE(TransferLength));
		outp(Port->Length, HIBYTE(TransferLength));

		// enable DMA channel
		outp(Port->Mask, ChannelSelect);
	}

	return status;
}


/*---------------------------------------------------------------------
   Function: DMA_EndTransfer

   Ends use of the specified DMA channel.
---------------------------------------------------------------------*/

int32_t DMA_EndTransfer(int32_t channel)
{
	const DMA_PORT *Port;

	int32_t ChannelSelect;
	int32_t status;

	status = DMA_VerifyChannel(channel);
	if (status == DMA_Ok)
	{
		Port = &DMA_PortInfo[channel];
		ChannelSelect = channel & 0x3;

		// Mask off DMA channel
		outp(Port->Mask, 4 | ChannelSelect);

		// Clear flip-flop to lower byte with any data
		outp(Port->Clear, 0);
	}

	return status;
}


/*---------------------------------------------------------------------
   Function: DMA_GetCurrentPos

   Returns the position of the specified DMA transfer.
---------------------------------------------------------------------*/

uint8_t *DMA_GetCurrentPos(int32_t channel)
{
	const DMA_PORT *Port;

	uint32_t addr   = 0;
	 int32_t status = DMA_VerifyChannel(channel);

	if (status == DMA_Ok)
	{
		Port = &DMA_PortInfo[channel];

		if (Port->Width == WORD)
		{
			// Get address
			addr  = inp(Port->Address) << 1;
			addr |= inp(Port->Address) << 9;

			// Get page
			addr |= inp(Port->Page) << 16;
		} else {
			// Get address
			addr  = inp(Port->Address);
			addr |= inp(Port->Address) << 8;

			// Get page
			addr |= inp(Port->Page) << 16;
		}
	}

	return (uint8_t *)(addr + __djgpp_conventional_base);
}
