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

#include <string.h>

#include "wolfdef.h"
#include "a_taskmn.h"


static task __far* t;

static volatile LongWord ticcount;
LongWord LastTick;				/* Last system tick (60hz) */

static Boolean isTimerSet;


static void I_TimerISR(void)
{
	ticcount++;
}

/**********************************

	Wait a single system tick

**********************************/

void WaitTick(void)
{
	do {
	} while (ReadTick()==LastTick);	/* Tick changed? */
	LastTick=ReadTick();		/* Save it */
}

/**********************************

	Wait a specific number of system ticks
	from a time mark before you get control

**********************************/

void WaitTicks(Word Count)
{
	LongWord TickMark;		/* Temp tick mark */

	do {
		TickMark = ReadTick();	/* Get the mark */
	} while ((TickMark-LastTick)<=Count);	/* Time up? */
	LastTick = TickMark;	/* Save the new time mark */
}

/**********************************

	Get the current system tick

**********************************/

LongWord ReadTick(void)
{
	return ticcount;
}


void I_InitTimer(void)
{
	t = TS_ScheduleTask(I_TimerISR, TICRATE, 0); // max priority
	TS_Dispatch();

	isTimerSet = TRUE;
}


void I_ShutdownTimer(void)
{
	if (isTimerSet) {
		TS_Terminate(t);
		t = NULL;
		TS_Shutdown();
	}
}
