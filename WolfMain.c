#include <string.h>

#include "wolfdef.h"


static Word LastTicCount;			/* Tick value at start of render */

static const Boolean SlowDown = TRUE;			/* Force the game to 15 hz */ /* If true, then limit game to 30hz */


/**********************************

	Return the absolute value
	
**********************************/

Word w_abs(int val)
{
	return val>=0 ? val : -val;
}

/**********************************

	Return a seeded random number
	
**********************************/

static const Byte rndtable[256] = {
	  0,  8, 109, 220, 222, 241, 149, 107,  75, 248, 254, 140,  16,  66,
	 74, 21, 211,  47,  80, 242, 154,  27, 205, 128, 161,  89,  77,  36,
	 95,110,  85,  48, 212, 140, 211, 249,  22,  79, 200,  50,  28, 188,
	 52,140, 202, 120,  68, 145,  62,  70, 184, 190,  91, 197, 152, 224,
	149,104,  25, 178, 252, 182, 202, 182, 141, 197,   4,  81, 181, 242,
	145, 42,  39, 227, 156, 198, 225, 193, 219,  93, 122, 175, 249,   0,
	175,143,  70, 239,  46, 246, 163,  53, 163, 109, 168, 135,   2, 235,
	 25, 92,  20, 145, 138,  77,  69, 166,  78, 176, 173, 212, 166, 113,
	 94,161,  41,  50, 239,  49, 111, 164,  70,  60,   2,  37, 171,  75,
	136,156,  11,  56,  42, 146, 138, 229,  73, 146,  77,  61,  98, 196,
	135,106,  63, 197, 195,  86,  96, 203, 113, 101, 170, 247, 181, 113,
	 80,250, 108,   7, 255, 237, 129, 226,  79, 107, 112, 166, 103, 241,
	 24,223, 239, 120, 198,  58,  60,  82, 128,   3, 184,  66, 143, 224,
	145,224,  81, 206, 163,  45,  63,  90, 168, 114,  59,  33, 159,  95,
	 28,139, 123,  98, 125, 196,  15,  70, 194, 253,  54,  14, 109, 226,
	 71, 17, 161,  93, 186,  87, 244, 138,  20,  52, 123, 251,  26,  36,
	 17, 46,  52, 231, 232,  76,  31, 221,  84,  37, 216, 165, 212, 106,
	197,242,  98,  43,  39, 175, 254, 145, 190,  84, 118, 222, 187, 136,
	120,163, 236, 249
};

static Word rndindex = 0;

Word w_rnd(void)
{
	rndindex = (rndindex+1)&0xff;	/* Next index */
	return rndtable[rndindex];		/* Return the number */
}


/**********************************

	Return an angle value based on a slope.
	Assume that x is >= to y.
		
**********************************/

static Word AngleFromSlope2(Word y,Word x)
{
	return tantoangle[((LongWord)y<<SLOPEBITS)/x];
}

/**********************************

	Convert an arbitrary point from the viewxy into an angle.
	
	To get a global angle from cartesian coordinates, the coordinates are flipped until 
	they are in the first octant of the coordinate system, then the y (<=x) is scaled and 
	divided by x to get a tangent (slope) value which is looked up in the tantoangle[] table.
	The +1 size is to handle the case when x==y without additional checking.
	
**********************************/

#define	ANG90	0x4000
#define	ANG180	0x8000
#define	ANG270	0xc000
angle_t PointToAngle(fixed_t x, fixed_t y)
{	
	x -= viewx;		/* Adjust the x and y based on the camera */
	y = viewy - y;
	
	if (x>=0) {	/* x is positive? */
		if (y>=0) {	/* y is positive? */
			if (x>y) {
				return AngleFromSlope2(y,x);	/* octant 0*/
			} else {
				return ANG90-1-AngleFromSlope2(x,y);	/* octant 1 */
			}
		} else {	/* y<0 */
			y = -y;	/* Negate y (Make positive) */
			if (x>y) {
				return -AngleFromSlope2(y,x);	/* octant 8 */
			} else {
				return ANG270+AngleFromSlope2 (x,y);	/* octant 7 */
			}
		}
	} else {	/* x<0*/
		x = -x;	/* Force x positive */
		if (y>= 0) {	/* Is y positive? */
			if (x>y) {
				return ANG180-1-AngleFromSlope2(y,x);	/* octant 3 */
			} else {
				return ANG90+AngleFromSlope2(x,y);		/* octant 2 */
			}
		} else {	/* y<0*/
			y = -y;	/* Force y positive also */
			if (x>y) {
				return ANG180+AngleFromSlope2 (y,x);	/* octant 4 */
			}
		}
	}
	return ANG270-1-AngleFromSlope2(x,y);	/* octant 5 */
}


