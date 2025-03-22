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
   module: BLASTER.C

   author: James R. Dose
   date:   February 4, 1994

   Low level routines to support Sound Blaster, Sound Blaster Pro,
   Sound Blaster 16, and compatible sound cards.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <conio.h>
#include <dos.h>
#include <ctype.h>

#include "i_system.h"
#include "a_blast.h"
#include "a_dma.h"

#define VALID   ( 1 == 1 )
#define INVALID ( !VALID )

#define BLASTER_MixerAddressPort  0x04
#define BLASTER_MixerDataPort     0x05
#define BLASTER_ResetPort         0x06
#define BLASTER_ReadPort          0x0A
#define BLASTER_WritePort         0x0C
#define BLASTER_DataAvailablePort 0x0E
#define BLASTER_Ready             0xAA
#define BLASTER_16BitDMAAck       0x0F

#define MIXER_DSP4xxISR_Ack       0x82
#define MIXER_DSP4xxISR_Enable    0x83
#define MIXER_16BITDMA_INT        0x2
#define MIXER_8BITDMA_INT         0x1
#define MIXER_DisableMPU401Interrupts 0xB
#define MIXER_SBProOutputSetting  0x0E
#define MIXER_SBProStereoFlag     0x02
#define MIXER_SBProVoice          0x04
#define MIXER_SB16VoiceLeft       0x32
#define MIXER_SB16VoiceRight      0x33

#define DSP_Version1xx            0x0100
#define DSP_Version2xx            0x0200
#define DSP_Version201            0x0201
#define DSP_Version3xx            0x0300
#define DSP_Version4xx            0x0400

#define DSP_8BitAutoInitMode          0x1c
#define DSP_8BitHighSpeedAutoInitMode 0x90
#define DSP_SetBlockLength            0x48
#define DSP_Old8BitDAC                0x14
#define DSP_16BitDAC                  0xB6
#define DSP_8BitDAC                   0xC6
#define DSP_SetTimeConstant           0x40
#define DSP_Set_DA_Rate               0x41
#define DSP_Halt8bitTransfer          0xd0
#define DSP_Halt16bitTransfer         0xd5
#define DSP_SpeakerOn                 0xd1
#define DSP_SpeakerOff                0xd3
#define DSP_GetVersion                0xE1
#define DSP_Reset                     0xFFFF


#if defined __DJGPP__
static _go32_dpmi_seginfo BLASTER_OldInt, BLASTER_NewInt;
#elif defined __WATCOMC__
static void (__interrupt *BLASTER_OldInt)(void);
#endif


enum BLASTER_Types
{
	SB     = 1,
	SBPro  = 2,
	SB20   = 3,
	SB16   = 6
};

typedef struct
{
	uint32_t Address;
	uint32_t Interrupt;
	uint32_t Dma8;
	uint32_t Dma16;
	uint32_t Type;

	uint32_t MinSamplingRate;
	uint32_t MaxSamplingRate;
	uint32_t MaxMixMode;
} BLASTER_CONFIG;

#define UNDEFINED -1

static BLASTER_CONFIG BLASTER_Config =
{
	UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED, UNDEFINED,
	UNDEFINED, UNDEFINED, UNDEFINED
};

static Boolean BLASTER_Installed = FALSE;
static int32_t BLASTER_Version;

static uint8_t *BLASTER_DMABuffer;
static uint8_t *BLASTER_DMABufferEnd;
static uint8_t *BLASTER_CurrentDMABuffer;
static int32_t  BLASTER_TotalDMABufferSize;

#define BLASTER_DefaultSampleRate 11000
#define BLASTER_DefaultMixMode    MONO_8BIT

#define MONO_8BIT_SAMPLE_SIZE    1
#define MONO_16BIT_SAMPLE_SIZE   2
#define STEREO_8BIT_SAMPLE_SIZE  ( 2 * MONO_8BIT_SAMPLE_SIZE )
#define STEREO_16BIT_SAMPLE_SIZE ( 2 * MONO_16BIT_SAMPLE_SIZE )

