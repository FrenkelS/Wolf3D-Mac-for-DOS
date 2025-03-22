/*
Copyright (C) 1994-1995 Apogee Software, Ltd.
Copyright (C) 2023 Frenkel Smeijers

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
   file:   DMA.H

   author: James R. Dose
   date:   February 4, 1994

   Public header file for DMA.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __DMA_H
#define __DMA_H

enum DMA_ERRORS
{
	DMA_Error = -1,
	DMA_Ok    = 0
};

int32_t DMA_VerifyChannel(int32_t channel);
int32_t DMA_SetupTransfer(int32_t  channel, uint8_t *address, int32_t length);
int32_t DMA_EndTransfer(int32_t channel);
uint8_t *DMA_GetCurrentPos(int32_t channel);

#endif
