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
   module: LL_MAN.H

   author: James R. Dose
   date:   February 4, 1994

   Public header for LL_MAN.C.  Linked list management routines.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __LL_MAN_H
#define __LL_MAN_H

void LL_AddNode(   uint8_t *node, uint8_t **head, uint8_t **tail, int32_t next, int32_t prev);
void LL_RemoveNode(uint8_t *node, uint8_t **head, uint8_t **tail, int32_t next, int32_t prev);

#define LL_AddToTail(type, listhead, node)              \
    LL_AddNode( ( uint8_t * )( node ),                  \
                ( uint8_t ** )&( ( listhead )->end ),   \
                ( uint8_t ** )&( ( listhead )->start ), \
                ( int32_t )&( ( type * ) 0 )->prev,     \
                ( int32_t )&( ( type * ) 0 )->next )

#define LL_Remove( type, listhead, node )                  \
    LL_RemoveNode( ( uint8_t * )( node ),                  \
                   ( uint8_t ** )&( ( listhead )->start ), \
                   ( uint8_t ** )&( ( listhead )->end ),   \
                   ( int32_t )&( ( type * ) 0 )->next,     \
                   ( int32_t )&( ( type * ) 0 )->prev )

#define LL_Empty(a) ((a)->start == NULL)

#define LL_Reset(list) (list)->start = NULL; (list)->end = NULL

#endif
