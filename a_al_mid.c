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
   module: AL_MIDI.C

   author: James R. Dose
   date:   April 1, 1994

   Low level routines to support General MIDI music on Adlib compatible
   cards.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <conio.h>
#include <dos.h>
#include <string.h>

#include "i_system.h"
#include "a_al_mid.h"
#include "a_ll_man.h"


#define AL_MaxVolume             127
#define AL_DefaultChannelVolume  90
#define AL_DefaultPitchBendRange 200

#define ADLIB_PORT 0x388


#define AL_VoiceNotFound -1

/* Number of slots for the voices on the chip */
#define NumChipSlots 18

#define NUM_VOICES      9
#define NUM_CHANNELS    16

#define NOTE_ON         0x2000  /* Used to turn note on or toggle note */
#define NOTE_OFF        0x0000

#define MAX_VELOCITY   0x7f
#define MAX_OCTAVE     7
#define MAX_NOTE       (MAX_OCTAVE * 12 + 11)
#define FINETUNE_MAX   31
#define FINETUNE_RANGE (FINETUNE_MAX + 1)

#define PITCHBEND_CENTER 1638400

#define MIDI_VOLUME          7
#define MIDI_PAN             10
#define MIDI_DETUNE          94
#define MIDI_ALL_NOTES_OFF   0x7B
#define MIDI_RESET_ALL_CONTROLLERS 0x79
#define MIDI_RPN_MSB               100
#define MIDI_RPN_LSB               101
#define MIDI_DATAENTRY_MSB         6
#define MIDI_DATAENTRY_LSB         38
#define MIDI_PITCHBEND_RPN         0


typedef struct VOICE
{
	struct VOICE *next;
	struct VOICE *prev;

	unsigned num;
	unsigned key;
	unsigned velocity;
	unsigned channel;
	unsigned pitch;
	int      timbre;
	unsigned status;
} VOICE;

typedef struct
{
	VOICE *start;
	VOICE *end;
} VOICELIST;

typedef struct
{
	VOICELIST Voices;
	int       Timbre;
	int       Pitchbend;
	int       KeyOffset;
	unsigned  KeyDetune;
	unsigned  Volume;
	int       Pan;
	int       Detune;
	unsigned  RPN;
	short     PitchBendRange;
	short     PitchBendSemiTones;
	short     PitchBendHundreds;
} CHANNEL;

typedef struct
{
	unsigned char SAVEK[2];
	unsigned char Level[2];
	unsigned char Env1[2];
	unsigned char Env2[2];
	unsigned char Wave[2];
	unsigned char Feedback;
	signed   char Transpose;
	signed   char Velocity;
} TIMBRE;

static TIMBRE ADLIB_TimbreBank[256];


/* Definition of octave information to be ORed onto F-Number */

enum octaves
{
	OCTAVE_0 = 0x0000,
	OCTAVE_1 = 0x0400,
	OCTAVE_2 = 0x0800,
	OCTAVE_3 = 0x0C00,
	OCTAVE_4 = 0x1000,
	OCTAVE_5 = 0x1400,
	OCTAVE_6 = 0x1800,
	OCTAVE_7 = 0x1C00
};

static const unsigned OctavePitch[MAX_OCTAVE + 1] =
{
	OCTAVE_0, OCTAVE_1, OCTAVE_2, OCTAVE_3,
	OCTAVE_4, OCTAVE_5, OCTAVE_6, OCTAVE_7,
};

static unsigned NoteMod12[MAX_NOTE + 1];
static unsigned NoteDiv12[MAX_NOTE + 1];

// Pitch table

//static unsigned NotePitch[ FINETUNE_MAX + 1 ][ 12 ] =
//   {
//      { C, C_SHARP, D, D_SHARP, E, F, F_SHARP, G, G_SHARP, A, A_SHARP, B },
//   };