static  int32_t BLASTER_TransferLength   = 0;
static  int32_t BLASTER_MixMode          = BLASTER_DefaultMixMode;
static uint32_t BLASTER_SamplePacketSize = MONO_16BIT_SAMPLE_SIZE;
static uint32_t BLASTER_SampleRate       = BLASTER_DefaultSampleRate;

static uint32_t BLASTER_HaltTransferCommand = DSP_Halt8bitTransfer;

static volatile Boolean BLASTER_SoundPlaying;

static void ( *BLASTER_CallBack )( void );

static int32_t  BLASTER_IntController1Mask;
static int32_t  BLASTER_IntController2Mask;

static int32_t BLASTER_OriginalVoiceVolumeLeft  = 255;
static int32_t BLASTER_OriginalVoiceVolumeRight = 255;

static int32_t BLASTER_DMAChannel;

static void BLASTER_DSP1xx_BeginPlayback(int32_t length);

/*---------------------------------------------------------------------
   Function: BLASTER_EnableInterrupt

   Enables the triggering of the sound card interrupt.
---------------------------------------------------------------------*/

static void BLASTER_EnableInterrupt(void)
{
	// Unmask system interrupt
	int32_t mask = inp(0x21) & ~(1 << BLASTER_Config.Interrupt);
	outp(0x21, mask);
}


/*---------------------------------------------------------------------
   Function: BLASTER_DisableInterrupt

   Disables the triggering of the sound card interrupt.
---------------------------------------------------------------------*/

static void BLASTER_DisableInterrupt(void)
{
	int32_t mask;

	// Restore interrupt mask
	mask  = inp(0x21) & ~(1 << BLASTER_Config.Interrupt);
	mask |= BLASTER_IntController1Mask & (1 << BLASTER_Config.Interrupt);
	outp(0x21, mask);
}


/*---------------------------------------------------------------------
   Function: BLASTER_ServiceInterrupt

   Handles interrupt generated by sound card at the end of a voice
   transfer.  Calls the user supplied callback function.
---------------------------------------------------------------------*/

// This is defined because we can't create local variables in a
// function that switches stacks.
static int32_t GlobalStatus;

static void __interrupt BLASTER_ServiceInterrupt(void)
{
	// Acknowledge interrupt
	// Check if this is this an SB16 or newer
	if ( BLASTER_Version >= DSP_Version4xx )
	{
		outp( BLASTER_Config.Address + BLASTER_MixerAddressPort, MIXER_DSP4xxISR_Ack );

		GlobalStatus = inp( BLASTER_Config.Address + BLASTER_MixerDataPort );

		// Check if a 16-bit DMA interrupt occurred
		if ( GlobalStatus & MIXER_16BITDMA_INT )
		{
			// Acknowledge 16-bit transfer interrupt
			inp( BLASTER_Config.Address + BLASTER_16BitDMAAck );
		} else if ( GlobalStatus & MIXER_8BITDMA_INT ) {
			inp( BLASTER_Config.Address + BLASTER_DataAvailablePort );
		} else {
			// Wasn't our interrupt.  Call the old one.
			_chain_intr(BLASTER_OldInt);
		}
	} else {
		// Older card - can't detect if an interrupt occurred.
		inp( BLASTER_Config.Address + BLASTER_DataAvailablePort );
	}

	// Keep track of current buffer
	BLASTER_CurrentDMABuffer += BLASTER_TransferLength;

	if ( BLASTER_CurrentDMABuffer >= BLASTER_DMABufferEnd )
		BLASTER_CurrentDMABuffer = BLASTER_DMABuffer;

	// Continue playback on cards without autoinit mode
	if ( BLASTER_Version < DSP_Version2xx )
	{
		if ( BLASTER_SoundPlaying )
			BLASTER_DSP1xx_BeginPlayback( BLASTER_TransferLength );
	}

	// Call the caller's callback function
	if ( BLASTER_CallBack != NULL )
		BLASTER_CallBack();

	// send EOI to Interrupt Controller
	outp( 0x20, 0x20 );
}


/*---------------------------------------------------------------------
   Function: BLASTER_WriteDSP

   Writes a byte of data to the sound card's DSP.
---------------------------------------------------------------------*/

