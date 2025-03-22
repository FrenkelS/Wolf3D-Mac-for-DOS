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
 *     Audio sub-system 16-bit
 *
 *-----------------------------------------------------------------------------*/

#include "Burger.h"
#include "i_system.h"
#include "i_audio.h"
#include "a_pcfx.h"
#include "Sounds.h"


static const Word priorities[NUMSOUNDS] = {
	0, // SND_NOSOUND
	99, // SND_THROWSWITCH
	90, // SND_GETKEY
	70, // SND_BONUS
	20, // SND_OPENDOOR
	50, // SND_DOGBARK
	50, // SND_DOGDIE
	50, // SND_ESEE
	50, // SND_ESEE2
	50, // SND_EDIE
	50, // SND_EDIE2
	0, // SND_BODYFALL
	0, // SND_PAIN
	80, // SND_GETAMMO
	49, // SND_KNIFE
	50, // SND_GUNSHT
	50, // SND_MGUN
	50, // SND_CHAIN
	50, // SND_FTHROW
	49, // SND_ROCKET
	50, // SND_PWALL
	0, // SND_PWALL2
	99, // SND_GUTEN
	99, // SND_SHIT
	85, // SND_HEAL
	87, // SND_THUMBSUP
	85, // SND_EXTRA
	0, // SND_OUCH1
	0, // SND_OUCH2
	99, // SND_PDIE
	10, // SND_HITWALL
	49, // SND_KNIFEMISS
	50, // SND_BIGGUN
	99, // SND_COMEHERE
	50, // SND_ESEE3
	50, // SND_ESEE4
	0, // SND_OK
	0, // SND_MENU
	99, // SND_HITLERSEE
	99, // SND_SHITHEAD
	50, // SND_BOOM
	10, // SND_LOCKEDDOOR
	45, // SND_MECHSTEP
};


static Word sfxDevice = AHW_NONE;


void I_InitSound(void)
{
	if (M_CheckParm("nosound")) {
		return;
	}

	// Sound effects
	if (M_CheckParm("nosfx")) {
		return;
	}

	sfxDevice = AHW_PC_SPEAKER;
	PCFX_Init();
}


void I_ShutdownSound(void)
{
	if (sfxDevice == AHW_PC_SPEAKER)
		PCFX_Shutdown();
}


void PlaySound(Word SoundNum)
{
	if (SoundNum == 0)
		return;

	if (sfxDevice == AHW_NONE)
		return;

	SoundNum &= 0x7fff;
	PCFX_Play(SoundNum, priorities[SoundNum]);
}


void StopSound(Word SoundNum)
{
	if (SoundNum == 0)
		return;

	if (sfxDevice == AHW_NONE)
		return;

	PCFX_Stop(SoundNum);
}


void PlaySong(Word Song)
{
	UNUSED(Song);
}
