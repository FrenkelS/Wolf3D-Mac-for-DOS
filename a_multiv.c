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
   module: MULTIVOC.C

   author: James R. Dose
   date:   December 20, 1993

   Routines to provide multichannel digitized sound playback for
   Sound Blaster compatible sound cards.

   (c) Copyright 1993 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <dos.h>
#include <time.h>

#include "i_system.h"
#include "i_audio.h"
#include "a_blast.h"
#include "a_dma.h"
#include "a_inter.h"
#include "a_ll_man.h"
#include "a_multiv.h"

enum MV_Errors
{
	MV_Error   = -1,
	MV_Ok      = 0,
	MV_UnsupportedCard,
	MV_NotInstalled,
	MV_NoVoices,
	MV_NoMem,
	MV_VoiceNotFound,
	MV_BlasterError,
	MV_DMAFailure
};

typedef enum
{
	NoMoreData,
	KeepPlaying
} playbackstatus;


typedef struct VoiceNode
{
	struct VoiceNode *next;
	struct VoiceNode *prev;

	void ( *mix )( void );

	uint8_t __far* NextBlock;
	uint32_t BlockLength;

	uint32_t FixedPointBufferSize;

	uint8_t __far* sound;
	uint32_t length;
	uint32_t RateScale;
	uint32_t position;
	Boolean  Playing;

	Word     handle;
	int32_t  priority;

	int16_t *LeftVolume;
	int16_t *RightVolume;
} VoiceNode;

static playbackstatus MV_GetNextRawBlock(VoiceNode *voice);

typedef struct
{
	VoiceNode *start;
	VoiceNode *end;
} VList;

#define MV_NumVoices       8

typedef uint8_t HARSH_CLIP_TABLE_8[ MV_NumVoices * 256 ];

void MV_Mix8BitMono(void);
void MV_Mix8BitStereo(void);
void MV_Mix16BitMono(void);
void MV_Mix16BitStereo(void);


#define IS_QUIET( ptr )  ( ( void * )( ptr ) == ( void * )&MV_VolumeTable[ 0 ] )

#define MV_MaxVolume       63

//static signed short MV_VolumeTable[ MV_MaxVolume + 1 ][ 256 ];
static int16_t MV_VolumeTable[ 63 + 1 ][ 256 ];

static Boolean MV_Installed   = FALSE;
static int32_t MV_SoundCard   = AHW_SOUND_BLASTER;


#define MixBufferSize     256

#define NumberOfBuffers   16
#define TotalBufferSize   (MixBufferSize * NumberOfBuffers)

static int32_t MV_BufferSize      = MixBufferSize;
static int32_t MV_NumberOfBuffers = NumberOfBuffers;

static int32_t MV_MixMode    = MONO_8BIT;
static int32_t MV_Channels   = 1;
static int32_t MV_Bits       = 8;

#define SILENCE_16BIT     0
#define SILENCE_8BIT      0x80808080

static int32_t MV_Silence    = SILENCE_8BIT;
static Boolean MV_SwapLeftRight = FALSE;

#define SAMPLE_RATE 22050

static int32_t MV_RequestedMixRate;
static int32_t MV_MixRate;

static int32_t MV_DMAChannel = -1;
static int32_t MV_BuffShift;

static int32_t MV_TotalMemory;

static int      MV_BufferDescriptor;
static Boolean  MV_BufferEmpty[NumberOfBuffers];
static uint8_t *MV_MixBuffer[NumberOfBuffers + 1];

static VoiceNode __far* MV_Voices = NULL;

static volatile VList VoiceList = { NULL, NULL };
static volatile VList VoicePool = { NULL, NULL };

#define MV_MinVoiceHandle  1

static int32_t MV_MixPage      = 0;
static Word    MV_VoiceHandle  = MV_MinVoiceHandle;

uint8_t *MV_HarshClipTable		__attribute__ ((externally_visible));
uint8_t *MV_MixDestination		__attribute__ ((externally_visible));
uint32_t MV_MixPosition			__attribute__ ((externally_visible));
int16_t *MV_LeftVolume			__attribute__ ((externally_visible));
int16_t *MV_RightVolume			__attribute__ ((externally_visible));
int32_t  MV_SampleSize			__attribute__ ((externally_visible)) = 1;
int32_t  MV_RightChannelOffset	__attribute__ ((externally_visible));