static void BLASTER_WriteDSP(uint32_t data)
{
	int32_t port = BLASTER_Config.Address + BLASTER_WritePort;

	uint32_t count = 0xFFFF;

	do
	{
		if ((inp(port) & 0x80) == 0)
		{
			outp(port, data);
			break;
		}

		count--;
	} while (count > 0);
}


/*---------------------------------------------------------------------
   Function: BLASTER_ReadDSP

   Reads a byte of data from the sound card's DSP.
---------------------------------------------------------------------*/

static int32_t BLASTER_ReadDSP(void)
{
	int32_t port = BLASTER_Config.Address + BLASTER_DataAvailablePort;

	int32_t status = BLASTER_Error;

	uint32_t count = 0xFFFF;

	do
	{
		if (inp(port) & 0x80)
		{
			status = inp(BLASTER_Config.Address + BLASTER_ReadPort);
			break;
		}

		count--;
	} while( count > 0 );

	return status;
}


/*---------------------------------------------------------------------
   Function: BLASTER_ResetDSP

   Sends a reset command to the sound card's Digital Signal Processor
   (DSP), causing it to perform an initialization.
---------------------------------------------------------------------*/

static int32_t BLASTER_ResetDSP(void)
{
	volatile int32_t count;

	int32_t port = BLASTER_Config.Address + BLASTER_ResetPort;

	int32_t status = BLASTER_Error;

	outp(port, 1);

	count = 0x100;
	do
	{
		count--;
	} while (count > 0);

	outp(port, 0);

	count = 100;
	do
	{
		if (BLASTER_ReadDSP() == BLASTER_Ready)
		{
			status = BLASTER_Ok;
			break;
		}

		count--;
	} while (count > 0);

	return status;
}


/*---------------------------------------------------------------------
   Function: BLASTER_GetDSPVersion

   Returns the version number of the sound card's DSP.
---------------------------------------------------------------------*/

static int32_t BLASTER_GetDSPVersion(void)
{
	int32_t MajorVersion;
	int32_t MinorVersion;

	BLASTER_WriteDSP(DSP_GetVersion);

	MajorVersion = BLASTER_ReadDSP();
	MinorVersion = BLASTER_ReadDSP();

	if ((MajorVersion == BLASTER_Error) || (MinorVersion == BLASTER_Error))
		return BLASTER_Error;
	else
		return (MajorVersion << 8) + MinorVersion;
}


/*---------------------------------------------------------------------
   Function: BLASTER_SetPlaybackRate

   Sets the rate at which the digitized sound will be played in
   hertz.
---------------------------------------------------------------------*/

#define CalcTimeConstant( rate, samplesize ) \
   ( ( 65536L - ( 256000000L / ( ( samplesize ) * ( rate ) ) ) ) >> 8 )

#define CalcSamplingRate( tc ) \
   ( 256000000L / ( 65536L - ( tc << 8 ) ) )

static void BLASTER_SetPlaybackRate(uint32_t rate)
{
	if (BLASTER_Version < DSP_Version4xx)
	{
		// Send sampling rate as time constant for older Sound
		// Blaster compatible cards.

		 int32_t timeconstant;
		uint32_t ActualRate;

		ActualRate = rate * BLASTER_SamplePacketSize;
		if (ActualRate < BLASTER_Config.MinSamplingRate)
			rate = BLASTER_Config.MinSamplingRate / BLASTER_SamplePacketSize;

		if (ActualRate > BLASTER_Config.MaxSamplingRate)
			rate = BLASTER_Config.MaxSamplingRate / BLASTER_SamplePacketSize;

		timeconstant = (int32_t)CalcTimeConstant(rate, BLASTER_SamplePacketSize);

		// Keep track of what the actual rate is
		BLASTER_SampleRate  = (uint32_t)CalcSamplingRate(timeconstant);
		BLASTER_SampleRate /= BLASTER_SamplePacketSize;

		BLASTER_WriteDSP(DSP_SetTimeConstant);
		BLASTER_WriteDSP(timeconstant);
	} else {
		// Send literal sampling rate for cards with DSP version
		// 4.xx (Sound Blaster 16)

		int32_t LoByte;
		int32_t HiByte;

		BLASTER_SampleRate = rate;

		if (BLASTER_SampleRate < BLASTER_Config.MinSamplingRate)
			BLASTER_SampleRate = BLASTER_Config.MinSamplingRate;

		if (BLASTER_SampleRate > BLASTER_Config.MaxSamplingRate)
			BLASTER_SampleRate = BLASTER_Config.MaxSamplingRate;

		HiByte = HIBYTE(BLASTER_SampleRate);
		LoByte = LOBYTE(BLASTER_SampleRate);

		// Set playback rate
		BLASTER_WriteDSP(DSP_Set_DA_Rate);
		BLASTER_WriteDSP(HiByte);
		BLASTER_WriteDSP(LoByte);
	}
}


