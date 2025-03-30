/* Emacs style mode select   -*- C++ -*-
 *-----------------------------------------------------------------------------
 *
 *
 *  PrBoom: a Doom port merged with LxDoom and LSDLDoom
 *  based on BOOM, a modified and improved DOOM engine
 *  Copyright (C) 1999 by
 *  id Software, Chi Hoang, Lee Killough, Jim Flynn, Rand Phares, Ty Halderman
 *  Copyright (C) 1999-2001 by
 *  Jess Haas, Nicolas Kalkhof, Colin Phipps, Florian Schulze
 *  Copyright 2005, 2006 by
 *  Florian Schulze, Colin Phipps, Neil Stevens, Andrey Budko
 *  Copyright 2023-2025 by
 *  Frenkel Smeijers
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
 *      Handles WAD file header, directory, lump I/O.
 *
 *-----------------------------------------------------------------------------
 */

#include <string.h>

#include "Burger.h"
#include "i_system.h"
#include "w_wad.h"
#include "z_zone.h"


//
// TYPES
//

typedef struct
{
  int32_t  filepos;
  uint16_t size;
  int16_t  filler;        // always zero
  char name[8];
} filelump_t;


//
// GLOBALS
//

static FILE* fileWAD;

static int16_t numlumps;

static filelump_t __far* fileinfo;

static void __far*__far* lumpcache;

//
// LUMP BASED ROUTINES.
//

#define BUFFERSIZE 512

static void _ffread(void __far* ptr, uint16_t size, FILE* fp)
{
	uint8_t __far* dest = ptr;
	uint8_t buffer[BUFFERSIZE];

	while (size >= BUFFERSIZE)
	{
		fread(buffer, BUFFERSIZE, 1, fp);
		_fmemcpy(dest, buffer, BUFFERSIZE);
		dest += BUFFERSIZE;
		size -= BUFFERSIZE;
	}

	if (size > 0)
	{
		fread(buffer, size, 1, fp);
		_fmemcpy(dest, buffer, size);
	}
}


static Boolean W_LoadWADIntoXMS(void)
{
	int32_t size;
	Boolean xms;
	uint8_t buffer[BUFFERSIZE];
	uint32_t dest;

	fseek(fileWAD, 0, SEEK_END);
	size = ftell(fileWAD);
	xms = Z_InitXms(size);
	if (!xms)
	{
#if defined __IA16_SYS_MSDOS
		I_Error("Not enough XMS available");
#else
		printf("Not enough XMS available\n");
#endif
		return FALSE;
	}

	printf("Loading WAD file into XMS\n");
	printf("Get Psyched!\n");

	fseek(fileWAD, 0, SEEK_SET);
	dest = 0;

	while (size >= BUFFERSIZE)
	{
		fread(buffer, BUFFERSIZE, 1, fileWAD);
		Z_MoveConventionalMemoryToExtendedMemory(dest, buffer, BUFFERSIZE);
		dest += BUFFERSIZE;
		size -= BUFFERSIZE;
	}

	if (size > 0)
	{
		fread(buffer, size, 1, fileWAD);
		Z_MoveConventionalMemoryToExtendedMemory(dest, buffer, size);
	}

	return TRUE;
}


static void W_ReadDataFromFile(void __far* dest, uint32_t src, uint16_t length)
{
	fseek(fileWAD, src, SEEK_SET);
	_ffread(dest, length, fileWAD);
}


typedef void (*W_ReadData_f)(void __far* dest, uint32_t src, uint16_t length);
static W_ReadData_f readfunc;


typedef struct
{
  char identification[4]; // Should be "IWAD" or "PWAD".
  int16_t  numlumps;
  int16_t  filler;        // always zero
  int32_t  infotableofs;
} wadinfo_t;

void W_Init(void)
{
	Boolean xms;
	wadinfo_t header;

	fileWAD = fopen("MACWOLF2.WAD", "rb");
	if (fileWAD != NULL) {
		printf("Second");
	} else {
		fileWAD = fopen("MACWOLF1.WAD", "rb");
		if (fileWAD != NULL) {
			printf("First");
		} else {
			I_Error("Can't open WAD file.");
		}
	}
	printf(" Encounter\n");

	xms = W_LoadWADIntoXMS();
	readfunc = xms ? Z_MoveExtendedMemoryToConventionalMemory : W_ReadDataFromFile;

	readfunc(&header, 0, sizeof(header));

	fileinfo = Z_MallocStatic(header.numlumps * sizeof(filelump_t));
	readfunc(fileinfo, header.infotableofs, sizeof(filelump_t) * header.numlumps);

	lumpcache = Z_MallocStatic(header.numlumps * sizeof(*lumpcache));
	_fmemset(lumpcache, 0, header.numlumps * sizeof(*lumpcache));

	numlumps = header.numlumps;
}


void W_Shutdown(void)
{
	readfunc = W_ReadDataFromFile;
}


uint16_t W_LumpLength(int16_t num)
{
	return fileinfo[num].size;
}


void W_ReadLumpByNum(int16_t num, void __far* ptr)
{
	const filelump_t __far* lump = &fileinfo[num];
	readfunc(ptr, lump->filepos, lump->size);
}


static void __far* W_GetLumpByNumWithUser(int16_t num, void __far*__far* user)
{
	const filelump_t __far* lump;
	void __far* ptr;

	lump = &fileinfo[num];

	ptr = Z_MallocStaticWithUser(lump->size, user);

	readfunc(ptr, lump->filepos, lump->size);
	return ptr;
}


void __far* W_GetLumpByNum(int16_t num)
{
	if (lumpcache[num])
		Z_ChangeTagToStatic(lumpcache[num]);
	else
		lumpcache[num] = W_GetLumpByNumWithUser(num, &lumpcache[num]);

	return lumpcache[num];
}


Boolean W_IsLumpCached(int16_t num)
{
	return lumpcache[num] != NULL;
}


void __far* W_TryGetLumpByNum(int16_t num)
{
	if (lumpcache[num])
	{
		Z_ChangeTagToStatic(lumpcache[num]);
		return lumpcache[num];
	}
	else if (Z_IsEnoughFreeMemory(W_LumpLength(num)))
		return W_GetLumpByNum(num);
	else
		return NULL;
}