static const unsigned NotePitch[FINETUNE_MAX + 1][12] =
{
	{0x157, 0x16b, 0x181, 0x198, 0x1b0, 0x1ca, 0x1e5, 0x202, 0x220, 0x241, 0x263, 0x287},
	{0x157, 0x16b, 0x181, 0x198, 0x1b0, 0x1ca, 0x1e5, 0x202, 0x220, 0x242, 0x264, 0x288},
	{0x158, 0x16c, 0x182, 0x199, 0x1b1, 0x1cb, 0x1e6, 0x203, 0x221, 0x243, 0x265, 0x289},
	{0x158, 0x16c, 0x183, 0x19a, 0x1b2, 0x1cc, 0x1e7, 0x204, 0x222, 0x244, 0x266, 0x28a},
	{0x159, 0x16d, 0x183, 0x19a, 0x1b3, 0x1cd, 0x1e8, 0x205, 0x223, 0x245, 0x267, 0x28b},
	{0x15a, 0x16e, 0x184, 0x19b, 0x1b3, 0x1ce, 0x1e9, 0x206, 0x224, 0x246, 0x268, 0x28c},
	{0x15a, 0x16e, 0x185, 0x19c, 0x1b4, 0x1ce, 0x1ea, 0x207, 0x225, 0x247, 0x269, 0x28e},
	{0x15b, 0x16f, 0x185, 0x19d, 0x1b5, 0x1cf, 0x1eb, 0x208, 0x226, 0x248, 0x26a, 0x28f},
	{0x15b, 0x170, 0x186, 0x19d, 0x1b6, 0x1d0, 0x1ec, 0x209, 0x227, 0x249, 0x26b, 0x290},
	{0x15c, 0x170, 0x187, 0x19e, 0x1b7, 0x1d1, 0x1ec, 0x20a, 0x228, 0x24a, 0x26d, 0x291},
	{0x15d, 0x171, 0x188, 0x19f, 0x1b7, 0x1d2, 0x1ed, 0x20b, 0x229, 0x24b, 0x26e, 0x292},
	{0x15d, 0x172, 0x188, 0x1a0, 0x1b8, 0x1d3, 0x1ee, 0x20c, 0x22a, 0x24c, 0x26f, 0x293},
	{0x15e, 0x172, 0x189, 0x1a0, 0x1b9, 0x1d4, 0x1ef, 0x20d, 0x22b, 0x24d, 0x270, 0x295},
	{0x15f, 0x173, 0x18a, 0x1a1, 0x1ba, 0x1d4, 0x1f0, 0x20e, 0x22c, 0x24e, 0x271, 0x296},
	{0x15f, 0x174, 0x18a, 0x1a2, 0x1bb, 0x1d5, 0x1f1, 0x20f, 0x22d, 0x24f, 0x272, 0x297},
	{0x160, 0x174, 0x18b, 0x1a3, 0x1bb, 0x1d6, 0x1f2, 0x210, 0x22e, 0x250, 0x273, 0x298},
	{0x161, 0x175, 0x18c, 0x1a3, 0x1bc, 0x1d7, 0x1f3, 0x211, 0x22f, 0x251, 0x274, 0x299},
	{0x161, 0x176, 0x18c, 0x1a4, 0x1bd, 0x1d8, 0x1f4, 0x212, 0x230, 0x252, 0x276, 0x29b},
	{0x162, 0x176, 0x18d, 0x1a5, 0x1be, 0x1d9, 0x1f5, 0x212, 0x231, 0x254, 0x277, 0x29c},
	{0x162, 0x177, 0x18e, 0x1a6, 0x1bf, 0x1d9, 0x1f5, 0x213, 0x232, 0x255, 0x278, 0x29d},
	{0x163, 0x178, 0x18f, 0x1a6, 0x1bf, 0x1da, 0x1f6, 0x214, 0x233, 0x256, 0x279, 0x29e},
	{0x164, 0x179, 0x18f, 0x1a7, 0x1c0, 0x1db, 0x1f7, 0x215, 0x235, 0x257, 0x27a, 0x29f},
	{0x164, 0x179, 0x190, 0x1a8, 0x1c1, 0x1dc, 0x1f8, 0x216, 0x236, 0x258, 0x27b, 0x2a1},
	{0x165, 0x17a, 0x191, 0x1a9, 0x1c2, 0x1dd, 0x1f9, 0x217, 0x237, 0x259, 0x27c, 0x2a2},
	{0x166, 0x17b, 0x192, 0x1aa, 0x1c3, 0x1de, 0x1fa, 0x218, 0x238, 0x25a, 0x27e, 0x2a3},
	{0x166, 0x17b, 0x192, 0x1aa, 0x1c3, 0x1df, 0x1fb, 0x219, 0x239, 0x25b, 0x27f, 0x2a4},
	{0x167, 0x17c, 0x193, 0x1ab, 0x1c4, 0x1e0, 0x1fc, 0x21a, 0x23a, 0x25c, 0x280, 0x2a6},
	{0x168, 0x17d, 0x194, 0x1ac, 0x1c5, 0x1e0, 0x1fd, 0x21b, 0x23b, 0x25d, 0x281, 0x2a7},
	{0x168, 0x17d, 0x194, 0x1ad, 0x1c6, 0x1e1, 0x1fe, 0x21c, 0x23c, 0x25e, 0x282, 0x2a8},
	{0x169, 0x17e, 0x195, 0x1ad, 0x1c7, 0x1e2, 0x1ff, 0x21d, 0x23d, 0x260, 0x283, 0x2a9},
	{0x16a, 0x17f, 0x196, 0x1ae, 0x1c8, 0x1e3, 0x1ff, 0x21e, 0x23e, 0x261, 0x284, 0x2ab},
	{0x16a, 0x17f, 0x197, 0x1af, 0x1c8, 0x1e4, 0x200, 0x21f, 0x23f, 0x262, 0x286, 0x2ac}
};

