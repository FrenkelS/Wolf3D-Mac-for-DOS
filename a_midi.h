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
   module: MIDI.H

   author: James R. Dose
   date:   May 25, 1994

   Public header for MIDI.C.  Midi song file playback routines.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __MIDI_H
#define __MIDI_H

enum MIDI_Errors
{
	MIDI_Error   = -1,
	MIDI_Ok      = 0,
	MIDI_NullMidiModule,
	MIDI_InvalidMidiFile,
	MIDI_UnknownMidiFormat,
	MIDI_NoTracks,
	MIDI_InvalidTrack,
	MIDI_NoMemory
};

typedef struct
{
	void (*NoteOff)(int32_t channel, int32_t key, int32_t velocity);
	void (*NoteOn)(int32_t channel, int32_t key, int32_t velocity);
	void (*PolyAftertouch)(int32_t channel, int32_t key, int32_t pressure);
	void (*ControlChange)(int32_t channel, int32_t number, int32_t value);
	void (*ProgramChange)(int32_t channel, int32_t program);
	void (*ChannelAftertouch)(int32_t channel, int32_t pressure);
	void (*PitchBend)(int32_t channel, int32_t lsb, int32_t msb);
	void (*SetVolume)(int32_t volume);
	int32_t (*GetVolume)(void);
} midifuncs;

void    MIDI_SetVolume(int32_t volume);
void    MIDI_SetMidiFuncs(midifuncs *funcs, int32_t soundDevice);
void    MIDI_StopSong(void);
int32_t MIDI_PlaySong(uint8_t __far* song);

#endif