/*---------------------------------------------------------------------
   Function: BLASTER_GetPlaybackRate

   Returns the rate at which the digitized sound will be played in
   hertz.
---------------------------------------------------------------------*/

uint32_t BLASTER_GetPlaybackRate(void)
{
	return BLASTER_SampleRate;
}


int32_t BLASTER_GetDMAChannel(void)
{
	return BLASTER_DMAChannel;
}


/*---------------------------------------------------------------------
   Function: BLASTER_SetMixMode

   Sets the sound card to play samples in mono or stereo.
---------------------------------------------------------------------*/

#define BLASTER_MaxMixMode        STEREO_16BIT

static const uint32_t BLASTER_SampleSize[ BLASTER_MaxMixMode + 1 ] =
{
	MONO_8BIT_SAMPLE_SIZE,  STEREO_8BIT_SAMPLE_SIZE,
	MONO_16BIT_SAMPLE_SIZE, STEREO_16BIT_SAMPLE_SIZE
};

int32_t BLASTER_SetMixMode(int32_t mode)
{
	mode &= BLASTER_MaxMixMode;

	if (!(BLASTER_Config.MaxMixMode & STEREO))
		mode &= ~STEREO;

	if (!(BLASTER_Config.MaxMixMode & SIXTEEN_BIT))
		mode &= ~SIXTEEN_BIT;

	BLASTER_MixMode = mode;
	BLASTER_SamplePacketSize = BLASTER_SampleSize[mode];

	// For the Sound Blaster Pro, we have to set the mixer chip
	// to play mono or stereo samples.

	if (BLASTER_Config.Type == SBPro)
	{
		int32_t data;
		int32_t	port = BLASTER_Config.Address + BLASTER_MixerAddressPort;
		outp(port, MIXER_SBProOutputSetting);

		port = BLASTER_Config.Address + BLASTER_MixerDataPort;

		// Get current mode
		data = inp(port);

		// set stereo mode bit
		if (mode & STEREO)
			data |= MIXER_SBProStereoFlag;
		else
			data &= ~MIXER_SBProStereoFlag;

		// set the mode
		outp(port, data);

		BLASTER_SetPlaybackRate(BLASTER_SampleRate);
	}

	return mode;
}


/*---------------------------------------------------------------------
   Function: BLASTER_StopPlayback

   Ends the DMA transfer of digitized sound to the sound card.
---------------------------------------------------------------------*/

void BLASTER_StopPlayback(void)
{
	// Don't allow anymore interrupts
	BLASTER_DisableInterrupt();

	if (BLASTER_HaltTransferCommand == DSP_Reset)
		BLASTER_ResetDSP();
	else
		BLASTER_WriteDSP(BLASTER_HaltTransferCommand);

	// Disable the DMA channel
	DMA_EndTransfer(BLASTER_DMAChannel);

	// Turn off speaker
	BLASTER_WriteDSP(DSP_SpeakerOff);

	BLASTER_SoundPlaying = FALSE;

	BLASTER_DMABuffer = NULL;
}


/*---------------------------------------------------------------------
   Function: BLASTER_SetupDMABuffer

   Programs the DMAC for sound transfer.
---------------------------------------------------------------------*/

static int32_t BLASTER_SetupDMABuffer(uint8_t *BufferPtr, int32_t BufferSize)
{
	int32_t DmaStatus = DMA_SetupTransfer(BLASTER_DMAChannel, BufferPtr, BufferSize);
	if (DmaStatus != DMA_Ok)
		return BLASTER_Error;

	BLASTER_DMABuffer          = BufferPtr;
	BLASTER_CurrentDMABuffer   = BufferPtr;
	BLASTER_TotalDMABufferSize = BufferSize;
	BLASTER_DMABufferEnd       = BufferPtr + BufferSize;

	return BLASTER_Ok;
}