// Slot numbers as a function of the voice and the operator.
// ( melodic only)

static const int slotVoice[NUM_VOICES][2] =
{
	{ 0,  3},  // voice 0
	{ 1,  4},  // 1
	{ 2,  5},  // 2
	{ 6,  9},  // 3
	{ 7, 10},  // 4
	{ 8, 11},  // 5
	{12, 15},  // 6
	{13, 16},  // 7
	{14, 17},  // 8
};

static int VoiceLevel[NumChipSlots];
static int VoiceKsl[NumChipSlots];

// This table gives the offset of each slot within the chip.
// offset = fn( slot)

static const char offsetSlot[NumChipSlots] =
{
	 0,  1,  2,  3,  4,  5,
	 8,  9, 10, 11, 12, 13,
	16, 17, 18, 19, 20, 21
};

static VOICE     Voice[NUM_VOICES * 2];
static VOICELIST Voice_Pool;

static volatile CHANNEL   Channel[NUM_CHANNELS];

#define AL_MaxMidiChannel 16


/*---------------------------------------------------------------------
   Function: AL_SendOutput

   Sends data to the Adlib.
---------------------------------------------------------------------*/

static void AL_SendOutput(int reg, int data)
{
	int delay;

	outp(ADLIB_PORT, reg);

	for (delay = 6; delay > 0; delay--)
		inp(ADLIB_PORT);

	outp(ADLIB_PORT + 1, data);

	for (delay = 27; delay > 0; delay--)
		inp(ADLIB_PORT);
}


/*---------------------------------------------------------------------
   Function: AL_SetVoiceTimbre

   Programs the specified voice's timbre.
---------------------------------------------------------------------*/

static void AL_SetVoiceTimbre(int voice)
{
	int    off;
	int    slot;
	int    voc;
	int    patch;
	int    channel;
	TIMBRE *timbre;

	channel = Voice[voice].channel;

	if (channel == 9)
		patch = Voice[voice].key + 128;
	else
		patch = Channel[channel].Timbre;

	if (Voice[voice].timbre == patch)
		return;

	Voice[voice].timbre = patch;
	timbre = &ADLIB_TimbreBank[patch];

	voc  = (voice >= NUM_VOICES) ? voice - NUM_VOICES : voice;
	slot = slotVoice[voc][0];
	off  = offsetSlot[slot];

	VoiceLevel[slot] = 63 - (timbre->Level[0] & 0x3f);
	VoiceKsl[slot]   = timbre->Level[0] & 0xc0;

	AL_SendOutput(0xA0 + voc, 0);
	AL_SendOutput(0xB0 + voc, 0);

	// Let voice clear the release
	AL_SendOutput(0x80 + off, 0xff);

	AL_SendOutput(0x60 + off, timbre->Env1[0]);
	AL_SendOutput(0x80 + off, timbre->Env2[0]);
	AL_SendOutput(0x20 + off, timbre->SAVEK[0]);
	AL_SendOutput(0xE0 + off, timbre->Wave[0]);

	AL_SendOutput(0x40 + off, timbre->Level[0]);
	slot = slotVoice[voc][1];

	AL_SendOutput(0xC0 + voice, timbre->Feedback);

	off = offsetSlot[slot];

	VoiceLevel[slot] = 63 - (timbre->Level[1] & 0x3f);
	VoiceKsl[slot]   = timbre->Level[1] & 0xc0;
	AL_SendOutput(0x40 + off, 63);

	// Let voice clear the release
	AL_SendOutput(0x80 + off, 0xff);

	AL_SendOutput(0x60 + off, timbre->Env1[1]);
	AL_SendOutput(0x80 + off, timbre->Env2[1]);
	AL_SendOutput(0x20 + off, timbre->SAVEK[1]);
	AL_SendOutput(0xE0 + off, timbre->Wave[1]);
}


