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
 *     VGA Mode Y 320x200 256 colors
 *
 *-----------------------------------------------------------------------------*/

#include <conio.h>
#include <dos.h>
#include <string.h>

#include "wolfdef.h"


#define PLANEWIDTH 80


#define PAGE_SIZE 0x0400

#define PAGE0		0xa000
#define PAGE1		(PAGE0+PAGE_SIZE)
#define PAGE2		(PAGE1+PAGE_SIZE)
#define PAGE3		(PAGE2+PAGE_SIZE)


#define SC_INDEX                0x3c4
#define SC_MAPMASK              2
#define SC_MEMMODE              4

#define CRTC_INDEX              0x3d4
#define CRTC_STARTHIGH          12
#define CRTC_UNDERLINE          20
#define CRTC_MODE               23

#define GC_INDEX                0x3ce
#define GC_MODE                 5
#define GC_MISCELLANEOUS        6


static Boolean isGraphicsModeSet = FALSE;

static unsigned char __far* VideoPointer;	/* Pointer to video memory */
static unsigned char __far* ViewPointer;
static Word ViewPointerOffset;


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

	VideoPointer = D_MK_FP(PAGE1, 0 + __djgpp_conventional_base);
	ViewPointerOffset = ((MAXVIEWHEIGHT - viewheight) / 2) * PLANEWIDTH + ((SCREENWIDTH - scaledviewwidth) / 4) / 2;
	ViewPointer = VideoPointer + ViewPointerOffset;

	outp(SC_INDEX, SC_MEMMODE);
	outp(SC_INDEX + 1, (inp(SC_INDEX + 1) & ~8) | 4);

	outp(GC_INDEX, GC_MODE);
	outp(GC_INDEX + 1, inp(GC_INDEX + 1) & ~0x13);

	outp(GC_INDEX, GC_MISCELLANEOUS);
	outp(GC_INDEX + 1, inp(GC_INDEX + 1) & ~2);

	outp(SC_INDEX, SC_MAPMASK);
	outp(SC_INDEX + 1, 15);

	/* Force the offscreen memory blank */
	_fmemset(D_MK_FP(PAGE0, 0 + __djgpp_conventional_base), BLACK, 0xffff);

	outp(CRTC_INDEX, CRTC_UNDERLINE);
	outp(CRTC_INDEX + 1, inp(CRTC_INDEX + 1) & ~0x40);

	outp(CRTC_INDEX, CRTC_MODE);
	outp(CRTC_INDEX + 1, inp(CRTC_INDEX + 1) | 0x40);
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
	volatile unsigned char __far* Screenad;
	unsigned char __far* ShapePtr2;
	unsigned char __far* ShapePtr2ad;
	unsigned short __far* ShapePtr3;
	Word Width;
	Word Height;
	Word Plane;
	unsigned char __far* vp;
	Word w;
	Word h;

	ShapePtr3 = ShapePtr;
	Width     = ShapePtr3[0];		/* 16 bit width */
	Height    = ShapePtr3[1];		/* 16 bit height */
	ShapePtr2 = (unsigned char __far*) &ShapePtr3[2];
	vp = D_MK_FP(PAGE0, 0 + __djgpp_conventional_base);
	ScreenPtr = &vp[(y * PLANEWIDTH) + (x >> 2)];

	Plane = x & 3;
	for (w = Width; w != 0; w--) {
		outp(SC_INDEX + 1, 1 << Plane);

		ShapePtr2ad = ShapePtr2;
		Screenad    = ScreenPtr;
		for (h = Height; h != 0; h--) {
			*(Screenad + 0u * PAGE_SIZE * 16) = *ShapePtr2ad;
			*(Screenad + 1u * PAGE_SIZE * 16) = *ShapePtr2ad;
			*(Screenad + 2u * PAGE_SIZE * 16) = *ShapePtr2ad;
			ShapePtr2ad += Width;
			Screenad    += PLANEWIDTH;
		}

		ShapePtr2++;
		if (++Plane == 4) {
			Plane = 0;
			ScreenPtr++;
		}
	}
}


void DrawShapeNum(Word x, Word y, Word RezNum)
{
	void __far* lump = W_GetLumpByNum(RezNum);
	DrawShape(x, y, lump);
	Z_ChangeTagToCache(lump);
}


void DrawRawFullScreen(Word RezNum)
{
	const uint8_t __far* lump = W_TryGetLumpByNum(RezNum);

	if (lump != NULL) {
		Word plane;
		for (plane = 0; plane < 4; plane++) {
			Word x;
			Word y;
			uint8_t __far* dest = VideoPointer;

			outp(SC_INDEX + 1, 1 << plane);
			for (y = 0; y < SCREENHEIGHT; y++) {
				for (x = 0; x < SCREENWIDTH / 4; x++) {
					*dest++ = lump[y * SCREENWIDTH + (x * 4) + plane];
				}
			}
		}
		Z_ChangeTagToCache(lump);
	}
}


