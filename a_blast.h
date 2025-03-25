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
   module: BLASTER.H

   author: James R. Dose
   date:   February 4, 1994

   Public header for BLASTER.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __BLASTER_H
#define __BLASTER_H

enum BLASTER_ERRORS
{
	BLASTER_Error = -1,
	BLASTER_Ok    = 0
};

#define STEREO      1
#define SIXTEEN_BIT 2

#define MONO_8BIT    0
#define STEREO_8BIT  ( STEREO )
#define STEREO_16BIT ( STEREO | SIXTEEN_BIT )

uint32_t BLASTER_GetPlaybackRate(void);
int32_t  BLASTER_GetDMAChannel(void);
int32_t  BLASTER_SetMixMode(int32_t mode);
void     BLASTER_StopPlayback(void);
int32_t  BLASTER_BeginBufferedPlayback(uint8_t *BufferStart, int32_t BufferSize, int32_t NumDivisions, uint32_t SampleRate, int32_t MixMode, void (*CallBackFunc)(void));
Boolean  BLASTER_Detect(void);
void     BLASTER_SetupWaveBlaster(void);
void     BLASTER_Init(void);
void     BLASTER_Shutdown(void);

#endif
