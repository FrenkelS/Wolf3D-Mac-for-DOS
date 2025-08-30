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
   module: MUSIC.H

   author: James R. Dose
   date:   March 25, 1994

   Public header for MUSIC.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __MUSIC_H
#define __MUSIC_H

enum MUSIC_ERRORS
{
	MUSIC_Warning = -2,
	MUSIC_Error   = -1,
	MUSIC_Ok      = 0
};

typedef struct
{
	uint32_t tickposition;
	uint32_t milliseconds;
	uint32_t measure;
	uint32_t beat;
	uint32_t tick;
} songposition;

int32_t MUSIC_Init(int32_t SoundCard);
void    MUSIC_Shutdown(void);
void    MUSIC_SetVolume(int32_t volume);
void    MUSIC_StopSong(void);
int32_t MUSIC_PlaySong(uint8_t __far* song);

#endif
