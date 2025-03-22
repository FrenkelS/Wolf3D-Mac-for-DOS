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
   module: TASK_MAN.H

   author: James R. Dose
   date:   July 25, 1994

   Public header for TASK_MAN.C, a low level timer task scheduler.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __TASK_MAN_H
#define __TASK_MAN_H

typedef struct task
{
	struct task __far*  next;
	struct task __far*  prev;
	void				(*TaskService)(void);
	int32_t				rate;
	volatile int32_t	count;
	int32_t				priority;
	Boolean				active;
} task;

void TS_Shutdown(void);
task __far* TS_ScheduleTask(void (*Function)(void), int32_t rate, int32_t priority);
void TS_Terminate(task __far* ptr);
void TS_Dispatch(void);
void TS_SetTaskRate(task *Task, int32_t rate);

#endif