static int32_t MV_ErrorCode = MV_Ok;

#define MV_SetErrorCode( status ) \
   MV_ErrorCode   = ( status )


/*---------------------------------------------------------------------
   Function: MV_Mix

   Mixes the sound into the buffer.
---------------------------------------------------------------------*/

uint32_t MV_Position	__attribute__ ((externally_visible));
uint32_t MV_Rate		__attribute__ ((externally_visible));
uint8_t *MV_Start		__attribute__ ((externally_visible));
uint32_t MV_Length		__attribute__ ((externally_visible));

static void MV_Mix(VoiceNode *voice, int32_t buffer)
{
	int32_t   length;
	uint32_t  FixedPointBufferSize;

	if ((voice->length == 0) && (MV_GetNextRawBlock(voice) != KeepPlaying))
		return;

	length               = MixBufferSize;
	FixedPointBufferSize = voice->FixedPointBufferSize;

	MV_MixDestination    = MV_MixBuffer[buffer];
	MV_LeftVolume        = voice->LeftVolume;
	MV_RightVolume       = voice->RightVolume;

	if ((MV_Channels == 2) && (IS_QUIET(MV_LeftVolume)))
	{
		MV_LeftVolume      = MV_RightVolume;
		MV_MixDestination += MV_RightChannelOffset;
	}

	// Add this voice to the mix
	while (length > 0)
	{
		MV_Start    = voice->sound;
		MV_Rate     = voice->RateScale;
		MV_Position = voice->position;

		// Check if the last sample in this buffer would be
		// beyond the length of the sample block
		if ((MV_Position + FixedPointBufferSize) >= voice->length)
		{
			if (MV_Position < voice->length)
				MV_Length = (voice->length - MV_Position + MV_Rate - 1) / MV_Rate;
			else
			{
				MV_GetNextRawBlock(voice);
				return;
			}
		}
		else
			MV_Length = length;

		voice->mix();

		if (MV_Length & 1)
		{
			MV_MixPosition += MV_Rate;
			MV_Length      -= 1;
		}
		voice->position = MV_MixPosition;

		length -= MV_Length;

		if (voice->position >= voice->length)
		{
			// Get the next block of sound
			if (MV_GetNextRawBlock(voice) != KeepPlaying)
				return;

			if (length > 0)
			{
				// Get the position of the last sample in the buffer
				FixedPointBufferSize = voice->RateScale * (length - 1);
			}
		}
	}
}


/*---------------------------------------------------------------------
   Function: MV_PlayVoice

   Adds a voice to the play list.
---------------------------------------------------------------------*/

static void MV_PlayVoice(VoiceNode *voice)
{
	uint32_t flags = DisableInterrupts();

	LL_AddToTail(VoiceNode, &VoiceList, voice);

	RestoreInterrupts(flags);
}


/*---------------------------------------------------------------------
   Function: MV_StopVoice

   Removes the voice from the play list and adds it to the free list.
---------------------------------------------------------------------*/

static void MV_StopVoice(VoiceNode *voice)
{
	uint32_t flags = DisableInterrupts();

	// move the voice from the play list to the free list
	LL_Remove(VoiceNode, &VoiceList, voice);
	LL_AddToTail(VoiceNode, &VoicePool, voice);

	RestoreInterrupts(flags);
}


static void ClearBuffer_DW(void *ptr, uint32_t data, int32_t length)
{
	int32_t i;

	for (i = 0; i < length; i++)
		((uint32_t *)ptr)[i] = data;
}


/*---------------------------------------------------------------------
   Function: MV_ServiceVoc

   Starts playback of the waiting buffer and mixes the next one.
---------------------------------------------------------------------*/