static Word redshift;
static Word goldshift;
static Word damagecount;
static Word bonuscount;

void StartBonusFlash(void)
{
	bonuscount = 32;
}


void StartDamageFlash(Word damage)
{
	damagecount += damage;
	if (damagecount > 200)
		damagecount = 200;
}


static void UpdatePaletteShifts(void)
{
	Word red, gold;

	if (bonuscount)
		bonuscount--;
	if (damagecount)
		damagecount--;

	red = damagecount / 2;
	if (red > 8)
		red = 8;

	gold = bonuscount / 8;

	if (red != redshift || gold != goldshift) {
		redshift  = red;
		goldshift = gold;
		IO_ColorScreen(goldshift, redshift);
	}
}


/**********************************

	Died() has already spun the view toward the killer
	GameOver() scales the "game over" sprite in and pulses it until an event

**********************************/

static void GameOver(void)
{	
	topspritenum = S_GAMEOVER;		/* Game over words */
	topspritescale = 8;	/* Start the scale factor */
	do {
		RenderView();		/* Draw the 3-d view */
		topspritescale+=8;
	} while (topspritescale<120);
	
	for (;;) {		/* Stay here forever */
		do {
			if (WaitTicksEvent(1)) {	/* Canceled? */
				return;
			}
			RenderView();				/* Show the 3-D view */
			topspritescale-=2;	/* Make smaller */
		} while(topspritescale>100);	/* Not too small... */
		do {
			if (WaitTicksEvent(1)) {	/* Canceled? */
				return;
			}
			RenderView();				/* Show the 3-D view */
			topspritescale+=2;	/* Make bigger */
		} while (topspritescale<120);	/* Not too big... */
	}
}

/**********************************

	Show the word "Victory" scaling in...
	
**********************************/

static void VictoryScale(void)
{
	topspritenum = S_VICTORY;
	topspritescale = 16;
	
	do {
		RenderView();
		topspritescale += 16;
	} while (topspritescale<240);
	
	for (;;) {
		do {
			if (WaitTicksEvent(1)) {
				return;
			}
			RenderView();
			topspritescale-=4;
		} while (topspritescale>200);
		do {
			if (WaitTicksEvent(1)) {
				return;
			}
			RenderView();
			topspritescale+=4;
		} while (topspritescale<240);	
	}
}

/**********************************

	You died...
	
**********************************/

static void Died (void)
{
	Word Timer;	/* Time mark */
	Word Adds;		/* Number of tics elapsed */
	Word Total;	
	angle_t	SrcAngle;	/* Angle of view */
	angle_t DestAngle;	/* Angle of death */
	fixed_t Motion;		/* Motion constant */
	
	gamestate.attackframe = 0;		/* Remove the gun shape */
	NoWeaponDraw = TRUE;			/* The weapon is not drawn */
	PlaySound(SND_PDIE);				/* ARRRGGGHHHH!! */
	IO_DrawFace(9);					/* Show the dead face */
		
/* find angle to face attacker */

	SrcAngle = gamestate.viewangle<<SHORTTOANGLESHIFT;		/* Get the fine current angle */
	DestAngle = PointToAngle(killx,killy)&(0xffff<<SHORTTOANGLESHIFT);	/* What's the direction of the kill angle */	
	
/* rotate to attacker */

	if (SrcAngle!=DestAngle) {				/* Do I need to rotate? */
		Motion = (fixed_t)(DestAngle-SrcAngle)/ (fixed_t)30;		/* Differance in fine angles */
		LastTicCount = ReadTick();
		Total = 30;			/* Number of tics to elapse */
		for (;;) {	/* There yet? */
			Timer = ReadTick();		/* How much time has elapsed */
			Adds = Timer-LastTicCount;	/* Save the time */
			LastTicCount = Timer;	/* Mark the new time */
			if (Adds>=Total) {		/* Too much time? */
				break;				/* Get out now! */
			}
			Total-=Adds;			/* Remove elapsed time */
			SrcAngle += Motion*Adds;	/* Spin */
			gamestate.viewangle = SrcAngle>>SHORTTOANGLESHIFT;	/* Set the new view angle */
			RenderView();		/* Show the view */
		}				
		gamestate.viewangle = DestAngle>>SHORTTOANGLESHIFT;		/* Finish the motion */
		RenderView();			/* Draw the screen */
	}
/* done */

	if (!gamestate.lives) {
		GameOver();			/* Show Game over... */
	} else {
		WaitEvent();			/* Wait for an event */
		gamestate.health = 100;	/* Restore health */
		if (difficulty) {		/* Remove weapons if difficult */
			gamestate.weapon = gamestate.pendingweapon = WP_PISTOL;
			gamestate.machinegun = FALSE;
			gamestate.chaingun = FALSE;
			gamestate.missile = FALSE;
			gamestate.flamethrower = FALSE;
			gamestate.ammo = STARTAMMO;
			gamestate.missiles = 0;
			gamestate.gas = 0;
			gamestate.maxammo = 99;
			gamestate.keys = 0;
		} else {
			if (gamestate.ammo < STARTAMMO*2) {	/* If easy, then reload... */
				gamestate.ammo = STARTAMMO*2;
			}
		}
		gamestate.attackframe = 0;		/* Reset the attack frame */
		gamestate.attackcount = 0;
	}
	FadeToBlack();			/* Fade the screen to black */
}

