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
#ifndef __AL_MIDI_H
#define __AL_MIDI_H

void AL_Shutdown(void);
void AL_Init(void);
void AL_NoteOff(int32_t channel, int32_t key, int32_t velocity);
void AL_NoteOn(int32_t channel, int32_t key, int32_t velocity);
void AL_ControlChange(int32_t channel, int32_t type, int32_t data);
void AL_ProgramChange(int32_t channel, int32_t patch);
void AL_SetPitchBend(int32_t channel, int32_t lsb, int32_t msb);
Boolean AL_DetectFM(void);
void AL_RegisterTimbreBank(uint8_t __far* timbres);

#endif