static void MV_ServiceVoc(void)
{
	VoiceNode *voice;
	VoiceNode *next;
	uint8_t   *buffer;

	if (MV_DMAChannel >= 0)
	{
		// Get the currently playing buffer
		buffer = DMA_GetCurrentPos(MV_DMAChannel);
		MV_MixPage   = (uint32_t)(buffer - MV_MixBuffer[0]);
		MV_MixPage >>= MV_BuffShift;
	}

	// Toggle which buffer we'll mix next
	MV_MixPage++;

	if (MV_MixPage >= MV_NumberOfBuffers)
		MV_MixPage -= MV_NumberOfBuffers;

	// Initialize buffer
	//Commented out so that the buffer is always cleared.
	//This is so the guys at Echo Speech can mix into the
	//buffer even when no sounds are playing.
	if (!MV_BufferEmpty[MV_MixPage])
	{
		ClearBuffer_DW(MV_MixBuffer[MV_MixPage], MV_Silence, MV_BufferSize >> 2);
		MV_BufferEmpty[MV_MixPage] = TRUE;
	}

	// Play any waiting voices
	voice = VoiceList.start;
	while (voice != NULL)
	{
		MV_BufferEmpty[MV_MixPage] = FALSE;

		MV_Mix(voice, MV_MixPage);

		next = voice->next;

		// Is this voice done?
		if (!voice->Playing)
			MV_StopVoice(voice);

		voice = next;
	}
}


/*---------------------------------------------------------------------
   Function: MV_GetNextRawBlock

   Controls playback of demand fed data.
---------------------------------------------------------------------*/

static playbackstatus MV_GetNextRawBlock(VoiceNode *voice)
{
	if (voice->BlockLength == 0)
	{
		voice->Playing = FALSE;

		return NoMoreData;
	} else {
		voice->sound        = voice->NextBlock;
		voice->position    -= voice->length;
		voice->length       = voice->BlockLength > 0x8000 ? 0x8000 : voice->BlockLength;
		voice->NextBlock   += voice->length;
		voice->BlockLength -= voice->length;
		voice->length     <<= 16;

		return KeepPlaying;
	}
}


/*---------------------------------------------------------------------
   Function: MV_GetVoice

   Locates the voice with the specified handle.
---------------------------------------------------------------------*/

static VoiceNode *MV_GetVoice(Word handle)
{
	VoiceNode *voice;
	
	uint32_t flags = DisableInterrupts();

	voice = VoiceList.start;
	while (voice != NULL)
	{
		if (handle == voice->handle)
			break;

		voice = voice->next;
	}

	RestoreInterrupts(flags);

	if (voice == NULL)
		MV_SetErrorCode(MV_VoiceNotFound);

	return voice;
}


/*---------------------------------------------------------------------
   Function: MV_VoicePlaying

   Checks if the voice associated with the specified handle is
   playing.
---------------------------------------------------------------------*/

Boolean MV_VoicePlaying(Word handle)
{
	VoiceNode *voice;

	if (!MV_Installed)
	{
		MV_SetErrorCode(MV_NotInstalled);
		return FALSE;
	}

	voice = MV_GetVoice(handle);

	return voice != NULL;
}


/*---------------------------------------------------------------------
   Function: MV_KillAllVoices

   Stops output of all currently active voices.
---------------------------------------------------------------------*/

static void MV_KillAllVoices(void)
{
	if (!MV_Installed)
	{
		MV_SetErrorCode(MV_NotInstalled);
		return;
	}

	// Remove all the voices from the list
	while (VoiceList.start != NULL)
	{
		MV_Kill(VoiceList.start->handle);
	}
}


/*---------------------------------------------------------------------
   Function: MV_Kill

   Stops output of the voice associated with the specified handle.
---------------------------------------------------------------------*/

void MV_Kill(Word handle)
{
	VoiceNode *voice;
	uint32_t  flags;

	if (!MV_Installed)
	{
		MV_SetErrorCode(MV_NotInstalled);
		return;
	}

	flags = DisableInterrupts();

	voice = MV_GetVoice(handle);
	if (voice == NULL)
		MV_SetErrorCode(MV_VoiceNotFound);
	else
		MV_StopVoice(voice);

	RestoreInterrupts(flags);
}


/*---------------------------------------------------------------------
   Function: MV_AllocVoice

   Retrieve an inactive or lower priority voice for output.
---------------------------------------------------------------------*/

