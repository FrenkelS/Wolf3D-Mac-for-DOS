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
   module: LL_MAN.C

   author: James R. Dose
   date:   January 1, 1994

   Linked list management routines.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include "i_system.h"
#include "a_ll_man.h"


#define OFFSET( structure, offset ) \
   ( *( ( uint8_t ** )&( structure )[ offset ] ) )


void LL_AddNode(uint8_t *item, uint8_t **head, uint8_t **tail, int32_t next, int32_t prev)
{
	OFFSET(item, prev) = NULL;
	OFFSET(item, next) = *head;

	if (*head)
		OFFSET(*head, prev) = item;
	else
		*tail = item;

	*head = item;
}


void LL_RemoveNode(uint8_t *item, uint8_t **head, uint8_t **tail, int32_t next, int32_t prev)
{
	if (OFFSET(item, prev) == NULL)
		*head = OFFSET(item, next);
	else
		OFFSET(OFFSET(item, prev), next) = OFFSET(item, next);

	if (OFFSET(item, next) == NULL)
		*tail = OFFSET(item, prev);
	else
		OFFSET(OFFSET(item, next), prev) = OFFSET(item, prev);

	OFFSET(item, next) = NULL;
	OFFSET(item, prev) = NULL;
}
