#include <ctype.h>
#include <stdlib.h>

#include "wolfdef.h"

/**********************************

	Main game introduction

**********************************/

void Intro(void)
{
	FadeToBlack();		/* Fade out the video */
	DrawRawFullScreen(rMacPlayPic);

	BlastScreen();
	StartSong(SongListPtr[0]);	/* Play the song */
	FadeTo(rMacPlayPal);	/* Fade in the picture */
	WaitTicksEvent(240);		/* Wait for event */
	FadeTo(rIdLogoPal);
	if (toupper(WaitTicksEvent(240))=='B') {		/* Wait for event */
		FadeToBlack();
		DrawRawFullScreen(rYummyPic);
		BlastScreen();
		FadeTo(rYummyPal);
		WaitTicksEvent(600);
	}
}
