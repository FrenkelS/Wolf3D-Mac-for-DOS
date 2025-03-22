/*-----------------------------------------------------------------------------
 *
 *
 *  Copyright (C) 2025 Frenkel Smeijers
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 *
 * DESCRIPTION:
 *     256 color Palette Manager
 *
 *-----------------------------------------------------------------------------*/

#include <conio.h>
#include <dos.h>
#include <string.h>

#include "wolfdef.h"

/**********************************

	Palette Manager

**********************************/

/**********************************

	Load and set a palette from a pointer

**********************************/

#define PEL_WRITE_ADR   0x3c8
#define PEL_DATA        0x3c9

static Byte CurrentPal[768];

static void SetAPalettePtr(unsigned char __far* PalPtr)
{
	int_fast16_t i;

	_fmemcpy(CurrentPal,PalPtr,768);

	outp(PEL_WRITE_ADR, 0);
	for (i = 0; i < 255 * 3; i++) // don't set the 256th color
		outp(PEL_DATA, (*PalPtr++) >> 2);
}

/**********************************

	Load and set a palette resource

**********************************/

void SetAPalette(Word PalNum)
{
	void __far* lump = W_GetLumpByNum(PalNum);
	SetAPalettePtr(lump);		/* Set the current palette */
	Z_ChangeTagToCache(lump);					/* Release the resource */
}

/**********************************

	Fade the palette

**********************************/

static void FadeToPtr(unsigned char __far* PalPtr)
{
	Word __far* DestPalette;				/* Dest offsets */
	Byte __far* WorkPalette;		/* Palette to draw */
	Byte __far* SrcPal;
	Word Count;
	Word i;

	if (!_fmemcmp(PalPtr,CurrentPal,768)) {	/* Same palette? */
		return;
	}

	DestPalette = Z_MallocStatic(768 * sizeof(Word));				/* Dest offsets */
	WorkPalette = Z_MallocStatic(768 * sizeof(Byte));		/* Palette to draw */
	SrcPal      = Z_MallocStatic(768 * sizeof(Byte));

	_fmemcpy(SrcPal,CurrentPal,768);
	i = 0;
	do {		/* Convert the source palette to ints */
		DestPalette[i] = PalPtr[i];			
	} while (++i<768);

	i = 0;
	do {
		DestPalette[i] -= SrcPal[i];	/* Convert to delta's */
	} while (++i<768);

	Count = 1;
	do {
		i = 0;
		do {
			WorkPalette[i] = ((DestPalette[i] * (int)(Count)) / 16) + SrcPal[i];
		} while (++i<768);
		SetAPalettePtr(WorkPalette);
		WaitTicks(1);
	} while (++Count<17);

	Z_Free(SrcPal);
	Z_Free(WorkPalette);
	Z_Free(DestPalette);
}

/**********************************

	Fade the screen to black

**********************************/

void FadeToBlack(void)
{
	unsigned char MyPal[768];

	memset(MyPal,0,sizeof(MyPal));	/* Fill with black */
	MyPal[0] = MyPal[1] = MyPal[2] = 255;
	FadeToPtr(MyPal);
}

/**********************************

	Fade the screen to a palette

**********************************/

void FadeTo(Word RezNum)
{
	void __far* lump = W_GetLumpByNum(RezNum);
	FadeToPtr(lump);
	Z_ChangeTagToCache(lump);
}


static const uint8_t colors[14][3] =
{
	// normal
	{7, 7, 7},

	// red
	{0x07, 0, 0},
	{0x0e, 0, 0},
	{0x15, 0, 0},
	{0x1c, 0, 0},
	{0x23, 0, 0},
	{0x2a, 0, 0},
	{0x31, 0, 0},
	{0x3b, 0, 0},

	// gold
	{0x06, 0x05, 0x02},
	{0x0d, 0x0b, 0x04},
	{0x14, 0x11, 0x06},
	{0x1a, 0x17, 0x08}
};


void IO_ColorScreen(Word bonus, Word damage)
{
	Word pal;

	if (bonus > damage) {
		pal = bonus + 9;
	} else if (damage) {
		pal = damage + 1;
	} else {
		pal = 0;
	}

	outp(PEL_WRITE_ADR, DAMAGECOLOR);
	outp(PEL_DATA, colors[pal][0]);
	outp(PEL_DATA, colors[pal][1]);
	outp(PEL_DATA, colors[pal][2]);
}