/*---------------------------------------------------------------------
   Function: AL_SetVoiceVolume

   Sets the volume of the specified voice.
---------------------------------------------------------------------*/

static void AL_SetVoiceVolume(int voice)
{
	int channel;
	int velocity;
	int slot;
	int voc;
	unsigned long t1;
	unsigned long volume;
	TIMBRE *timbre;

	channel = Voice[voice].channel;

	timbre = &ADLIB_TimbreBank[Voice[voice].timbre];

	velocity = Voice[voice].velocity + timbre->Velocity;
	if (velocity > MAX_VELOCITY)
		velocity = MAX_VELOCITY;

	voc  = (voice >= NUM_VOICES) ? voice - NUM_VOICES : voice;
	slot = slotVoice[voc][1];

	// amplitude
	t1  = (unsigned)VoiceLevel[slot];
	t1 *= (velocity + 0x80);
	t1  = (Channel[channel].Volume * t1) >> 15;

	volume  = t1 ^ 63;
	volume |= (unsigned)VoiceKsl[slot];

	AL_SendOutput(0x40 + offsetSlot[slot], volume);

	// Check if this timbre is Additive
	if (timbre->Feedback & 0x01)
	{
		slot = slotVoice[voc][0];

		// amplitude
		t1  = (unsigned)VoiceLevel[slot];
		t1 *= (velocity + 0x80);
		t1  = (Channel[channel].Volume * t1) >> 15;

		volume  = t1 ^ 63;
		volume |= (unsigned)VoiceKsl[slot];

		AL_SendOutput(0x40 + offsetSlot[slot], volume);
	}
}


/*---------------------------------------------------------------------
   Function: AL_AllocVoice

   Retrieves a free voice from the voice pool.
---------------------------------------------------------------------*/

static int AL_AllocVoice(void)
{
	int voice;

	if (Voice_Pool.start)
	{
		voice = Voice_Pool.start->num;
		LL_Remove(VOICE, &Voice_Pool, &Voice[voice]);
		return voice;
	} else
		return AL_VoiceNotFound;
}


/*---------------------------------------------------------------------
   Function: AL_GetVoice

   Determines which voice is associated with a specified note and
   MIDI channel.
---------------------------------------------------------------------*/

static int AL_GetVoice(int channel, int key)
{
	VOICE *voice;

	voice = Channel[channel].Voices.start;

	while (voice != NULL)
	{
		if (voice->key == key)
			return voice->num;

		voice = voice->next;
	}

	return AL_VoiceNotFound;
}


/*---------------------------------------------------------------------
   Function: AL_SetVoicePitch

   Programs the pitch of the specified voice.
---------------------------------------------------------------------*/

static void AL_SetVoicePitch(int voice)
{
	int note;
	int channel;
	int patch;
	int detune;
	int ScaleNote;
	int Octave;
	int pitch;
	int voc;

	voc  = (voice >= NUM_VOICES) ? voice - NUM_VOICES : voice;
	channel = Voice[voice].channel;

	if (channel == 9)
	{
		patch = Voice[voice].key + 128;
		note  = ADLIB_TimbreBank[patch].Transpose;
	} else {
		patch = Channel[channel].Timbre;
		note  = Voice[voice].key + ADLIB_TimbreBank[patch].Transpose;
	}

	note += Channel[channel].KeyOffset - 12;

	if (note > MAX_NOTE)
		note = MAX_NOTE;
	else if (note < 0)
		note = 0;

	detune = Channel[channel].KeyDetune;

	ScaleNote = NoteMod12[note];
	Octave    = NoteDiv12[note];

	pitch = OctavePitch[Octave] | NotePitch[detune][ScaleNote];

	Voice[voice].pitch = pitch;

	pitch |= Voice[voice].status;

	AL_SendOutput(0xA0 + voc, pitch);
	AL_SendOutput(0xB0 + voc, pitch >> 8);
}