static VoiceNode *MV_AllocVoice(int32_t priority)
{
	VoiceNode   *voice;
	VoiceNode   *node;

	uint32_t flags = DisableInterrupts();

	// Check if we have any free voices
	if (LL_Empty(&VoicePool))
	{
		// check if we have a higher priority than a voice that is playing.
		voice = node = VoiceList.start;
		while (node != NULL)
		{
			if (node->priority < voice->priority)
				voice = node;

			node = node->next;
		}

		if (priority >= voice->priority)
			MV_Kill( voice->handle );
	}

	// Check if any voices are in the voice pool
	if (LL_Empty(&VoicePool))
	{
		// No free voices
		RestoreInterrupts(flags);
		return NULL;
	}

	voice = VoicePool.start;
	LL_Remove(VoiceNode, &VoicePool, voice);
	RestoreInterrupts(flags);

	// Find a free voice handle
	do
	{
		MV_VoiceHandle++;
		if (MV_VoiceHandle < MV_MinVoiceHandle)
			MV_VoiceHandle = MV_MinVoiceHandle;
	} while (MV_VoicePlaying(MV_VoiceHandle));

	voice->handle = MV_VoiceHandle;

	return voice;
}


/*---------------------------------------------------------------------
   Function: MV_SetVoicePitch

   Sets the pitch for the specified voice.
---------------------------------------------------------------------*/

static void MV_SetVoicePitch(VoiceNode *voice)
{
	voice->RateScale = (SAMPLE_RATE * 0x10000) / MV_MixRate;

	// Multiply by MixBufferSize - 1
	voice->FixedPointBufferSize = (voice->RateScale * MixBufferSize) - voice->RateScale;
}


/*---------------------------------------------------------------------
   Function: MV_GetVolumeTable

   Returns a pointer to the volume table associated with the specified
   volume.
---------------------------------------------------------------------*/

static int16_t *MV_GetVolumeTable(int32_t vol)
{
	int16_t *table;

	if (vol > 255)
		vol = 255;
	else if (vol < 0)
		vol = 0;

	vol = vol >> 2;

	table = (int16_t *) &MV_VolumeTable[vol];

	return table;
}


/*---------------------------------------------------------------------
   Function: MV_SetVoiceMixMode

   Selects which method should be used to mix the voice.
---------------------------------------------------------------------*/

#define T_SIXTEENBIT_STEREO 0
#define T_8BITS       1
#define T_MONO        2
#define T_LEFTQUIET   8
#define T_RIGHTQUIET  16
#define T_DEFAULT     T_SIXTEENBIT_STEREO

static void MV_SetVoiceMixMode(VoiceNode *voice)
{
	uint32_t flags;
	 int32_t test;

	flags = DisableInterrupts();

	test = T_DEFAULT;
	if (MV_Bits == 8)
		test |= T_8BITS;

	if (MV_Channels == 1)
		test |= T_MONO;
	else
	{
		if (IS_QUIET(voice->RightVolume))
			test |= T_RIGHTQUIET;
		else if (IS_QUIET(voice->LeftVolume))
			test |= T_LEFTQUIET;
	}

	switch (test)
	{
		case T_8BITS | T_MONO:
			voice->mix = MV_Mix8BitMono;
			break;

		case T_8BITS | T_LEFTQUIET:
			MV_LeftVolume = MV_RightVolume;
			voice->mix = MV_Mix8BitMono;
			break;

		case T_8BITS | T_RIGHTQUIET:
			voice->mix = MV_Mix8BitMono;
			break;

		case T_8BITS:
			voice->mix = MV_Mix8BitStereo;
			break;

		case T_MONO:
			voice->mix = MV_Mix16BitMono;
			break;

		case T_LEFTQUIET:
			MV_LeftVolume = MV_RightVolume;
			voice->mix = MV_Mix16BitMono;
			break;

		case T_RIGHTQUIET:
			voice->mix = MV_Mix16BitMono;
			break;

		case T_SIXTEENBIT_STEREO:
			voice->mix = MV_Mix16BitStereo;
			break;

		default:
			voice->mix = MV_Mix8BitMono;
	}

	RestoreInterrupts(flags);
}


