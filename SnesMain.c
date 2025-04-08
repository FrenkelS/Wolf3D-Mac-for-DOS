#include <ctype.h>
#include <setjmp.h>
#include <string.h>

#include "WolfDef.h"

/**********************************

	Prepare the screen for game

**********************************/

void SetupPlayScreen (void)
{
	SetAPalette(rBlackPal);		/* Force black palette */
	ClearTheScreen();		/* Clear the screen to black */
	BlastScreen();
	firstframe = TRUE;				/* fade in after drawing first frame */
}

/**********************************

	Display the automap
	
**********************************/

static int clamp(int x)
{
	return x >= 0 ? x : 0;
}

void RunAutoMap(void)
{
	Word vx,vy;
	Word Width,Height;
	Word CenterX,CenterY;
	Word oldjoy,newjoy;
	Word oldzoomlevel;

	MakeSmallFont();				/* Make the tiny font */
	playstate = EX_AUTOMAP;
	vx = viewx>>8;					/* Get my center x,y */
	vy = viewy>>8;

	Width  = SCREENWIDTH  >> automapzoomlevel;		/* Width of map in tiles */
	Height = SCREENHEIGHT >> automapzoomlevel;		/* Height of map in tiles */
	CenterX = Width/2;
	CenterY = Height/2;
	if (vx>=CenterX) {
		vx -= CenterX;
	} else {
		vx = 0;
	}
	if (vy>=CenterY) {
		vy -= CenterY;
	} else {
		vy = 0;
	}

	oldjoy = joystick1;
	do {
		ClearTheScreen();
		DrawAutomap(vx,vy);
		oldzoomlevel = automapzoomlevel;
		do {
			ReadSystemJoystick();
		} while (joystick1 == oldjoy && automapzoomlevel == oldzoomlevel);

		if (automapzoomlevel != oldzoomlevel) {
			Width  = SCREENWIDTH  >> automapzoomlevel;
			Height = SCREENHEIGHT >> automapzoomlevel;
		}

		if (joystick1 != oldjoy) {
			oldjoy &= joystick1;
			newjoy = joystick1 ^ oldjoy;
			if (newjoy & (JOYPAD_START|JOYPAD_SELECT|JOYPAD_A|JOYPAD_B|JOYPAD_X|JOYPAD_Y)) {
				playstate = EX_STILLPLAYING;
			}
			if (newjoy & JOYPAD_UP && vy) {
				--vy;
			}
			if (newjoy & (JOYPAD_LFT | JOYPAD_TL) && vx) {
				--vx;
			}
			if (newjoy & (JOYPAD_RGT | JOYPAD_TR) && vx < clamp(MAPSIZE - Width)) {
				++vx;
			}
			if (newjoy & JOYPAD_DN && vy < (MAPSIZE - Height)) {
				++vy;
			}
		}
	} while (playstate==EX_AUTOMAP);

	playstate = EX_STILLPLAYING;
/* let the player scroll around until the start button is pressed again */
	KillSmallFont();			/* Release the tiny font */
	RedrawStatusBar();
	I_ClearView();
	ReadSystemJoystick();
	mouseturn = 0;
}

/**********************************

	Begin a new game
	
**********************************/

static void StartGame(void)
{	
	if (playstate!=EX_LOADGAME) {	/* Variables already preset */
		NewGame();				/* init basic game stuff */
	}
	SetupPlayScreen();
	GameLoop();			/* Play the game */
	StopSong();			/* Make SURE music is off */
}

/**********************************

	Show the game logo 

**********************************/

static void TitleScreen (void)
{
	playstate = EX_LIMBO;	/* Game is not in progress */
	FadeToBlack();		/* Fade out the video */
	DrawRawFullScreen(rTitlePic);
	BlastScreen();
	StartSong(SongListPtr[0]);
	FadeTo(rTitlePal);	/* Fade in the picture */
	WaitTicksEvent(0);		/* Wait for event */
	playstate = EX_COMPLETED;
}

/**********************************

	Main entry point for the game (Called after InitTools)

**********************************/

static jmp_buf ResetJmp;
static Boolean JumpOK;
extern Word NumberIndex;

void WolfMain(void)
{
	InitTools();		/* Init the system environment */
	WaitTick();			/* Wait for a system tick to go by */
	playstate = (exit_t)setjmp(ResetJmp);	
	NumberIndex = 36;	/* Force the score to redraw properly */
	IntermissionHack = FALSE;
	if (playstate) {
		goto DoGame;	/* Begin a new game or saved game */
	}
	JumpOK = TRUE;		/* Jump vector is VALID */
	FlushKeys();		/* Allow a system event */
	Intro();			/* Do the game intro */
	for (;;) {
		TitleScreen();		/* Show the game logo */
		StartSong(SongListPtr[0]);
		ClearTheScreen();	/* Blank out the title page */
		BlastScreen();
		SetAPalette(rBlackPal);
		if (ChooseGameDiff()) {	/* Choose your difficulty */
			playstate = EX_NEWGAME;	/* Start a new game */
DoGame:
			FadeToBlack();		/* Fade the screen */
			StartGame();		/* Play the game */
		}
	}
}
