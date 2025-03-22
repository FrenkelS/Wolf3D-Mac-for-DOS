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
   module: PCFX.C

   author: James R. Dose
   date:   April 1, 1994

   Low level routines to support PC sound effects created by Muse.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <conio.h>
#include <dos.h>
#include <string.h>

#include "i_system.h"
#include "a_inter.h"
#include "a_pcfx.h"
#include "a_taskmn.h"


static Word		PCFX_SoundNum;
static Word		PCFX_LengthLeft;
static Word		PCFX_Priority;
static uint8_t	data[306];
static uint8_t	*PCFX_Sound = NULL;
static uint8_t	PCFX_LastSample;
static task __far* PCFX_ServiceTask = NULL;

static Boolean	PCFX_Installed = FALSE;


/*---------------------------------------------------------------------
   Function: PCFX_Stop

   Halts playback of the currently playing sound effect.
---------------------------------------------------------------------*/

void PCFX_Stop(Word SoundNum)
{
	uint32_t flags;

	if (PCFX_Sound == NULL)
		return;

	if (PCFX_SoundNum != SoundNum)
		return;

	flags = DisableInterrupts();

	// Turn off speaker
	outp(0x61, inp(0x61) & 0xfc);

	PCFX_Sound      = NULL;
	PCFX_LastSample = 0;
	PCFX_SoundNum   = 0;
	PCFX_Priority   = 0;

	RestoreInterrupts(flags);
}


/*---------------------------------------------------------------------
   Function: PCFX_Service

   Task Manager routine to perform the playback of a sound effect.
---------------------------------------------------------------------*/

static void PCFX_Service(void)
{
	if (PCFX_Sound)
	{
		uint8_t value = *PCFX_Sound++;

		if (value != PCFX_LastSample)
		{
			PCFX_LastSample = value;
			if (value)
			{
				uint16_t valueTimes60 = value * 60;
				outp(0x43, 0xb6);
				outp(0x42, LOBYTE(valueTimes60));
				outp(0x42, HIBYTE(valueTimes60));
				outp(0x61, inp(0x61) | 0x3);
			} else
				outp(0x61, inp(0x61) & 0xfc);
		}

		if (--PCFX_LengthLeft == 0)
			PCFX_Stop(PCFX_SoundNum);
	}
}


/*---------------------------------------------------------------------
   Function: PCFX_Play

   Starts playback of a Muse sound effect.
---------------------------------------------------------------------*/

void PCFX_Play(Word SoundNum, Word priority)
{
	uint32_t flags;
	uint8_t __far* lump;

	if (priority < PCFX_Priority)
		return;

	PCFX_Stop(PCFX_SoundNum);

	PCFX_SoundNum   = SoundNum;
	PCFX_Priority   = priority;
	PCFX_LengthLeft = W_LumpLength(SoundNum + 150);

	lump = W_TryGetLumpByNum(SoundNum + 150);
	if (lump != NULL) {
		_fmemcpy(data, lump, PCFX_LengthLeft);
		Z_ChangeTagToCache(lump);
	} else {
		W_ReadLumpByNum(SoundNum + 150, data);
	}

	flags = DisableInterrupts();

	PCFX_Sound = &data[0];

	RestoreInterrupts(flags);
}


/*---------------------------------------------------------------------
   Function: PCFX_Init

   Initializes the sound effect engine.
---------------------------------------------------------------------*/

#define SND_TICRATE     140     // tic rate for updating sound

void PCFX_Init(void)
{
	if (PCFX_Installed)
		return;

	PCFX_Stop(PCFX_SoundNum);
	PCFX_ServiceTask = TS_ScheduleTask(PCFX_Service, SND_TICRATE, 2);
	TS_Dispatch();

	PCFX_Installed = TRUE;
}


/*---------------------------------------------------------------------
   Function: PCFX_Shutdown

   Ends the use of the sound effect engine.
---------------------------------------------------------------------*/

void PCFX_Shutdown(void)
{
	if (PCFX_Installed)
	{
		PCFX_Stop(PCFX_SoundNum);
		TS_Terminate(PCFX_ServiceTask);
		PCFX_Installed = FALSE;
	}
}