/*---------------------------------------------------------------------
   Function: MV_SetVoiceVolume

   Sets the stereo and mono volume level of the voice associated
   with the specified handle.
---------------------------------------------------------------------*/

static void MV_SetVoiceVolume(VoiceNode *voice, int32_t vol, int32_t left, int32_t right)
{
	if (MV_Channels == 1)
	{
		left  = vol;
		right = vol;
	}

	if (MV_SwapLeftRight)
	{
		// SBPro uses reversed panning
		voice->LeftVolume  = MV_GetVolumeTable(right);
		voice->RightVolume = MV_GetVolumeTable(left);
	} else {
		voice->LeftVolume  = MV_GetVolumeTable(left);
		voice->RightVolume = MV_GetVolumeTable(right);
	}

	MV_SetVoiceMixMode(voice);
}


/*---------------------------------------------------------------------
   Function: MV_SetMixMode

   Prepares Multivoc to play stereo of mono digitized sounds.
---------------------------------------------------------------------*/

static void MV_SetMixMode(void)
{
	if (!MV_Installed)
	{
		MV_SetErrorCode(MV_NotInstalled);
		return;
	}

	switch(MV_SoundCard)
	{
		case AHW_SOUND_BLASTER:
			MV_MixMode = BLASTER_SetMixMode(STEREO | SIXTEEN_BIT);
			break;
	}

	MV_Channels = MV_MixMode & STEREO      ?  2 : 1;
	MV_Bits     = MV_MixMode & SIXTEEN_BIT ? 16 : 8;

	MV_BuffShift  = 7 + MV_Channels;
	MV_SampleSize = MV_Channels;

	if (MV_Bits == 8)
		MV_Silence = SILENCE_8BIT;
	else
	{
		MV_Silence     = SILENCE_16BIT;
		MV_BuffShift  += 1;
		MV_SampleSize *= 2;
	}

	MV_BufferSize = MixBufferSize * MV_SampleSize;
	MV_NumberOfBuffers = TotalBufferSize / MV_BufferSize;

	MV_RightChannelOffset = MV_SampleSize / 2;
}


/*---------------------------------------------------------------------
   Function: MV_StartPlayback

   Starts the sound playback engine.
---------------------------------------------------------------------*/

static int32_t MV_StartPlayback(void)
{
	int32_t status;
	int32_t buffer;

	// Initialize the buffers
	ClearBuffer_DW(MV_MixBuffer[0], MV_Silence, TotalBufferSize >> 2);
	for (buffer = 0; buffer < MV_NumberOfBuffers; buffer++)
		MV_BufferEmpty[buffer] = TRUE;

	// Set the mix buffer variables
	MV_MixPage = 1;

	// Start playback
	switch (MV_SoundCard)
	{
		case AHW_SOUND_BLASTER:
			status = BLASTER_BeginBufferedPlayback(MV_MixBuffer[0], TotalBufferSize, MV_NumberOfBuffers, MV_RequestedMixRate, MV_MixMode, MV_ServiceVoc);
			if (status != BLASTER_Ok)
			{
				MV_SetErrorCode(MV_BlasterError);
				return MV_Error;
			}

			MV_MixRate = BLASTER_GetPlaybackRate();
			MV_DMAChannel = BLASTER_GetDMAChannel();
			break;
	}

	return MV_Ok;
}


/*---------------------------------------------------------------------
   Function: MV_StopPlayback

   Stops the sound playback engine.
---------------------------------------------------------------------*/

static void MV_StopPlayback(void)
{
	VoiceNode   *voice;
	VoiceNode   *next;
	uint32_t    flags;

	// Stop sound playback
	switch (MV_SoundCard)
	{
		case AHW_SOUND_BLASTER:
			BLASTER_StopPlayback();
			break;
	}

	// Make sure all callbacks are done.
	flags = DisableInterrupts();

	voice = VoiceList.start;
	while (voice != NULL)
	{
		next = voice->next;

		MV_StopVoice(voice);

		voice = next;
	}

	RestoreInterrupts(flags);
}


/*---------------------------------------------------------------------
   Function: MV_PlayRaw

   Begin playback of sound data
---------------------------------------------------------------------*/

