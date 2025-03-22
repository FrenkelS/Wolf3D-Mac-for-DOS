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
 *     Graphics subsystem
 *     MCGA / VGA Mode 13H 320x200 256 colors
 *
 *-----------------------------------------------------------------------------*/

#include <conio.h>
#include <dos.h>
#include <string.h>

#include "wolfdef.h"


#define PLANEWIDTH SCREENWIDTH


static Boolean isGraphicsModeSet = FALSE;

static unsigned char __far* VideoPointer;	/* Pointer to video memory */
static unsigned char __far* ViewPointer;
static uint8_t __far* videomemory;
static uint8_t __far* viewvideomemory;


static void I_SetScreenMode(uint16_t mode)
{
	union REGS regs;
	regs.w.ax = mode;
	int86(0x10, &regs, &regs);
}


void I_InitGraphics(void)
{
	I_SetScreenMode(0x13);
	isGraphicsModeSet = TRUE;

	VideoPointer = Z_MallocStatic(1u * SCREENWIDTH * SCREENHEIGHT);
	ViewPointer = VideoPointer;

	videomemory = D_MK_FP(0xa000, 0 + __djgpp_conventional_base);
	viewvideomemory = videomemory;

	ClearTheScreen();			/* Force the offscreen memory blank */
	BlastScreen();
}


void I_ShutdownGraphics(void)
{
	if (isGraphicsModeSet)
		I_SetScreenMode(3);
}


/**********************************

	Draw a shape

**********************************/

static void DrawShape(Word x, Word y, void __far* ShapePtr)
{
	unsigned char __far* ScreenPtr;
	unsigned char __far* Screenad;
	unsigned char __far* ShapePtr2;
	unsigned short __far* ShapePtr3;
	Word Width;
	Word Height;
	Word Width2;

	ShapePtr3 = ShapePtr;
	Width     = ShapePtr3[0];		/* 16 bit width */
	Height    = ShapePtr3[1];		/* 16 bit height */
	ShapePtr2 = (unsigned char __far*) &ShapePtr3[2];
	ScreenPtr = &VideoPointer[y * PLANEWIDTH + x];
	do {
		Width2 = Width;
		Screenad = ScreenPtr;
		do {
			*Screenad++ = *ShapePtr2++;
		} while (--Width2);
		ScreenPtr += PLANEWIDTH;
	} while (--Height);
}


void DrawShapeNum(Word x, Word y, Word RezNum)
{
	void __far* lump = W_GetLumpByNum(RezNum);
	DrawShape(x, y, lump);
	Z_ChangeTagToCache(lump);
}


void DrawRawFullScreen(Word RezNum)
{
	void __far* lump = W_TryGetLumpByNum(RezNum);

	if (lump != NULL) {
		_fmemcpy(VideoPointer, lump, 1u * SCREENWIDTH * SCREENHEIGHT);
		Z_ChangeTagToCache(lump);
	} else {
		W_ReadLumpByNum(RezNum, VideoPointer);
	}
}


/**********************************

	Draw a masked shape

**********************************/

static void DrawMShape(Word x,Word y,void __far* ShapePtr)
{
	unsigned char __far* ScreenPtr;
	unsigned char __far* Screenad;
	unsigned char __far* MaskPtr;
	unsigned char __far* ShapePtr2;
	Word Width;
	Word Height;
	Word Width2;

	ShapePtr2 = ShapePtr;
	Width  = ShapePtr2[0];
	Height = ShapePtr2[2];
	ShapePtr2 +=4;
	MaskPtr = &ShapePtr2[Width*Height];
	ScreenPtr = &ViewPointer[y * PLANEWIDTH + x];
	do {
		Width2 = Width;
		Screenad = ScreenPtr;
		do {
			if (!*MaskPtr++) {
				*Screenad = *ShapePtr2;
			}
			++ShapePtr2;
			++Screenad;
		} while (--Width2);
		ScreenPtr += PLANEWIDTH;
	} while (--Height);
}


static void DrawMShapeHalf(Word x,Word y,void __far* ShapePtr)
{
	unsigned char __far* ScreenPtr;
	unsigned char __far* Screenad;
	unsigned char __far* MaskPtr;
	unsigned char __far* ShapePtr2;
	Word Width;
	Word Height;
	Word Width2;
	Word WidthOdd;

	ShapePtr2 = ShapePtr;
	Width  = ShapePtr2[0];
	Height = ShapePtr2[2];
	ShapePtr2 +=4;
	MaskPtr = &ShapePtr2[Width*Height];
	ScreenPtr = &ViewPointer[y * PLANEWIDTH + x];
	WidthOdd = Width & 1;
	Height = (Height / 2) + (Height & 1);
	do {
		Width2 = Width / 2;
		Screenad = ScreenPtr;
		do {
			if (!*MaskPtr++) {
				*Screenad = *ShapePtr2;
			}
			++ShapePtr2;
			++MaskPtr;
			++ShapePtr2;
			++Screenad;
		} while (--Width2);
		MaskPtr   += Width + WidthOdd;
		ShapePtr2 += Width + WidthOdd;
		ScreenPtr += PLANEWIDTH;
	} while (--Height);
}