/*---------------------------------------------------------------------
   Function: BLASTER_DSP1xx_BeginPlayback

   Starts playback of digitized sound on cards compatible with DSP
   version 1.xx.
---------------------------------------------------------------------*/

static void BLASTER_DSP1xx_BeginPlayback(int32_t length)
{
	int32_t SampleLength = length - 1;
	int32_t HiByte = HIBYTE( SampleLength );
	int32_t LoByte = LOBYTE( SampleLength );

	// Program DSP to play sound
	BLASTER_WriteDSP( DSP_Old8BitDAC );
	BLASTER_WriteDSP( LoByte );
	BLASTER_WriteDSP( HiByte );

	BLASTER_HaltTransferCommand = DSP_Halt8bitTransfer;

	BLASTER_SoundPlaying = TRUE;
}


/*---------------------------------------------------------------------
   Function: BLASTER_DSP2xx_BeginPlayback

   Starts playback of digitized sound on cards compatible with DSP
   version 2.xx.
---------------------------------------------------------------------*/

#define DSP_MaxNormalRate         22000

static void BLASTER_DSP2xx_BeginPlayback(int32_t length)
{
	int32_t SampleLength = length - 1;
	int32_t HiByte = HIBYTE( SampleLength );
	int32_t LoByte = LOBYTE( SampleLength );

	BLASTER_WriteDSP( DSP_SetBlockLength );
	BLASTER_WriteDSP( LoByte );
	BLASTER_WriteDSP( HiByte );

	if ( ( BLASTER_Version >= DSP_Version201 ) && ( DSP_MaxNormalRate < ( BLASTER_SampleRate * BLASTER_SamplePacketSize ) ) )
	{
		BLASTER_WriteDSP( DSP_8BitHighSpeedAutoInitMode );
		BLASTER_HaltTransferCommand = DSP_Reset;
	} else {
		BLASTER_WriteDSP( DSP_8BitAutoInitMode );
		BLASTER_HaltTransferCommand = DSP_Halt8bitTransfer;
	}

	BLASTER_SoundPlaying = TRUE;
}


/*---------------------------------------------------------------------
   Function: BLASTER_DSP4xx_BeginPlayback

   Starts playback of digitized sound on cards compatible with DSP
   version 4.xx, such as the Sound Blaster 16.
---------------------------------------------------------------------*/

#define DSP_SignedBit                 0x10
#define DSP_StereoBit                 0x20

#define DSP_UnsignedMonoData      0x00
#define DSP_SignedMonoData        ( DSP_SignedBit )
#define DSP_UnsignedStereoData    ( DSP_StereoBit )
#define DSP_SignedStereoData      ( DSP_SignedBit | DSP_StereoBit )

static void BLASTER_DSP4xx_BeginPlayback(int32_t length)
{
	int32_t TransferCommand;
	int32_t TransferMode;
	int32_t SampleLength;
	int32_t LoByte;
	int32_t HiByte;

	if ( BLASTER_MixMode & SIXTEEN_BIT )
	{
		TransferCommand = DSP_16BitDAC;
		SampleLength = ( length / 2 ) - 1;
		BLASTER_HaltTransferCommand = DSP_Halt16bitTransfer;
		if ( BLASTER_MixMode & STEREO )
			TransferMode = DSP_SignedStereoData;
		else
			TransferMode = DSP_SignedMonoData;
	} else {
		TransferCommand = DSP_8BitDAC;
		SampleLength = length - 1;
		BLASTER_HaltTransferCommand = DSP_Halt8bitTransfer;
		if ( BLASTER_MixMode & STEREO )
			TransferMode = DSP_UnsignedStereoData;
		else
			TransferMode = DSP_UnsignedMonoData;
	}

	HiByte = HIBYTE( SampleLength );
	LoByte = LOBYTE( SampleLength );

	// Program DSP to play sound
	BLASTER_WriteDSP( TransferCommand );
	BLASTER_WriteDSP( TransferMode );
	BLASTER_WriteDSP( LoByte );
	BLASTER_WriteDSP( HiByte );

	BLASTER_SoundPlaying = TRUE;
}