Word MV_PlayRaw(uint8_t __far* ptr, Word length)
{
	VoiceNode *voice;

	int32_t vol      = 254;
	int32_t left     = 254;
	int32_t right    = 254;
	int32_t priority = 27;

	if (!MV_Installed)
	{
		MV_SetErrorCode(MV_NotInstalled);
		return MV_Error;
	}

	// Request a voice from the voice pool
	voice = MV_AllocVoice(priority);
	if (voice == NULL)
	{
		MV_SetErrorCode(MV_NoVoices);
		return MV_Error;
	}

	voice->Playing     = TRUE;
	voice->NextBlock   = ptr;
	voice->position    = 0;
	voice->BlockLength = length;
	voice->length      = 0;
	voice->next        = NULL;
	voice->prev        = NULL;
	voice->priority    = priority;

	MV_SetVoicePitch(voice);
	MV_SetVoiceVolume(voice, vol, left, right);
	MV_PlayVoice(voice);

	return voice->handle;
}


/*---------------------------------------------------------------------
   Function: MV_SetVolume

   Sets the volume of digitized sound playback.

   Create the table used to convert sound data to a specific volume
   level.
---------------------------------------------------------------------*/

static void MV_SetVolume(void)
{
	int32_t volume;
	int32_t val;
	int32_t i;

	for (volume = 0; volume < 128; volume++)
	{
		MV_HarshClipTable[volume]       = 0;
		MV_HarshClipTable[volume + 384] = 255;
	}
	for (volume = 0; volume < 256; volume++)
		MV_HarshClipTable[volume + 128] = volume;

	// For each volume level, create a translation table with the
	// appropriate volume calculated.
	for (volume = 0; volume <= MV_MaxVolume; volume++)
	{
		if (MV_Bits == 16)
		{
			for (i = 0; i < 65536; i += 256)
			{
				val   = i - 0x8000;
				val  *= volume;
				val  /= MV_MaxVolume;
				MV_VolumeTable[volume][i / 256] = val;
			}
		} else {
			for (i = 0; i < 256; i++)
			{
				val   = i - 0x80;
				val  *= volume;
				val  /= MV_MaxVolume;
				MV_VolumeTable[volume][i] = val;
			}
		}
	}
}


/*---------------------------------------------------------------------
   Function: MV_TestPlayback

   Checks if playback has started.
---------------------------------------------------------------------*/

static int32_t MV_TestPlayback(void)
{
	uint32_t flags;
	int32_t  time;
	int32_t  start;
	int32_t  status;

	flags = DisableInterrupts();
	_enable();

	status = MV_Error;
	start  = MV_MixPage;
	time   = clock() + CLOCKS_PER_SEC * 2;

	while (clock() < time)
	{
		if (MV_MixPage != start)
		{
			status = MV_Ok;
			break;
		}
	}

	RestoreInterrupts(flags);

	if (status != MV_Ok)
		MV_SetErrorCode(MV_DMAFailure);

	return status;
}


#if defined __WATCOMC__
static int32_t __dpmi_allocate_dos_memory(int32_t paragraphs, int32_t *selector)
{
	union REGS regs;
	regs.w.ax = 0x0100;
	regs.w.bx = paragraphs;
	int386(0x31, &regs, &regs);

	if (regs.w.cflag)
		return -1;
	else {
		*selector = regs.w.dx;
		return regs.w.ax;
	}
}


static void __dpmi_free_dos_memory(int32_t selector)
{
	union REGS regs;
	regs.w.ax = 0x0101;
	regs.w.dx = selector;
	int386(0x31, &regs, &regs);
}
#endif


/*---------------------------------------------------------------------
   Function: MV_Init

   Perform the initialization of variables and memory used by
   Multivoc.
---------------------------------------------------------------------*/

