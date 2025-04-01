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
 *
 *-----------------------------------------------------------------------------*/

#include <stdarg.h>
#include <string.h>

#include "wolfdef.h"


static int myargc;
static const char * const * myargv;


int M_GetParmCount(void)
{
	return myargc;
}


int M_GetParm(int p)
{
	return strtol(myargv[p], NULL, 0x10);
}


int M_CheckParm(char *check)
{
	int i;

	for (i = 1; i < myargc; i++)
		if (!stricmp(check, myargv[i]))
			return i;

	return 0;
}


/***************************

	Init all the system tools

***************************/

void InitTools(void)
{
	Z_Init();
	W_Init();
	I_InitKeyboard();
	I_InitMouse();
	I_InitTimer();

	__djgpp_nearptr_enable();
	I_InitSound();
	I_InitGraphics();

	GetTableMemory();			/* Get memory for far tables math tables */
	MapListPtr = (maplist_t __far*) W_GetMapLumpByNum(rMapList);	/* Get the map list */
	SongListPtr = (unsigned short __far*) W_GetMapLumpByNum(rSongList);
	WallListPtr = (unsigned short __far*) W_GetLumpByNum(MyWallList);
}


static void I_Shutdown(void)
{
	I_ShutdownGraphics();
	I_ShutdownSound();
	I_ShutdownTimer();
	I_ShutdownMouse();
	I_ShutdownKeyboard();
	W_Shutdown();
	Z_Shutdown();
}


void I_Quit(void)
{
	I_Shutdown();

	exit(0);
}


void I_Error(const char *error, ...)
{
	va_list argptr;

	I_Shutdown();

	va_start(argptr, error);
	vprintf(error, argptr);
	va_end(argptr);
	printf("\n");
	exit(1);
}


/**********************************

	Choose the game difficulty
	
**********************************/

Word ChooseGameDiff(void)
{
	if (M_CheckParm("baby"))
		difficulty = 0;
	else if (M_CheckParm("easy"))
		difficulty = 1;
	//else if (M_CheckParm("normal"))
	//	difficulty = 2;
	else if (M_CheckParm("hard"))
		difficulty = 3;
	else
		difficulty = 2;

	return TRUE;
}


void FinishLoadGame(void)
{
	//TODO printf("Implement me: FinishLoadGame()");
}

/**********************************

	Beg for $$$ at the end of the shareware version
	
**********************************/

void ShareWareEnd(void)
{
	//TODO printf("Implement me: ShareWareEnd()\n");
}


int main(int argc, const char * const * argv)
{
	printf("MacWolf for DOS\n");

	myargc = argc;
	myargv = argv;

	WolfMain();
	return 0;
}