/*---------------------------------------------------------------------
   Function: BLASTER_BeginBufferedPlayback

   Begins multibuffered playback of digitized sound on the sound card.
---------------------------------------------------------------------*/

int32_t BLASTER_BeginBufferedPlayback(uint8_t *BufferStart, int32_t BufferSize, int32_t NumDivisions, uint32_t SampleRate, int32_t MixMode, void (*CallBackFunc)(void))
{
	int32_t DmaStatus;
	int32_t TransferLength;

	if ( BLASTER_SoundPlaying )
		BLASTER_StopPlayback();

	BLASTER_SetMixMode( MixMode );

	DmaStatus = BLASTER_SetupDMABuffer( BufferStart, BufferSize );
	if ( DmaStatus == BLASTER_Error )
		return BLASTER_Error;

	BLASTER_SetPlaybackRate( SampleRate );

	// Specifie the user function to call at the end of a sound transfer
	BLASTER_CallBack = CallBackFunc;

	BLASTER_EnableInterrupt();

	// Turn on speaker
	BLASTER_WriteDSP(DSP_SpeakerOn);

	TransferLength = BufferSize / NumDivisions;
	BLASTER_TransferLength = TransferLength;

	//  Program the sound card to start the transfer.
	if ( BLASTER_Version < DSP_Version2xx )
		BLASTER_DSP1xx_BeginPlayback( TransferLength );
	else if ( BLASTER_Version < DSP_Version4xx )
		BLASTER_DSP2xx_BeginPlayback( TransferLength );
	else
		BLASTER_DSP4xx_BeginPlayback( TransferLength );

	return BLASTER_Ok;
}


/*---------------------------------------------------------------------
   Function: BLASTER_WriteMixer

   Writes a byte of data to the Sound Blaster's mixer chip.
---------------------------------------------------------------------*/

static void BLASTER_WriteMixer(int32_t reg, int32_t data)
{
	outp(BLASTER_Config.Address + BLASTER_MixerAddressPort, reg);
	outp(BLASTER_Config.Address + BLASTER_MixerDataPort,   data);
}


/*---------------------------------------------------------------------
   Function: BLASTER_ReadMixer

   Reads a byte of data from the Sound Blaster's mixer chip.
---------------------------------------------------------------------*/

static int32_t BLASTER_ReadMixer(int32_t reg)
{
	outp(BLASTER_Config.Address + BLASTER_MixerAddressPort, reg);
	return inp(BLASTER_Config.Address + BLASTER_MixerDataPort);
}


/*---------------------------------------------------------------------
   Function: BLASTER_SetVoiceVolume

   Sets the volume of the digitized sound channel on the Sound
   Blaster's mixer chip.
---------------------------------------------------------------------*/

#define volume 255

static void BLASTER_SetVoiceVolume(void)
{
	int32_t data;

	switch (BLASTER_Config.Type)
	{
		case SBPro:
			data = (volume & 0xf0) + (volume >> 4);
			BLASTER_WriteMixer(MIXER_SBProVoice, data);
			break;

		case SB16:
			BLASTER_WriteMixer(MIXER_SB16VoiceLeft,  volume & 0xf8);
			BLASTER_WriteMixer(MIXER_SB16VoiceRight, volume & 0xf8);
			break;
	}
}


/*---------------------------------------------------------------------
   Function: BLASTER_SaveVoiceVolume

   Saves the user's voice mixer settings.
---------------------------------------------------------------------*/

static void BLASTER_SaveVoiceVolume(void)
{
	switch(BLASTER_Config.Type)
	{
		case SBPro:
			BLASTER_OriginalVoiceVolumeLeft  = BLASTER_ReadMixer(MIXER_SBProVoice);
			break;

		case SB16:
			BLASTER_OriginalVoiceVolumeLeft  = BLASTER_ReadMixer(MIXER_SB16VoiceLeft);
			BLASTER_OriginalVoiceVolumeRight = BLASTER_ReadMixer(MIXER_SB16VoiceRight);
			break;
	}
}


