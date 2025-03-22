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
   module: MPU401.C

   author: James R. Dose
   date:   January 1, 1994

   Low level routines to support sending of MIDI data to MPU401
   compatible MIDI interfaces.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <conio.h>
#include <dos.h>

#include "i_system.h"
#include "a_mpu401.h"

#define MPU_NotFound       -1
#define MPU_UARTFailed     -2

#define MPU_ReadyToWrite   0x40
#define MPU_ReadyToRead    0x80
#define MPU_CmdEnterUART   0x3f
#define MPU_CmdReset       0xff
#define MPU_CmdAcknowledge 0xfe

#define MIDI_NOTE_OFF         0x80
#define MIDI_NOTE_ON          0x90
#define MIDI_POLY_AFTER_TCH   0xA0
#define MIDI_CONTROL_CHANGE   0xB0
#define MIDI_PROGRAM_CHANGE   0xC0
#define MIDI_AFTER_TOUCH      0xD0
#define MIDI_PITCH_BEND       0xE0

#define MPU_DefaultAddress 0x330
static int32_t MPU_BaseAddr = MPU_DefaultAddress;

#define MPU_Delay 0x5000;


/*---------------------------------------------------------------------
   Function: MPU_SendMidi

   Sends a byte of MIDI data to the music device.
---------------------------------------------------------------------*/

static void MPU_SendMidi(int32_t data)
{
	int32_t  port  = MPU_BaseAddr + 1;
	uint32_t count = MPU_Delay;

	while (count > 0)
	{
		// check if status port says we're ready for write
		if (!(inp(port) & MPU_ReadyToWrite))
			break;

		count--;
	}

	port--;

	// Send the midi data
	outp(port, data);
}


/*---------------------------------------------------------------------
   Function: MPU_NoteOff

   Sends a full MIDI note off event out to the music device.
---------------------------------------------------------------------*/

void MPU_NoteOff(int32_t channel, int32_t key, int32_t velocity)
{
	MPU_SendMidi(MIDI_NOTE_OFF | channel);
	MPU_SendMidi(key);
	MPU_SendMidi(velocity);
}


/*---------------------------------------------------------------------
   Function: MPU_NoteOn

   Sends a full MIDI note on event out to the music device.
---------------------------------------------------------------------*/

void MPU_NoteOn(int32_t channel, int32_t key, int32_t velocity)
{
	MPU_SendMidi(MIDI_NOTE_ON | channel);
	MPU_SendMidi(key);
	MPU_SendMidi(velocity);
}


/*---------------------------------------------------------------------
   Function: MPU_PolyAftertouch

   Sends a full MIDI polyphonic aftertouch event out to the music device.
---------------------------------------------------------------------*/

void MPU_PolyAftertouch(int32_t channel, int32_t key, int32_t pressure)
{
	MPU_SendMidi(MIDI_POLY_AFTER_TCH | channel);
	MPU_SendMidi(key);
	MPU_SendMidi(pressure);
}


/*---------------------------------------------------------------------
   Function: MPU_ControlChange

   Sends a full MIDI control change event out to the music device.
---------------------------------------------------------------------*/

void MPU_ControlChange(int32_t channel, int32_t number, int32_t value)
{
	MPU_SendMidi(MIDI_CONTROL_CHANGE | channel);
	MPU_SendMidi(number);
	MPU_SendMidi(value);
}


/*---------------------------------------------------------------------
   Function: MPU_ProgramChange

   Sends a full MIDI program change event out to the music device.
---------------------------------------------------------------------*/

void MPU_ProgramChange(int32_t channel, int32_t program)
{
	MPU_SendMidi(MIDI_PROGRAM_CHANGE | channel);
	MPU_SendMidi(program);
}


/*---------------------------------------------------------------------
   Function: MPU_ChannelAftertouch

   Sends a full MIDI channel aftertouch event out to the music device.
---------------------------------------------------------------------*/

void MPU_ChannelAftertouch(int32_t channel, int32_t pressure)
{
	MPU_SendMidi(MIDI_AFTER_TOUCH | channel);
	MPU_SendMidi(pressure);
}


/*---------------------------------------------------------------------
   Function: MPU_PitchBend

   Sends a full MIDI pitch bend event out to the music device.
---------------------------------------------------------------------*/

void MPU_PitchBend(int32_t channel, int32_t lsb, int32_t msb)
{
	MPU_SendMidi(MIDI_PITCH_BEND | channel);
	MPU_SendMidi(lsb);
	MPU_SendMidi(msb);
}


/*---------------------------------------------------------------------
   Function: MPU_SendCommand

   Sends a command to the MPU401 card.
---------------------------------------------------------------------*/

static void MPU_SendCommand(int32_t data)
{
	int32_t  port = MPU_BaseAddr + 1;
	uint32_t count = 0xffff;

	while (count > 0)
	{
		// check if status port says we're ready for write
		if (!(inp(port) & MPU_ReadyToWrite))
			break;

		count--;
	}

	outp(port, data);
}


/*---------------------------------------------------------------------
   Function: MPU_Reset

   Resets the MPU401 card.
---------------------------------------------------------------------*/

int32_t MPU_Reset(void)
{
	int32_t  port = MPU_BaseAddr + 1;
	uint32_t count = 0xffff;

	// Output "Reset" command via Command port
	MPU_SendCommand(MPU_CmdReset);

	// Wait for status port to be ready for read   
	while (count > 0)
	{
		if (!(inp(port) & MPU_ReadyToRead))
		{
			port--;

			// Check for a successful reset
			if (inp(port) == MPU_CmdAcknowledge)
				return MPU_Ok;

			port++;
		}
		count--;
	}

	// Failed to reset : MPU-401 not detected
   return MPU_NotFound;
}


/*---------------------------------------------------------------------
   Function: MPU_EnterUART

   Sets the MPU401 card to operate in UART mode.
---------------------------------------------------------------------*/

static int32_t MPU_EnterUART(void)
{
	int32_t  port = MPU_BaseAddr + 1;
	uint32_t count = 0xffff;

	// Output "Enter UART" command via Command port
	MPU_SendCommand(MPU_CmdEnterUART);

	// Wait for status port to be ready for read
	while (count > 0)
	{
		if (!(inp(port) & MPU_ReadyToRead))
		{
			port--;

			// Check for a successful reset
			if (inp(port) == MPU_CmdAcknowledge)
				return MPU_Ok;

			port++;
		}
		count--;
	}

	// Failed to reset : MPU-401 not detected
	return MPU_UARTFailed;
}


/*---------------------------------------------------------------------
   Function: MPU_Init

   Detects and initializes the MPU401 card.
---------------------------------------------------------------------*/

int32_t MPU_Init(int32_t addr)
{
	int32_t status;
	int32_t count = 4;

	MPU_BaseAddr = addr;

	while (count > 0)
	{
		status = MPU_Reset();
		if (status == MPU_Ok)
			return MPU_EnterUART();

		count--;
	}

	return status;
}