/*---------------------------------------------------------------------
   Function: AL_SetChannelVolume

   Sets the volume of the specified MIDI channel.
---------------------------------------------------------------------*/

static void AL_SetChannelVolume(int channel, int volume)
{
	VOICE *voice;

	if (volume < 0)
		volume = 0;
	else if (volume > AL_MaxVolume)
		volume = AL_MaxVolume;

	Channel[channel].Volume = volume;

	voice = Channel[channel].Voices.start;
	while (voice != NULL)
	{
		AL_SetVoiceVolume(voice->num);
		voice = voice->next;
	}
}


/*---------------------------------------------------------------------
   Function: AL_SetChannelPan

   Sets the pan position of the specified MIDI channel.
---------------------------------------------------------------------*/

static void AL_SetChannelPan(int channel, int pan)
{
	// Don't pan drum sounds
	if (channel != 9)
		Channel[channel].Pan = pan;
}


/*---------------------------------------------------------------------
   Function: AL_SetChannelDetune

   Sets the stereo voice detune of the specified MIDI channel.
---------------------------------------------------------------------*/

static void AL_SetChannelDetune(int channel, int detune)
{
	Channel[channel].Detune = detune;
}


/*---------------------------------------------------------------------
   Function: AL_ResetVoices

   Sets all voice info to the default state.
---------------------------------------------------------------------*/

static void AL_ResetVoices(void)
{
	int index;

	Voice_Pool.start = NULL;
	Voice_Pool.end = NULL;

	for (index = 0; index < NUM_VOICES; index++)
	{
		Voice[index].num = index;
		Voice[index].key = 0;
		Voice[index].velocity = 0;
		Voice[index].channel = -1;
		Voice[index].timbre = -1;
		Voice[index].status = NOTE_OFF;
		LL_AddToTail(VOICE, &Voice_Pool, &Voice[index]);
	}

	for (index = 0; index < NUM_CHANNELS; index++)
	{
		Channel[index].Voices.start    = NULL;
		Channel[index].Voices.end      = NULL;
		Channel[index].Timbre          = 0;
		Channel[index].Pitchbend       = 0;
		Channel[index].KeyOffset       = 0;
		Channel[index].KeyDetune       = 0;
		Channel[index].Volume          = AL_DefaultChannelVolume;
		Channel[index].Pan             = 64;
		Channel[index].RPN             = 0;
		Channel[index].PitchBendRange     = AL_DefaultPitchBendRange;
		Channel[index].PitchBendSemiTones = AL_DefaultPitchBendRange / 100;
		Channel[index].PitchBendHundreds  = AL_DefaultPitchBendRange % 100;
	}
}


/*---------------------------------------------------------------------
   Function: AL_CalcPitchInfo

   Calculates the pitch table.
---------------------------------------------------------------------*/

static void AL_CalcPitchInfo(void)
{
	int    note;

	for (note = 0; note <= MAX_NOTE; note++)
	{
		NoteMod12[note] = note % 12;
		NoteDiv12[note] = note / 12;
	}
}


/*---------------------------------------------------------------------
   Function: AL_FlushCard

   Sets all voices to a known (quiet) state.
---------------------------------------------------------------------*/

static void AL_FlushCard(void)
{
	int i;
	unsigned slot1;
	unsigned slot2;

	for (i = 0; i < NUM_VOICES; i++)
	{
		slot1 = offsetSlot[slotVoice[i][0]];
		slot2 = offsetSlot[slotVoice[i][1]];

		AL_SendOutput(0xA0 + i, 0);
		AL_SendOutput(0xB0 + i, 0);

		AL_SendOutput(0xE0 + slot1, 0);
		AL_SendOutput(0xE0 + slot2, 0);

		// Set the envelope to be fast and quiet
		AL_SendOutput(0x60 + slot1, 0xff);
		AL_SendOutput(0x60 + slot2, 0xff);
		AL_SendOutput(0x80 + slot1, 0xff);
		AL_SendOutput(0x80 + slot2, 0xff);

		// Maximum attenuation
		AL_SendOutput(0x40 + slot1, 0xff);
		AL_SendOutput(0x40 + slot2, 0xff);
	}
}