/**********************************

	Draw a masked shape with an offset

**********************************/

void DrawXMShapeNum(Word x,Word y,Word RezNum)
{
	unsigned short __far* ShapePtr = W_GetLumpByNum(RezNum);

	if (viewheight > 7 * 16) {
		x += ShapePtr[0];
		y += ShapePtr[1];
		DrawMShape(x,y,&ShapePtr[2]);
	} else {
		x += ShapePtr[0] / 2 + 16;
		y += ShapePtr[1] / 2 + 32;
		DrawMShapeHalf(x,y,&ShapePtr[2]);
	}

	Z_ChangeTagToCache(ShapePtr);
}

/**********************************

	Clear the screen

**********************************/

void ClearTheScreen(void)
{
	_fmemset(VideoPointer, BLACK, 1u * SCREENWIDTH * SCREENHEIGHT);
}

/**********************************

	Erase the floor and ceiling
	
**********************************/

void IO_ClearViewBuffer(void)
{
	Word y;

	for (y = 0; y < viewheight / 2; y++)
		_fmemset(&ViewPointer[y * PLANEWIDTH], 0x2f, scaledviewwidth);

	for (y = viewheight / 2; y < viewheight; y++)
		_fmemset(&ViewPointer[y * PLANEWIDTH], 0x2A, scaledviewwidth);
}


void I_ClearView(void)
{
	_fmemset(videomemory, DAMAGECOLOR, 1u * SCREENWIDTH * MAXVIEWHEIGHT);
}


void I_SetViewSize(Word blocks)
{
	scaledviewwidth = blocks * 32;
	viewwidth  = scaledviewwidth >> detailshift;
	viewheight = blocks * 16;
	ViewPointer = &VideoPointer[((MAXVIEWHEIGHT - viewheight) / 2) * PLANEWIDTH + (SCREENWIDTH - scaledviewwidth) / 2];
	viewvideomemory = &videomemory[((MAXVIEWHEIGHT - viewheight) / 2) * PLANEWIDTH + (SCREENWIDTH - scaledviewwidth) / 2];
	StartupRendering();

	I_ClearView();
}


/**********************************

	Update the wolf screen as fast as possible

**********************************/

void BlastScreen2(Rect *BlastRect) 
{
	UNUSED(BlastRect);
	BlastScreen();
}

void BlastScreen(void)
{
	_fmemcpy(videomemory, VideoPointer, 1u * SCREENWIDTH * SCREENHEIGHT);
}


void BlastView(void)
{
	Word y;

	for (y = 0; y < viewheight; y++)
		_fmemcpy(&viewvideomemory[y * PLANEWIDTH], &ViewPointer[y * PLANEWIDTH], scaledviewwidth);
}


#define ST_HEIGHT 40

void BlastStatusBar(void)
{
	_fmemcpy(&videomemory[1u * MAXVIEWHEIGHT * PLANEWIDTH], &VideoPointer[1u * MAXVIEWHEIGHT * PLANEWIDTH], SCREENWIDTH * ST_HEIGHT);
}


/**********************************

	Handle GET PSYCHED!

**********************************/

#define PSYCHEDWIDE 184
#define PSYCHEDHIGH 5
#define PSYCHEDX 20
#define PSYCHEDY 46
#define MAXINDEX (66+S_LASTONE)

/**********************************

	Draw the initial shape
		
**********************************/

void ShowGetPsyched(void)
{
	Word X,Y;

	ClearTheScreen();
	BlastScreen();
	X = (SCREENWIDTH-224)/2;
	Y = (SCREENHEIGHT-56)/2;
	DrawShapeNum(X,Y,rGetPsychPic);
	BlastScreen();
	SetAPalette(rGamePal);
}

/**********************************

	Update the thermomitor
	
**********************************/

void DrawPsyched(Word Index)
{
	Word Factor;
	Word X,Y;
	Word h;

	Factor = Index * PSYCHEDWIDE;		/* Calc the relative X pixel factor */
	Factor = Factor / MAXINDEX;
	X = ((SCREENWIDTH-224)/2) + PSYCHEDX;
	Y = ((SCREENHEIGHT-56)/2) + PSYCHEDY;
	for (h = 0; h < PSYCHEDHIGH; h++)
		_fmemset(&VideoPointer[(Y + h) * PLANEWIDTH + X], 64, Factor);
	BlastScreen();
}