void MV_Init(int32_t soundcard, int32_t Voices)
{
	uint8_t __far* ptr;
	int32_t  segment;
	int32_t  status;
	int32_t  paragraphs;
	int32_t  index;

	if (MV_Installed)
		MV_Shutdown();

	MV_SetErrorCode(MV_Ok);

	MV_TotalMemory = Voices * sizeof(VoiceNode) + sizeof(HARSH_CLIP_TABLE_8);
	ptr = Z_TryMallocStatic(MV_TotalMemory);
	if (!ptr)
	{
		MV_SetErrorCode(MV_NoMem);
		return;
	}

	MV_Voices = (VoiceNode __far*)ptr;
	MV_HarshClipTable = ptr + (MV_TotalMemory - sizeof(HARSH_CLIP_TABLE_8));

	LL_Reset(&VoiceList);
	LL_Reset(&VoicePool);

	for (index = 0; index < Voices; index++)
		LL_AddToTail(VoiceNode, &VoicePool, &MV_Voices[index]);

	// Allocate mix buffer within 1st megabyte
	paragraphs = ((2 * TotalBufferSize) + 15) / 16;
	segment = __dpmi_allocate_dos_memory(paragraphs, &MV_BufferDescriptor);
	if (segment == -1)
	{
		Z_Free(MV_Voices);
		MV_Voices = NULL;

		MV_TotalMemory = 0;

		MV_SetErrorCode(MV_NoMem);
		return;
	} else {
		ptr = (uint8_t *)((segment << 4) + __djgpp_conventional_base);

		// Make sure we don't cross a physical page
		if (((uint32_t)ptr & 0xffff) + TotalBufferSize > 0x10000)
			ptr = (uint8_t *)(((uint32_t)ptr & 0xff0000) + 0x10000);
	}

	MV_SwapLeftRight = FALSE;

	// Initialize the sound card
	switch (soundcard)
	{
		case AHW_SOUND_BLASTER:
			MV_SwapLeftRight = BLASTER_IsSwapLeftRight();
			break;

		default:
			MV_SetErrorCode(MV_UnsupportedCard);
			break;
	}

	if (MV_ErrorCode != MV_Ok)
	{
		status = MV_ErrorCode;

		Z_Free(MV_Voices);
		MV_Voices = NULL;

		MV_TotalMemory = 0;

		__dpmi_free_dos_memory(MV_BufferDescriptor);

		MV_SetErrorCode(status);
		return;
	}

	MV_SoundCard    = soundcard;
	MV_Installed    = TRUE;

	// Set the sampling rate
	MV_RequestedMixRate = SAMPLE_RATE;

	// Set Mixer to play stereo digitized sound
	MV_SetMixMode();

	MV_MixBuffer[MV_NumberOfBuffers] = ptr;
	for (index = 0; index < MV_NumberOfBuffers; index++)
	{
		MV_MixBuffer[index] = ptr;
		ptr += MV_BufferSize;
	}

	MV_SetVolume();

	// Start the playback engine
	status = MV_StartPlayback();
	if (status != MV_Ok)
	{
		// Preserve error code while we shutdown.
		status = MV_ErrorCode;
		MV_Shutdown();
		MV_SetErrorCode(status);
		return;
	}

	if (MV_TestPlayback() != MV_Ok)
	{
		status = MV_ErrorCode;
		MV_Shutdown();
		MV_SetErrorCode(status);
	}
}


/*---------------------------------------------------------------------
   Function: MV_Shutdown

   Restore any resources allocated by Multivoc back to the system.
---------------------------------------------------------------------*/

void MV_Shutdown(void)
{
	int32_t  buffer;
	uint32_t flags;

	if (!MV_Installed)
		return;

	flags = DisableInterrupts();

	MV_KillAllVoices();

	MV_Installed = FALSE;

	// Stop the sound playback engine
	MV_StopPlayback();

	// Shutdown the sound card
	switch (MV_SoundCard)
	{
		case AHW_SOUND_BLASTER:
			BLASTER_Shutdown();
			break;
	}

	RestoreInterrupts(flags);

	// Free any voices we allocated
	if (MV_Voices)
		Z_Free(MV_Voices);

	MV_Voices = NULL;

	MV_TotalMemory = 0;

	LL_Reset(&VoiceList);
	LL_Reset(&VoicePool);

	// Release the descriptor from our mix buffer
	__dpmi_free_dos_memory(MV_BufferDescriptor);
	for (buffer = 0; buffer < NumberOfBuffers; buffer++)
		MV_MixBuffer[buffer] = NULL;
}
