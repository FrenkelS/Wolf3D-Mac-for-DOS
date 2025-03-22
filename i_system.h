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
 *      DOS Wolfenstein 3-D header
 *
 *-----------------------------------------------------------------------------*/

#ifndef __I_SYSTEM__
#define __I_SYSTEM__


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "w_wad.h"
#include "z_zone.h"


_Noreturn void I_Error(const char *error, ...);


#define UNUSED(x)	(x = x)	// for pesky compiler / lint warnings


#define LOBYTE(w)	(((uint8_t *)&w)[0])
#define HIBYTE(w)	(((uint8_t *)&w)[1])


int M_GetParmCount(void);
int M_GetParm(int p);
int M_CheckParm(char *check);


void I_Quit(void);


void I_InitKeyboard(void);
void I_ShutdownKeyboard(void);
void I_InitMouse(void);
void I_ShutdownMouse(void);


void I_InitTimer(void);
void I_ShutdownTimer(void);


void I_InitSound(void);
void I_ShutdownSound(void);


void I_InitGraphics(void);
void I_ShutdownGraphics(void);
void I_SetViewSize(Word blocks);
void I_ClearView(void);


#endif