/**********************************

	Erase the Get Psyched screen
	
**********************************/

void EndGetPsyched(void)
{
	SetAPalette(rBlackPal);		/* Zap the palette */
	I_ClearView();
}


static LongWord ScaleDiv[2048];			/* Divide table for scalers */

/**********************************

	Create the compiled scalers

**********************************/

void SetupScalers(void)
{
	Word i;
	
	if (!ScaleDiv[1]) {		/* Divide table inited already? */
		i = 1;
		do {
			ScaleDiv[i] = 0x40000000/i;		/* Get the recipocal for the scaler */
		} while (++i<2048);
	}
}

/**********************************

	Draw a vertical line with a scaler
	(Used for wall and sprite drawing)
	
**********************************/

static void ScaleGlueFlat(Byte Art,Word MaxLines,Byte __far* ScreenPtr)
{
	uint16_t src16;
	uint32_t src32;

	switch (detailshift) {
	case 0:
		while (MaxLines--) {
			*ScreenPtr = Art;
			ScreenPtr += PLANEWIDTH;
		}
		break;
	case 1:
		src16 = Art;
		src16 |= src16 << 8;
		while (MaxLines--) {
			*(uint16_t __far*)ScreenPtr = src16;
			ScreenPtr += PLANEWIDTH;
		}
		break;
	case 2:
		src32 = Art;
		src32 |= src32 << 8;
		src32 |= src32 << 16;
		while (MaxLines--) {
			*(uint32_t __far*)ScreenPtr = src32;
			ScreenPtr += PLANEWIDTH;
		}
		break;
	}
}


static void ScaleGlue(Byte __far* ArtPtr,Word MaxLines,Byte __far* ScreenPtr,LongWord Frac,Word Integer,LongWord Delta)
{
	switch (detailshift) {
	case 0:
		while (MaxLines--) {
			*ScreenPtr = *ArtPtr;
			ScreenPtr += PLANEWIDTH;
			Delta += Frac;
			ArtPtr += Integer + (Delta < Frac);
		}
		break;
	case 1:
		while (MaxLines--) {
			uint16_t src = *ArtPtr;
			src |= src << 8;
			*(uint16_t __far*)ScreenPtr = src;
			ScreenPtr += PLANEWIDTH;
			Delta += Frac;
			ArtPtr += Integer + (Delta < Frac);
		}
		break;
	case 2:
		while (MaxLines--) {
			uint32_t src = *ArtPtr;
			src |= src << 8;
			src |= src << 16;
			*(uint32_t __far*)ScreenPtr = src;
			ScreenPtr += PLANEWIDTH;
			Delta += Frac;
			ArtPtr += Integer + (Delta < Frac);
		}
		break;
	}
}


void IO_ScaleWallColumn(Word x,Word scale,Word tile,Word column)
{
	LongWord TheFrac;
	Word y;
	Byte __far* ArtStart;
	Byte __far* lump;

	if (!scale) {		/* Uhh.. Don't bother */
		return;
	}

	TheFrac = ScaleDiv[scale];
	scale*=2;

	lump = W_TryGetLumpByNum(ArtData[tile]);
	if (!lump) {
		if (scale<viewheight) {
			y = (viewheight-scale)/2;
			ScaleGlueFlat(tile, scale,      &ViewPointer[(y * PLANEWIDTH) + (x << detailshift)]);
		} else {
			ScaleGlueFlat(tile, viewheight, &ViewPointer[x << detailshift]);
		}
	} else {
		ArtStart = &lump[(column&127)<<7];
		if (scale<viewheight) {
			y = (viewheight-scale)/2;
			ScaleGlue(ArtStart,scale,
				&ViewPointer[(y * PLANEWIDTH) + (x << detailshift)],
				TheFrac << 8,	/* Fractional value */
				TheFrac >> 24,	/* Integer value */
				0
			);
		} else {
			LongWord ly;
			y = (scale-viewheight)/2;		/* How manu lines to remove */
			ly = y*TheFrac;
			ScaleGlue(&ArtStart[ly>>24],viewheight,
				&ViewPointer[x << detailshift],
				TheFrac << 8,	/* Fractional value */
				TheFrac >> 24,	/* Integer value */
				ly<<8
			);
		}
		Z_ChangeTagToCache(lump);
	}
}

/**********************************

	Draw a vertical line with a masked scaler
	(Used for SPRITE drawing)
	
**********************************/

typedef struct {
	unsigned short Topy;
	unsigned short Boty;
	unsigned short Shape;
} SpriteRun;