/*---------------------------------------------------------------------
   Function: BLASTER_RestoreVoiceVolume

   Restores the user's voice mixer settings.
---------------------------------------------------------------------*/

static void BLASTER_RestoreVoiceVolume(void)
{
	switch(BLASTER_Config.Type)
	{
		case SBPro:
			BLASTER_WriteMixer(MIXER_SBProVoice, BLASTER_OriginalVoiceVolumeLeft);
			break;

		case SB16:
			BLASTER_WriteMixer(MIXER_SB16VoiceLeft,  BLASTER_OriginalVoiceVolumeLeft);
			BLASTER_WriteMixer(MIXER_SB16VoiceRight, BLASTER_OriginalVoiceVolumeRight);
			break;
	}
}


/*---------------------------------------------------------------------
   Function: BLASTER_GetEnv

   Retrieves the BLASTER environment settings and returns them to
   the caller.
---------------------------------------------------------------------*/

#define BlasterEnv_Address    'A'
#define BlasterEnv_Interrupt  'I'
#define BlasterEnv_8bitDma    'D'
#define BlasterEnv_16bitDma   'H'

static void BLASTER_GetEnv(int32_t *sbPort, int32_t *sbIrq, int32_t *sbDma8, int32_t *sbDma16)
{
	char *Blaster;
	char parameter;

	*sbDma16 = UNDEFINED;

	Blaster = getenv("BLASTER");
	if (Blaster == NULL)
		return;

	while(*Blaster != 0)
	{
		if (*Blaster == ' ')
		{
			Blaster++;
			continue;
		}

		parameter = toupper(*Blaster);
		Blaster++;

		if (!isxdigit(*Blaster))
			return;

		switch(parameter)
		{
			case BlasterEnv_Address:
				sscanf(Blaster, "%x", sbPort);
				break;
			case BlasterEnv_Interrupt:
				sscanf(Blaster, "%d", sbIrq);
				break;
			case BlasterEnv_8bitDma:
				sscanf(Blaster, "%d", sbDma8);
				break;
			case BlasterEnv_16bitDma:
				sscanf(Blaster, "%d", sbDma16);
				break;
			default  :
				// Skip the offending data
				// sscanf( Blaster, "%*s" );
				break;
		}

		while(isxdigit(*Blaster))
			Blaster++;
	}
}


/*---------------------------------------------------------------------
   Function: BLASTER_SetCardSettings

   Sets up the sound card's parameters.
---------------------------------------------------------------------*/

static void BLASTER_SetCardSettings(int32_t sbPort, int32_t sbIrq, int32_t sbDma, int32_t sbDma16)
{
	if (BLASTER_Installed)
		BLASTER_Shutdown();

	BLASTER_Config.Address   = sbPort;
	BLASTER_Config.Interrupt = sbIrq;
	BLASTER_Config.Dma8      = sbDma;
	BLASTER_Config.Dma16     = sbDma16;
}


Boolean BLASTER_Detect(void)
{
	int32_t sbPort  = 0;
	int32_t sbIrq   = 0;
	int32_t sbDma   = 0;
	int32_t sbDma16 = 0;

	BLASTER_GetEnv(&sbPort, &sbIrq, &sbDma, &sbDma16);
	BLASTER_SetCardSettings(sbPort, sbIrq, sbDma, sbDma16);

	return sbPort == 0 || sbIrq == 0 || sbDma == 0;
}


/*---------------------------------------------------------------------
   Function: BLASTER_SetupWaveBlaster

   Allows the WaveBlaster to play music while the Sound Blaster 16
   plays digital sound.
---------------------------------------------------------------------*/

void BLASTER_SetupWaveBlaster(void)
{
	if (BLASTER_Config.Type == SB16)
	{
		// Disable MPU401 interrupts.  If they are not disabled,
		// the SB16 will not produce sound or music.
		BLASTER_WriteMixer(MIXER_DSP4xxISR_Enable, MIXER_DisableMPU401Interrupts);
	}
}


Boolean BLASTER_IsSwapLeftRight(void)
{
	return BLASTER_Config.Type == SBPro;
}


