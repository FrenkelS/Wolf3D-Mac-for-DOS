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
   module: MUSIC.C

   author: James R. Dose
   date:   March 25, 1994

   Device independant music playback routines.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include "i_system.h"
#include "i_audio.h"
#include "a_al_mid.h"
#include "a_blast.h"
#include "a_midi.h"
#include "a_mpu401.h"
#include "a_music.h"
#include "a_taskmn.h"

static int32_t MUSIC_SoundDevice = AHW_NONE;

static midifuncs MUSIC_MidiFunctions;

static void MUSIC_InitFM(void);
static void MUSIC_InitMidi(void);


/*---------------------------------------------------------------------
   Function: MUSIC_Init

   Selects which sound device to use.
---------------------------------------------------------------------*/

int32_t MUSIC_Init(int32_t SoundCard)
{
	MUSIC_SoundDevice = SoundCard;

	switch (SoundCard)
	{
		case AHW_ADLIB:
			MUSIC_InitFM();
			break;

		case AHW_MPU_401:
			MUSIC_InitMidi();
			break;

		default:
			return MUSIC_Error;
	}

	return MUSIC_Ok;
}


/*---------------------------------------------------------------------
   Function: MUSIC_Shutdown

   Terminates use of sound device.
---------------------------------------------------------------------*/

void MUSIC_Shutdown(void)
{
	MIDI_StopSong();

	switch (MUSIC_SoundDevice)
	{
		case AHW_ADLIB :
			AL_Shutdown();
			break;

		case AHW_MPU_401:
			MPU_Reset();
			break;
	}
}


/*---------------------------------------------------------------------
   Function: MUSIC_SetVolume

   Sets the volume of music playback.
---------------------------------------------------------------------*/

void MUSIC_SetVolume(int32_t volume)
{
	if (MUSIC_SoundDevice == AHW_NONE)
		return;

	if (volume < 0)
		volume = 0;
	else if (volume > 255)
		volume = 255;

	MIDI_SetVolume(volume);
}


/*---------------------------------------------------------------------
   Function: MUSIC_StopSong

   Stops playback of current song.
---------------------------------------------------------------------*/

void MUSIC_StopSong(void)
{
	MIDI_StopSong();
}


/*---------------------------------------------------------------------
   Function: MUSIC_PlaySong

   Begins playback of MIDI song.
---------------------------------------------------------------------*/

int32_t MUSIC_PlaySong(uint8_t *song)
{
	int32_t status;

	switch (MUSIC_SoundDevice)
	{
		case AHW_ADLIB:
		case AHW_MPU_401:
			MIDI_StopSong();
			status = MIDI_PlaySong(song);
			if (status != MIDI_Ok)
				return MUSIC_Warning;
			break;

		default:
			return MUSIC_Warning;
	}

	return MUSIC_Ok;
}


static void MUSIC_InitFM(void)
{
	// Init the fm routines
	AL_Init();

	MUSIC_MidiFunctions.NoteOff           = AL_NoteOff;
	MUSIC_MidiFunctions.NoteOn            = AL_NoteOn;
	MUSIC_MidiFunctions.PolyAftertouch    = NULL;
	MUSIC_MidiFunctions.ControlChange     = AL_ControlChange;
	MUSIC_MidiFunctions.ProgramChange     = AL_ProgramChange;
	MUSIC_MidiFunctions.ChannelAftertouch = NULL;
	MUSIC_MidiFunctions.PitchBend         = AL_SetPitchBend;
	MUSIC_MidiFunctions.SetVolume         = NULL;
	MUSIC_MidiFunctions.GetVolume         = NULL;

	MIDI_SetMidiFuncs(&MUSIC_MidiFunctions, MUSIC_SoundDevice);
}


static void MUSIC_InitMidi(void)
{
	BLASTER_SetupWaveBlaster();

	MUSIC_MidiFunctions.NoteOff           = MPU_NoteOff;
	MUSIC_MidiFunctions.NoteOn            = MPU_NoteOn;
	MUSIC_MidiFunctions.PolyAftertouch    = MPU_PolyAftertouch;
	MUSIC_MidiFunctions.ControlChange     = MPU_ControlChange;
	MUSIC_MidiFunctions.ProgramChange     = MPU_ProgramChange;
	MUSIC_MidiFunctions.ChannelAftertouch = MPU_ChannelAftertouch;
	MUSIC_MidiFunctions.PitchBend         = MPU_PitchBend;
	MUSIC_MidiFunctions.SetVolume         = NULL;
	MUSIC_MidiFunctions.GetVolume         = NULL;

	MIDI_SetMidiFuncs(&MUSIC_MidiFunctions, MUSIC_SoundDevice);
}