/**********************************

	Draw a masked shape

**********************************/

static void DrawMShape(Word x,Word y,void __far* ShapePtr)
{
	unsigned char __far* ScreenPtr;
	volatile unsigned char __far* Screenad;
	unsigned char __far* MaskPtr;
	unsigned char __far* MaskPtrad;
	unsigned char __far* ShapePtr2;
	unsigned char __far* ShapePtr2ad;
	Word Width;
	Word Height;
	Word Plane;
	Word w;
	Word h;

	ShapePtr2  = ShapePtr;
	Width      = ShapePtr2[0];
	Height     = ShapePtr2[2];
	ShapePtr2 += 4;
	MaskPtr    = &ShapePtr2[Width * Height];
	ScreenPtr  = &ViewPointer[(y * PLANEWIDTH) + (x >> 2)];

	Plane = x & 3;
	for (w = Width; w != 0; w--) {
		outp(SC_INDEX + 1, 1 << Plane);

		MaskPtrad   = MaskPtr;
		ShapePtr2ad = ShapePtr2;
		Screenad    = ScreenPtr;
		for (h = Height; h != 0; h--) {
			if (!*MaskPtrad) {
				*Screenad = *ShapePtr2ad;
			}
			MaskPtrad   += Width;
			ShapePtr2ad += Width;
			Screenad    += PLANEWIDTH;
		}

		MaskPtr++;
		ShapePtr2++;
		if (++Plane == 4) {
			Plane = 0;
			ScreenPtr++;
		}
	}
}


static void DrawMShapeHalf(Word x,Word y,void __far* ShapePtr)
{
	unsigned char __far* ScreenPtr;
	volatile unsigned char __far* Screenad;
	unsigned char __far* MaskPtr;
	unsigned char __far* MaskPtrad;
	unsigned char __far* ShapePtr2;
	unsigned char __far* ShapePtr2ad;
	Word Width;
	Word Height;
	Word Plane;
	Word w;
	Word h;

	ShapePtr2  = ShapePtr;
	Width      = ShapePtr2[0];
	Height     = ShapePtr2[2];
	ShapePtr2 += 4;
	MaskPtr    = &ShapePtr2[Width * Height];
	ScreenPtr  = &ViewPointer[(y * PLANEWIDTH) + (x >> 2)];
	Height     = (Height / 2) + (Height & 1);

	Plane = x & 3;
	for (w = (Width / 2) + (Width & 1); w != 0; w--) {
		outp(SC_INDEX + 1, 1 << Plane);

		MaskPtrad   = MaskPtr;
		ShapePtr2ad = ShapePtr2;
		Screenad    = ScreenPtr;
		for (h = Height; h != 0; h--) {
			if (!*MaskPtrad) {
				*Screenad = *ShapePtr2ad;
			}
			MaskPtrad   += Width * 2;
			ShapePtr2ad += Width * 2;
			Screenad    += PLANEWIDTH;
		}

		MaskPtr   += 2;
		ShapePtr2 += 2;
		if (++Plane == 4) {
			Plane = 0;
			ScreenPtr++;
		}
	}
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
	outp(SC_INDEX + 1, 15);
	_fmemset(VideoPointer, BLACK, PLANEWIDTH * SCREENHEIGHT);
}

/**********************************

	Erase the floor and ceiling
	
**********************************/

void IO_ClearViewBuffer(void)
{
	Word y;

	outp(SC_INDEX + 1, 15);

	for (y = 0; y < viewheight / 2; y++)
		_fmemset(&ViewPointer[y * PLANEWIDTH], 0x2f, scaledviewwidth / 4);

	for (y = viewheight / 2; y < viewheight; y++)
		_fmemset(&ViewPointer[y * PLANEWIDTH], 0x2A, scaledviewwidth / 4);
}


void I_ClearView(void)
{
	outp(SC_INDEX + 1, 15);
	_fmemset(D_MK_FP(PAGE0, 0 + __djgpp_conventional_base), DAMAGECOLOR, PLANEWIDTH * MAXVIEWHEIGHT);
	_fmemset(D_MK_FP(PAGE1, 0 + __djgpp_conventional_base), DAMAGECOLOR, PLANEWIDTH * MAXVIEWHEIGHT);
	_fmemset(D_MK_FP(PAGE2, 0 + __djgpp_conventional_base), DAMAGECOLOR, PLANEWIDTH * MAXVIEWHEIGHT);
}