/*---------------------------------------------------------------------
   Function: AL_Reset

   Sets the card to a known (quiet) state.
---------------------------------------------------------------------*/

static void AL_Reset(void)
{
	AL_SendOutput(1, 0x20);
	AL_SendOutput(0x08, 0);

	// Set the values: AM Depth, VIB depth & Rhythm
	AL_SendOutput(0xBD, 0);

	AL_FlushCard();
}


/*---------------------------------------------------------------------
   Function: AL_NoteOff

   Turns off a note on the specified MIDI channel.
---------------------------------------------------------------------*/

void AL_NoteOff(int32_t channel, int32_t key, int32_t velocity)
{
	int voice;
	int voc;

	UNUSED(velocity);

	// We only play channels 1 through 10
	if (channel > AL_MaxMidiChannel)
		return;

	voice = AL_GetVoice(channel, key);

	if (voice == AL_VoiceNotFound)
		return;

	Voice[voice].status = NOTE_OFF;

	voc = (voice >= NUM_VOICES) ? voice - NUM_VOICES : voice;

	AL_SendOutput(0xB0 + voc, HIBYTE(Voice[voice].pitch));

	LL_Remove(VOICE, &Channel[channel].Voices, &Voice[voice]);
	LL_AddToTail(VOICE, &Voice_Pool, &Voice[voice]);
}


/*---------------------------------------------------------------------
   Function: AL_NoteOn

   Plays a note on the specified MIDI channel.
---------------------------------------------------------------------*/

void AL_NoteOn(int32_t channel, int32_t key, int32_t velocity)
{
	int voice;

	// We only play channels 1 through 10
	if (channel > AL_MaxMidiChannel)
		return;

	if (velocity == 0)
	{
		AL_NoteOff(channel, key, 0);
		return;
	}

	voice = AL_AllocVoice();

	if (voice == AL_VoiceNotFound)
	{
		if (Channel[9].Voices.start)
		{
			AL_NoteOff(9, Channel[9].Voices.start->key, 0);
			voice = AL_AllocVoice();
		}
		if (voice == AL_VoiceNotFound)
			return;
	}

	Voice[voice].key      = key;
	Voice[voice].channel  = channel;
	Voice[voice].velocity = velocity;
	Voice[voice].status   = NOTE_ON;

	LL_AddToTail(VOICE, &Channel[channel].Voices, &Voice[voice]);

	AL_SetVoiceTimbre(voice);
	AL_SetVoiceVolume(voice);
	AL_SetVoicePitch(voice);
}


/*---------------------------------------------------------------------
   Function: AL_AllNotesOff

   Turns off all currently playing voices.
---------------------------------------------------------------------*/

static void AL_AllNotesOff(int channel)
{
	while (Channel[channel].Voices.start != NULL)
		AL_NoteOff(channel, Channel[channel].Voices.start->key, 0);
}


/*---------------------------------------------------------------------
   Function: AL_ControlChange

   Sets the value of a controller on the specified MIDI channel.
---------------------------------------------------------------------*/

void AL_ControlChange(int32_t channel, int32_t type, int32_t data)
{
	// We only play channels 1 through 10
	if (channel > AL_MaxMidiChannel)
		return;

	switch (type)
	{
		case MIDI_VOLUME:
			AL_SetChannelVolume(channel, data);
			break;

		case MIDI_PAN:
			AL_SetChannelPan(channel, data);
			break;

		case MIDI_DETUNE:
			AL_SetChannelDetune(channel, data);
			break;

		case MIDI_ALL_NOTES_OFF:
			AL_AllNotesOff(channel);
			break;

		case MIDI_RESET_ALL_CONTROLLERS:
			AL_ResetVoices();
			AL_SetChannelVolume(channel, AL_DefaultChannelVolume);
			AL_SetChannelPan(channel, 64);
			AL_SetChannelDetune(channel, 0);
			break;

		case MIDI_RPN_MSB:
			Channel[channel].RPN &= 0x00FF;
			Channel[channel].RPN |= (data & 0xFF) << 8;
			break;

		case MIDI_RPN_LSB:
			Channel[channel].RPN &= 0xFF00;
			Channel[channel].RPN |= data & 0xFF;
			break;

		case MIDI_DATAENTRY_MSB:
			if (Channel[channel].RPN == MIDI_PITCHBEND_RPN)
			{
				Channel[channel].PitchBendSemiTones = data;
				Channel[channel].PitchBendRange     =
					Channel[channel].PitchBendSemiTones * 100 +
					Channel[channel].PitchBendHundreds;
			}
			break;

		case MIDI_DATAENTRY_LSB:
			if ( Channel[ channel ].RPN == MIDI_PITCHBEND_RPN )
			{
				Channel[ channel ].PitchBendHundreds = data;
				Channel[ channel ].PitchBendRange    =
					Channel[ channel ].PitchBendSemiTones * 100 +
					Channel[ channel ].PitchBendHundreds;
			}
			break;
	}
}