void IO_ScaleMaskedColumn(Word x,Word scale,Word lumpNum,Word column)
{
	unsigned short __far* CharPtr;
	Byte __far* CharPtr2;
	int Y1,Y2;
	Byte __far* Screenad;
	SpriteRun __far* RunPtr;
	LongWord TheFrac;
	LongWord TFrac;
	Word TInt;
	Word RunCount;
	int TopY;
	Word Index;
	LongWord Delta;

	if (!scale) {
		return;
	}

	CharPtr = W_TryGetLumpByNum(lumpNum);
	if (!CharPtr) {
		if (scale<viewheight) {
			Word y = (viewheight-scale)/2;
			ScaleGlueFlat(lumpNum, scale,      &ViewPointer[(y * PLANEWIDTH) + (x << detailshift)]);
		} else {
			ScaleGlueFlat(lumpNum, viewheight, &ViewPointer[x << detailshift]);
		}
	} else {
		CharPtr2 = (Byte __far*) CharPtr;
		TheFrac = ScaleDiv[scale];		/* Get the scale fraction */ 
		RunPtr = (SpriteRun __far*) &CharPtr[CharPtr[column+1]/2];	/* Get the offset to the RunPtr data */
		Screenad = &ViewPointer[x << detailshift];		/* Set the base screen address */
		TFrac = TheFrac<<8;
		TInt = TheFrac>>24;
		TopY = (viewheight/2)-scale;		/* Number of pixels for 128 pixel shape */

		while (RunPtr->Topy != (unsigned short) -1) {		/* Not end of record? */
			Y1 = scale*(LongWord)RunPtr->Topy/128+TopY;
			if (Y1<(int)viewheight) {		/* Clip top? */
				Y2 = scale*(LongWord)RunPtr->Boty/128+TopY;
				if (Y2>0) {
					if (Y2>(int)viewheight) {
						Y2 = viewheight;
					}
					Index = RunPtr->Shape+(RunPtr->Topy/2);
					Delta = 0;
					if (Y1<0) {
						Delta = (0-(Word)Y1)*TheFrac;
						Index += (Delta>>24);
						Delta <<= 8;
						Y1 = 0;
					}
					RunCount = Y2-Y1;
					if (RunCount) {
						ScaleGlue(
						&CharPtr2[Index],	/* Pointer to art data */
						RunCount,			/* Number of lines to draw */
						&Screenad[Y1*PLANEWIDTH],			/* Pointer to screen */
						TFrac,				/* Fractional value */ 
						TInt,				/* Integer value */
						Delta					/* Delta value */
						);
					}
				}
			} 
			RunPtr++;						/* Next record */
		}	
		Z_ChangeTagToCache(CharPtr);
	}
}

/**********************************

	Draw an automap tile
	
**********************************/

static Byte __far* SmallFontPtr;

void DrawSmall(Word x, Word y, Word tile)
{
	Byte __far* Screenad;
	Byte __far* ArtStart;
	Word Width,Height;

	if (!SmallFontPtr) {
		return;
	}	
	x*=16;
	y*=16;
	Screenad = &VideoPointer[y * PLANEWIDTH + x];
	ArtStart = &SmallFontPtr[tile*(16*16)];
	Height = 0;
	do {
		Width = 16;
		do {
			Screenad[0] = ArtStart[0];
			++Screenad;
			++ArtStart;
		} while (--Width);
		Screenad+=PLANEWIDTH-16;
	} while (++Height<16);
}


void MakeSmallFont(void)
{
	Word i,j,Width,Height;
	Byte __far* DestPtr;
	Byte __far* ArtStart;
	Byte __far *TempPtr;
	
	SmallFontPtr = Z_TryMallocStatic(16*16*65);
	if (!SmallFontPtr) {
		return;
	}
	_fmemset(SmallFontPtr,0,16*16*65);		/* Erase the font */
	i = 0;
	DestPtr = SmallFontPtr;
	do {
		if (!ArtData[i]) {
			DestPtr+=(16*16);
		} else {
			ArtStart = W_GetLumpByNum(ArtData[i]);

			Height = 0;
			do {
				Width = 16;
				j = Height*8;
				do {
					DestPtr[0] = ArtStart[j];
					++DestPtr;
					j+=(WALLHEIGHT*8);
				} while (--Width);
			} while (++Height<16);

			Z_ChangeTagToCache(ArtStart);
		}
	} while (++i<64);
	TempPtr = W_GetLumpByNum(MyBJFace);
	_fmemcpy(DestPtr,TempPtr,16*16);
	Z_ChangeTagToCache(TempPtr);
}


void KillSmallFont(void)
{
	if (SmallFontPtr) {
		Z_Free(SmallFontPtr);
		SmallFontPtr = NULL;
	}
}