/**********************************

	Calls draw face if time to change
	
**********************************/

static void UpdateFace(void)
{
	Word Base;
	
	if (facecount>TicCount) {	/* Time to change the face? */
		facecount-=TicCount;	/* Wait a bit */
	} else {
		Base = (gamestate.health <= 25) ? 5 : 0;
		if (faceframe==Base) {	/* Normal frame? */
			++Base;		/* Use alternate */
		}
		faceframe=Base;	/* Set the new face */
		facecount = (w_rnd ()&31)+4;	/* Random time */
		IO_DrawFace(faceframe);		/* Draw the face */
	}
}


static void UpdateFps(void)
{
	static     Word fps_frames     = 0;
	static LongWord fps_timebefore = 0;

	if (ShowFps) {
		LongWord timenow      = ReadTick();
		    Word tics_elapsed = timenow - fps_timebefore;

		fps_frames++;

		if (tics_elapsed >= TICRATE) {
			Word fps_framerate = fps_frames * TICRATE / tics_elapsed;
			if (fps_framerate > 99)
				fps_framerate = 99;

			IO_DrawTreasure(fps_framerate);

			fps_frames     = 0;
			fps_timebefore = timenow;
		}
	}
}


/**********************************

	Prepare for a game loop
	
**********************************/

static void PrepPlayLoop (void)
{
	StartSong(SongListPtr[gamestate.mapon+2]);	/* start music */
	SetupGameLevel();
	StartupRendering();

	topspritescale = 0;	/* No overlay sprite */
	faceframe = 1;		/* First face */
	facecount = 1;		/* change face next tic */
	firstframe = TRUE;		/* Force a fade in */
	NoWeaponDraw = FALSE;	/* Allow drawing of weapon */
	memset(buttonstate,0,sizeof(buttonstate));	/* Kill the mouse system */
	bonuscount  = 0;
	damagecount = 0;
	goldshift   = 0;
	redshift    = 0;
	if (playstate!=EX_LOADGAME) {
		PushWallRec.pwallcount = 0;		/* No pushwalls */
		gamestate.playtime = 0;		/* Game has started */
	} else {
		FinishLoadGame();			/* Finish the load game code */
	}
	playstate = EX_STILLPLAYING;	/* Game is in progress */
}

/**********************************

	Play the game
	
**********************************/