/*---------------------------------------------------------------------
   Function: AL_ProgramChange

   Selects the instrument to use on the specified MIDI channel.
---------------------------------------------------------------------*/

void AL_ProgramChange(int32_t channel, int32_t patch)
{
	// We only play channels 1 through 10
	if (channel > AL_MaxMidiChannel)
		return;

	Channel[channel].Timbre  = patch;
}


/*---------------------------------------------------------------------
   Function: AL_SetPitchBend

   Sets the pitch bend amount on the specified MIDI channel.
---------------------------------------------------------------------*/

void AL_SetPitchBend(int32_t channel, int32_t lsb, int32_t msb)
{
	int            pitchbend;
	unsigned long  TotalBend;
	VOICE         *voice;

	// We only play channels 1 through 10
	if (channel > AL_MaxMidiChannel)
		return;

	pitchbend = lsb + (msb << 8);
	Channel[channel].Pitchbend = pitchbend;

	TotalBend  = pitchbend * Channel[channel].PitchBendRange;
	TotalBend /= (PITCHBEND_CENTER / FINETUNE_RANGE);

	Channel[channel].KeyOffset  = (int)(TotalBend / FINETUNE_RANGE);
	Channel[channel].KeyOffset -= Channel[channel].PitchBendSemiTones;

	Channel[channel].KeyDetune = (unsigned)(TotalBend % FINETUNE_RANGE);

	voice = Channel[channel].Voices.start;
	while (voice != NULL)
	{
		AL_SetVoicePitch(voice->num);
		voice = voice->next;
	}
}


/*---------------------------------------------------------------------
   Function: AL_DetectFM

   Determines if an Adlib compatible card is installed in the machine.
---------------------------------------------------------------------*/

Boolean AL_DetectFM(void)
{
	int status1;
	int status2;
	int delay;

	AL_SendOutput(4, 0x60);   // Reset T1 & T2
	AL_SendOutput(4, 0x80);   // Reset IRQ

	status1 = inp(ADLIB_PORT);

	AL_SendOutput(2, 0xff);   // Set timer 1
	AL_SendOutput(4, 0x21);   // Start timer 1

	for (delay = 200; delay > 0; delay--)
		inp(ADLIB_PORT);      // Delay for at least 80 microseconds.

	status2 = inp(ADLIB_PORT);

	AL_SendOutput(4, 0x60);   // Reset T1 & T2
	AL_SendOutput(4, 0x80);   // Reset IRQ

	return ((status1 & 0xe0) == 0x00) && ((status2 & 0xe0) == 0xc0);
}


/*---------------------------------------------------------------------
   Function: AL_Shutdown

   Ends use of the sound card and resets it to a quiet state.
---------------------------------------------------------------------*/

void AL_Shutdown(void)
{
	AL_ResetVoices();
	AL_Reset();
}


/*---------------------------------------------------------------------
   Function: AL_Init

   Begins use of the sound card.
---------------------------------------------------------------------*/

void AL_Init(void)
{
	AL_CalcPitchInfo();
	AL_Reset();
	AL_ResetVoices();
}


/*---------------------------------------------------------------------
   Function: AL_RegisterTimbreBank

   Copies user supplied timbres over the default timbre bank.
---------------------------------------------------------------------*/

void AL_RegisterTimbreBank(uint8_t __far* timbres)
{
	_fmemcpy(ADLIB_TimbreBank, timbres, sizeof(ADLIB_TimbreBank));
}