/*---------------------------------------------------------------------
   Function: BLASTER_Init

   Initializes the sound card and prepares the module to play
   digitized sounds.
---------------------------------------------------------------------*/

void BLASTER_Init(void)
{
	int32_t status;

	if ( BLASTER_Installed )
		BLASTER_Shutdown();

	// Save the interrupt masks
	BLASTER_IntController1Mask = inp(0x21);
	BLASTER_IntController2Mask = inp(0xA1);

	status = BLASTER_ResetDSP();
	if (status != BLASTER_Ok)
		return;

	BLASTER_SaveVoiceVolume();

	BLASTER_SoundPlaying = FALSE;

	BLASTER_CallBack = NULL;

	BLASTER_DMABuffer = NULL;

	BLASTER_Version = BLASTER_GetDSPVersion();

	BLASTER_DMAChannel = UNDEFINED;
	if (BLASTER_Config.Dma16 != UNDEFINED && BLASTER_Version >= DSP_Version4xx)
	{
		status = DMA_VerifyChannel(BLASTER_Config.Dma16);
		if (status == DMA_Ok)
			BLASTER_DMAChannel = BLASTER_Config.Dma16;
	}

	if (BLASTER_DMAChannel == UNDEFINED)
	{
		status = DMA_VerifyChannel(BLASTER_Config.Dma8);
		if (status != DMA_Ok)
			return;

		BLASTER_DMAChannel = BLASTER_Config.Dma8;
	}

	if (BLASTER_Version >= DSP_Version4xx)
	{
		BLASTER_Config.Type            = SB16;
		BLASTER_Config.MinSamplingRate = 5000;
		BLASTER_Config.MaxSamplingRate = 44100;
		BLASTER_Config.MaxMixMode      = BLASTER_DMAChannel == BLASTER_Config.Dma16 ? STEREO_16BIT : STEREO_8BIT;
	}
	else if (BLASTER_Version >= DSP_Version3xx)
	{
		BLASTER_Config.Type            = SBPro;
		BLASTER_Config.MinSamplingRate = 4000;
		BLASTER_Config.MaxSamplingRate = 44100;
		BLASTER_Config.MaxMixMode      = STEREO_8BIT;
	}
	else if (BLASTER_Version >= DSP_Version2xx)
	{
		BLASTER_Config.Type            = SB20;
		BLASTER_Config.MinSamplingRate = 4000;
		BLASTER_Config.MaxSamplingRate = 23000;
		BLASTER_Config.MaxMixMode      = MONO_8BIT;
	}
	else if (BLASTER_Version >= DSP_Version1xx)
	{		
		BLASTER_Config.Type            = SB;
		BLASTER_Config.MinSamplingRate = 4000;
		BLASTER_Config.MaxSamplingRate = 23000;
		BLASTER_Config.MaxMixMode      = MONO_8BIT;
	}
	else
		return;

	BLASTER_SetPlaybackRate(BLASTER_DefaultSampleRate);
	BLASTER_SetMixMode(BLASTER_DefaultMixMode);

	// Install our interrupt handler
	if (!(BLASTER_Config.Interrupt == 2 || BLASTER_Config.Interrupt == 5 || BLASTER_Config.Interrupt == 7))
		return;

	replaceInterrupt(BLASTER_OldInt, BLASTER_NewInt, BLASTER_Config.Interrupt + 8, BLASTER_ServiceInterrupt);

	BLASTER_SetVoiceVolume();

	BLASTER_Installed = TRUE;
}


/*---------------------------------------------------------------------
   Function: BLASTER_Shutdown

   Ends transfer of sound data to the sound card and restores the
   system resources used by the card.
---------------------------------------------------------------------*/

void BLASTER_Shutdown(void)
{
	// Halt the DMA transfer
	BLASTER_StopPlayback();

	BLASTER_RestoreVoiceVolume();

	// Reset the DSP
	BLASTER_ResetDSP();

	// Restore the original interrupt
	restoreInterrupt(BLASTER_Config.Interrupt + 8, BLASTER_OldInt, BLASTER_NewInt);

	BLASTER_SoundPlaying = FALSE;

	BLASTER_DMABuffer = NULL;

	BLASTER_CallBack = NULL;

	BLASTER_Installed = FALSE;
}