void I_SetViewSize(Word blocks)
{
	scaledviewwidth = blocks * 32;
	viewwidth  = scaledviewwidth >> detailshift;
	viewheight = blocks * 16;
	ViewPointerOffset = ((MAXVIEWHEIGHT - viewheight) / 2) * PLANEWIDTH + ((SCREENWIDTH - scaledviewwidth) / 4) / 2;
	ViewPointer = VideoPointer + ViewPointerOffset;
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
	// page flip between segments A000, A400 and A800
	outp(CRTC_INDEX, CRTC_STARTHIGH);
#if defined _M_I86
	outp(CRTC_INDEX + 1, D_FP_SEG(VideoPointer) >> 4);
	VideoPointer = D_MK_FP(D_FP_SEG(VideoPointer) + PAGE_SIZE, D_FP_OFF(VideoPointer));
	if (D_FP_SEG(VideoPointer) == PAGE3)
		VideoPointer = D_MK_FP(PAGE0, D_FP_OFF(VideoPointer));
#else
	outp(CRTC_INDEX + 1, (D_FP_SEG(VideoPointer) >> 4) & 0xf0);
	VideoPointer += (PAGE_SIZE << 4);
	if ((((uint32_t)VideoPointer) & (PAGE3 << 4)) == (PAGE3 << 4))
		VideoPointer -= (0x10000 - (PAGE_SIZE << 4));
#endif

	ViewPointer = VideoPointer + ViewPointerOffset;
}


void BlastView(void)
{
	BlastScreen();
}


