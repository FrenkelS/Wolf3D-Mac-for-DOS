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
 *     Audio sub-system 32-bit
 *
 *-----------------------------------------------------------------------------*/

#include <string.h>

#include "Burger.h"
#include "i_system.h"
#include "i_audio.h"
#include "a_al_mid.h"
#include "a_blast.h"
#include "a_mpu401.h"
#include "a_multiv.h"
#include "a_music.h"
#include "a_pcfx.h"
#include "Sounds.h"


#define NUM_CHANNELS 1


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


static Word musicDevice = AHW_NONE;
static Word sfxDevice   = AHW_NONE;


void I_InitSound(void)
{
	if (M_CheckParm("nosound"))
		return;


	// Music
	if (!M_CheckParm("nomusic")) {
		int p = M_CheckParm("mpu401");
		if (p && p < M_GetParmCount() - 1) {
			int32_t snd_Mport = M_GetParm(p + 1);
			if (MPU_Init(snd_Mport)) {
				I_Error("The MPU-401 isn't reponding");
			}
			musicDevice = AHW_MPU_401;
		} else if (!M_CheckParm("noal") && AL_DetectFM()) {
			uint8_t __far* genmidi = W_GetLumpByNum(62);
			AL_RegisterTimbreBank(genmidi);
			Z_Free(genmidi);
			musicDevice = AHW_ADLIB;
		}
	}

	if (musicDevice != AHW_NONE)
	{
		int32_t status = MUSIC_Init(musicDevice);
		if (status == MUSIC_Ok)
			MUSIC_SetVolume(0);
	}


	// Sound effects
	if (!M_CheckParm("nosfx")) {
		if (!M_CheckParm("nosb") && !BLASTER_Detect()) {
			sfxDevice = AHW_SOUND_BLASTER;
			BLASTER_Init();
			MV_Init(sfxDevice, NUM_CHANNELS);
		} else {
			sfxDevice = AHW_PC_SPEAKER;
			PCFX_Init();
		}
	}
}


void I_ShutdownSound(void)
{
	MUSIC_Shutdown();

	if (sfxDevice == AHW_PC_SPEAKER)
		PCFX_Shutdown();
	else if (sfxDevice == AHW_SOUND_BLASTER)
		MV_Shutdown();
}


/**********************************

	Play a sound resource

**********************************/

static Word LastSoundNum = -1;
static Word LastPriority = -1;
static Word soundHandle  = -1;
static uint8_t __far data[65535];

void PlaySound(Word SoundNum)
{
	uint16_t length;
	uint8_t __far* lump;
	Word priority;

	if (SoundNum == 0)
		return;

	if (sfxDevice == AHW_NONE)
		return;

	SoundNum &= 0x7fff;
	priority  = priorities[SoundNum];

	if (sfxDevice == AHW_PC_SPEAKER) {
		PCFX_Play(SoundNum, priority);
		return;
	}

	if (MV_VoicePlaying(soundHandle)) {
		if (priority < LastPriority) {
			return;
		}

		MV_Kill(soundHandle);
	}

	length = W_LumpLength(SoundNum + 77);
	lump   = W_TryGetLumpByNum(SoundNum + 77);
	if (lump != NULL) {
		_fmemcpy(data, lump, length);
		Z_ChangeTagToCache(lump);
	} else {
		W_ReadLumpByNum(SoundNum + 77, data);
	}

	LastSoundNum = SoundNum;
	LastPriority = priority;
	soundHandle  = MV_PlayRaw(data, length);
}

/**********************************

	Stop playing a sound resource

**********************************/

void StopSound(Word SoundNum)
{
	if (SoundNum == 0)
		return;

	if (sfxDevice == AHW_NONE)
		return;

	if (sfxDevice == AHW_PC_SPEAKER) {
		PCFX_Stop(SoundNum);
		return;
	}

	if (LastSoundNum != SoundNum)
		return;

	MV_Kill(soundHandle);

	LastSoundNum = -1;
	LastPriority = -1;
	soundHandle  = -1;
}


static Word LastSong = -1;

void PlaySong(Word Song)
{
	if (musicDevice == AHW_NONE)
		return;

	if (Song) {
		if (Song!=LastSong) {
			uint8_t __far* lump = W_GetLumpByNum(Song);
			int32_t status = MUSIC_PlaySong(lump);
			if (status == MUSIC_Ok)
				MUSIC_SetVolume(254);
			LastSong = Song;
		}
		return;
	}
	MUSIC_StopSong();
	if (LastSong != -1)
		Z_ChangeTagToCache(W_GetLumpByNum(LastSong));
	LastSong = -1;
}