static void PlayLoop(void)
{
	LongWord Timer;
	LastTicCount = ReadTick();
	do {
		Timer = ReadTick();		/* How much time has elapsed */
		TicCount = (Timer-LastTicCount);
		gamestate.playtime += TicCount;	/* Add the physical time to the elapsed time */
		
		LastTicCount=Timer;
		if (!SlowDown) {
			TicCount = 4;		/* Adjust from 4 */
		}
		if (TicCount>=5) {
			TicCount = 4;
		}
		IO_CheckInput();	/* Read the controls from the system */
		madenoise = FALSE;	/* No noise made (Yet) */
		MoveDoors();		/* Open and close all doors */
		MovePWalls();		/* Move all push walls */
		MovePlayer();		/* Move the player */
		MoveActors();		/* Move all the bad guys */
		MoveMissiles();		/* Move all projectiles */	
		UpdateFace();		/* Draw BJ's face and animate it */
		UpdatePaletteShifts();
		UpdateFps();
		viewx = actors[0].x;	/* Where is the camera? */
		viewy = actors[0].y;		
		RenderView();		/* Draw the 3D view */
	} while (playstate==EX_STILLPLAYING);
	StopSong();
}

/**********************************

	Set up new game to start from the beginning

**********************************/

void NewGame(void)
{
	nextmap = 0;			/* No next map inited */
	memset(&gamestate,0,sizeof(gamestate));	/* Zap the game variables */
	gamestate.weapon = gamestate.pendingweapon = WP_PISTOL;	/* Set the pistol */
	gamestate.health = 100;		/* Reset the health */
	gamestate.ammo = STARTAMMO;	/* Reset the ammo */
	if (!difficulty) {
		gamestate.ammo += STARTAMMO;	/* Double the ammo if easy */
	}		
	gamestate.maxammo = 99;		/* Refill the ammo */
	gamestate.lives = 3;		/* 3 lives */
	gamestate.nextextra = EXTRAPOINTS;	/* Next free life score needed */
	gamestate.godmode = FALSE;		/* Force god mode off */
}

/**********************************

	Redraw the main status bar at the bottom
	
**********************************/

void RedrawStatusBar(void)
{	
	IO_DrawStatusBar();			/* Draw the status bar */
	IO_DrawFloor(gamestate.mapon);	/* Draw the current floor # */
	IO_DrawScore(gamestate.score);	/* Draw the current score */
	IO_DrawTreasure(gamestate.treasure);	/* Draw the treasure score */
	IO_DrawLives(gamestate.lives);		/* Draw the life count */
	IO_DrawHealth(gamestate.health);	/* Draw the health */
	switch (gamestate.weapon) {			/* Draw the proper ammo */
	case WP_FLAMETHROWER:
		IO_DrawAmmo(gamestate.gas);
		break;
	case WP_MISSILE:
		IO_DrawAmmo(gamestate.missiles);
		break;
	default:
		IO_DrawAmmo(gamestate.ammo);
		break;
	}
	IO_DrawKeys(gamestate.keys);		/* Draw the keys held */
	IO_DrawFace(faceframe);				/* Draw BJ's face */
}

/**********************************

	Play the game!
	
**********************************/

void GameLoop (void)
{
	for(;;) {
skipbrief:		
		ShowGetPsyched();
		PrepPlayLoop();		/* Init internal variables */
		EndGetPsyched();
		RedrawStatusBar();	/* Redraw the main bar */
		PlayLoop();			/* Play the game */
		if (playstate == EX_DIED) {		/* Did you die? */
			--gamestate.lives;			/* Remove a life */
			Died();						/* Run the death code */
			if (!gamestate.lives) {		/* No more lives? */
				return;					/* Exit then */
			}	
			goto skipbrief;				/* Try again */
		}
		
		if (playstate == EX_SECRET) {	/* Going to the secret level? */
			nextmap = MapListPtr->InfoArray[gamestate.mapon].SecretLevel;
		} else if (playstate != EX_WARPED) {
			nextmap = MapListPtr->InfoArray[gamestate.mapon].NextLevel; /* Normal warp? */
		}			/* If warped, then nextmap is preset */
		if (nextmap == 0xffff) {	/* Last level? */
			VictoryScale();				/* You win!! */
			ReleaseMap();		/* Unload the map */
			Intermission();				/* Display the wrapup... */
			ShareWareEnd();			/* End the game for the shareware version */
//			VictoryIntermission();		/* Wrapup for victory! */
			return;
		}
		ReleaseMap();				/* Unload the game map */
		gamestate.keys = 0;			/* Zap the keys */
		WaitTicks(1);				/* Flush the ticker */
		WaitTicks(30);				/* Wait for the sound to finish */
		if (playstate != EX_WARPED) {	/* No bonus for warping! */
			Intermission();				/* Show the wrap up... */
		}
		gamestate.mapon = nextmap;	/* Next level */
	}
}