void BlastStatusBar(void)
{
	// Do nothing
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

	//Clear the screen
	outp(SC_INDEX + 1, 15);
	_fmemset(D_MK_FP(PAGE0, 0 + __djgpp_conventional_base), BLACK, PLANEWIDTH * SCREENHEIGHT);
	_fmemset(D_MK_FP(PAGE1, 0 + __djgpp_conventional_base), BLACK, PLANEWIDTH * SCREENHEIGHT);
	_fmemset(D_MK_FP(PAGE2, 0 + __djgpp_conventional_base), BLACK, PLANEWIDTH * SCREENHEIGHT);

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
	Word X, Y;
	Word w, h;

	Factor = Index * PSYCHEDWIDE;		/* Calc the relative X pixel factor */
	Factor = Factor / MAXINDEX;
	X = ((SCREENWIDTH - 224) / 2) + PSYCHEDX;
	Y = ((SCREENHEIGHT - 56) / 2) + PSYCHEDY;
	for (w = 0; w < Factor; w++) {
		outp(SC_INDEX + 1, 1 << ((X + w) & 3));
		for (h = 0; h < PSYCHEDHIGH; h++) {
			VideoPointer[((Y + h) * PLANEWIDTH) + ((X + w) >> 2)] = 64;
		}
	}
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

Byte __far* source;
Byte __far* dest;


static void ScaleGlueFlat(Byte Art,Word count)
{
	Byte __far* ScreenPtr = dest;
	while (count--) {
		*ScreenPtr = Art;
		ScreenPtr += PLANEWIDTH;
	}
}


#if defined _M_I86
#define COLBITS 8
#else
#define COLBITS 16
#endif


#if !defined _M_I86 || defined C_ONLY
static void ScaleGlue(Word fracstep,Word frac,Word count)
{
	Byte __far* ArtPtr = source;
	Byte __far* ScreenPtr = dest;

#if 1
	Word l = count >> 4;
	while (l--) {
		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;

		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;

		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;

		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		*ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
	}

	switch (count & 15) {
		case 15: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case 14: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case 13: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case 12: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case 11: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case 10: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case  9: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case  8: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case  7: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case  6: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case  5: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case  4: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case  3: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case  2: *ScreenPtr = ArtPtr[frac >> COLBITS]; ScreenPtr += PLANEWIDTH; frac += fracstep;
		case  1: *ScreenPtr = ArtPtr[frac >> COLBITS];
	}
#else
	while (count--) {
		*ScreenPtr = ArtPtr[frac >> COLBITS];
		ScreenPtr += PLANEWIDTH;
		frac += fracstep;
	}
#endif
}
#else
void ScaleGlue(Word fracstep,Word frac,Word count);
#endif


void IO_ScaleWallColumn(Word x,Word scale,Word tile,Word column)
{
	LongWord TheFrac;
	Byte __far* lump;

	if (!scale) {		/* Uhh.. Don't bother */
		return;
	}

	if (detailshift == 0) {
		outp(SC_INDEX + 1, 1 << (x & 3));
	} else if (detailshift == 1) {
		outp(SC_INDEX + 1, 3 << ((x & 1) * 2));
	} else {
		// Do nothing, IO_ClearViewBuffer has already set the map mask
	}

	TheFrac = ScaleDiv[scale];
	scale *= 2;

	if (scale < viewheight) {
		Word y = (viewheight - scale) / 2;
		dest = &ViewPointer[(y * PLANEWIDTH) + (x >> (2 - detailshift))];
	} else {
		dest = &ViewPointer[x >> (2 - detailshift)];
	}

	lump = W_TryGetLumpByNum(ArtData[tile]);
	if (!lump) {
		if (scale > viewheight) {
			scale = viewheight;
		}
		ScaleGlueFlat(tile, scale);
	} else {
		if (scale <= viewheight) {
			source = &lump[(column & 127) << 7];
#if defined _M_I86
			ScaleGlue(
				TheFrac >> 16,	/* Integer value + Fractional value */
				0,
				scale
			);
#else
			ScaleGlue(
				TheFrac >> 8,	/* Integer value + Fractional value */
				0,
				scale
			);
#endif
		} else {
			Word y = (scale - viewheight) / 2;		/* How manu lines to remove */
			LongWord ly = y * TheFrac;
			source = &lump[((column & 127) << 7) + (Word)(ly >> 24)];
#if defined _M_I86
			ScaleGlue(
				TheFrac >> 16,	/* Integer value + Fractional value */
				(Word)(ly >> 8) >> COLBITS,
				viewheight
			);
#else
			ScaleGlue(
				TheFrac >> 8,	/* Integer value + Fractional value */
				(Word)(ly << 8) >> COLBITS,
				viewheight
			);
#endif	
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
	Word RunCount;
	int TopY;
	Word Index;
	LongWord Delta;

	if (!scale) {
		return;
	}

	if (detailshift == 0) {
		outp(SC_INDEX + 1, 1 << (x & 3));
	} else if (detailshift == 1) {
		outp(SC_INDEX + 1, 3 << ((x & 1) * 2));
	} else {
		// Do nothing, IO_ClearViewBuffer has already set the map mask
	}

	CharPtr = W_TryGetLumpByNum(lumpNum);
	if (!CharPtr) {
		if (scale < viewheight) {
			Word y = (viewheight - scale) / 2;
			dest = &ViewPointer[(y * PLANEWIDTH) + (x >> (2 - detailshift))];
		} else {
			scale = viewheight;
			dest = &ViewPointer[x >> (2 - detailshift)];
		}
		ScaleGlueFlat(lumpNum, scale);
	} else {
		CharPtr2 = (Byte __far*) CharPtr;
		TheFrac = ScaleDiv[scale];		/* Get the scale fraction */ 
		RunPtr = (SpriteRun __far*) &CharPtr[CharPtr[column + 1] / 2];	/* Get the offset to the RunPtr data */
		Screenad = &ViewPointer[x >> (2 - detailshift)];		/* Set the base screen address */
		TopY = (viewheight / 2) - scale;		/* Number of pixels for 128 pixel shape */

		while (RunPtr->Topy != (unsigned short) -1) {		/* Not end of record? */
			Y1 = scale * (LongWord)RunPtr->Topy / 128 + TopY;
			if (Y1 < (int)viewheight) {		/* Clip top? */
				Y2 = scale * (LongWord)RunPtr->Boty / 128 + TopY;
				if (Y2 > 0) {
					if (Y2 > (int)viewheight) {
						Y2 = viewheight;
					}
					Index = RunPtr->Shape + (RunPtr->Topy / 2);
					Delta = 0;
					if (Y1 < 0) {
						Delta = (0 - (Word)Y1) * TheFrac;
						Index += (Delta >> 24);
#if defined _M_I86
						Delta >>= 8;
#else
						Delta <<= 8;
#endif
						Delta = (Word)Delta >> COLBITS;
						Y1 = 0;
					}
					RunCount = Y2 - Y1;
					if (RunCount) {
						source = &CharPtr2[Index],	/* Pointer to art data */
						dest   = &Screenad[Y1 * PLANEWIDTH],			/* Pointer to screen */
#if defined _M_I86
						ScaleGlue(
						TheFrac >> 16,				/* Integer value + Fractional value */ 
						Delta,					/* Delta value */
						RunCount			/* Number of lines to draw */
						);
#else
						ScaleGlue(
						TheFrac >> 8,				/* Integer value + Fractional value */ 
						Delta,					/* Delta value */
						RunCount			/* Number of lines to draw */
						);
#endif
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
	Word Plane;

	if (!SmallFontPtr) {
		return;
	}
	x*=16;
	y*=16;
	Screenad = &VideoPointer[(y * PLANEWIDTH) + (x >> 2)];
	ArtStart = &SmallFontPtr[tile*(16*16)];
	Height = 0;
	do {
		Plane = x & 3;
		Width = 16;
		do {
			outp(SC_INDEX + 1, 1 << Plane);
			Screenad[0] = ArtStart[0];
			++ArtStart;
			if (++Plane == 4) {
				Plane = 0;
				++Screenad;
			}
		} while (--Width);
		Screenad+=PLANEWIDTH-4;
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
