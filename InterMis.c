#include <stdlib.h>
#include <string.h>

#include "WolfDef.h"

extern Word NumberIndex;	/* Hack for drawing numbers */
static LongWord BJTime;	/* Time to draw BJ? */
static Word WhichBJ;	/* Which BJ to show */
static Word ParTime;		/* Par time for level */
static LongWord BonusScore;	/* Additional points */

#define BONUSX	220
#define BONUSY	54
#define	TIMEX	223
#define TIMEWIDTH 22
#define	TIMEY	74
#define	TIMEY2	94
#define SCOREX 220
#define SCOREY 174
#define	RATIOX	221
#define	RATIOY	114
#define	RATIOY2	133
#define	RATIOY3	153

/**********************************

	Draw BJ if needed
	
**********************************/

static Rect BJRect = {26,46,26+82,46+74};	/* Rect for BJ's picture */
static void ShowBJ(void) 
{		
	if ((ReadTick()-BJTime) >= 20) {		/* Time to draw a BJ? */
		BJTime = ReadTick();				/* Set the new time */
		if (WhichBJ!=2) {			/* Thumbs up? */
			WhichBJ ^= 1;			/* Nope, toggle breathing */
		}
		DrawShapeNum(46,26,rInterPics + WhichBJ);		/* Draw BJ */
		BlastScreen2(&BJRect);				/* Update video */
	}
}

/**********************************

	Have BJ Breath for a while
	
**********************************/

static void BJBreath(Word Delay)
{
	do {
		ShowBJ();
		if (WaitTicksEvent(1)) {
			break;
		}
	} while (--Delay);
}

/**********************************

	Draw the score 
	
**********************************/

static Rect ScoreRect = {SCOREY,SCOREX,SCOREY+22,SCOREX+(12*7)};

static void DrawIScore(void) 
{
	SetNumber(gamestate.score,SCOREX,SCOREY,7);		/* Draw the game score */
	BlastScreen2(&ScoreRect);
}

/**********************************

	Draw the earned bonus
	
**********************************/

static Rect BonusRect = {BONUSY,BONUSX,BONUSY+22,BONUSX+(12*7)};
static void DrawIBonus(void)
{
	SetNumber(BonusScore,BONUSX,BONUSY,7);
	BlastScreen2(&BonusRect);
}

/**********************************

	Draw a time value at the given coords
	
**********************************/

static void DrawTime(Word x,Word y,Word time)
{
	Word minutes,seconds;
	Rect TimeRect;

	TimeRect.left = x;
	TimeRect.right = x+((12*4)+TIMEWIDTH);
	TimeRect.top = y;
	TimeRect.bottom = y+22;
	
	minutes = time/60;
	seconds = time%60;
	SetNumber(minutes,x,y,2);
	x+=TIMEWIDTH;
	SetNumber(seconds,x,y,2);
	BlastScreen2(&TimeRect);
}

/**********************************

	Draws a ratio value at the given coords.
	
**********************************/

static void DrawRatio(Word x,Word y,Word theRatio)
{
	Rect RatioRect;
	
	RatioRect.top = y;
	RatioRect.left = x;
	RatioRect.bottom = y+22;
	RatioRect.right = x+(3*12);
	SetNumber(theRatio,x,y,3);
	BlastScreen2(&RatioRect);
}

/**********************************

	RollScore
	Do a Bill-Budgey roll of the old score to the new score,
	not bothering with the lower digit, as you never get less
	than ten for anything.

**********************************/

static void RollScore(void)
{
	Word i;

	do {
		if (BonusScore>1000) {
			i = 1000;
		} else {
			i = BonusScore;
		}
		BonusScore-=i;
		GivePoints(i);
		ShowBJ();
		DrawIScore();
		DrawIBonus();
		PlaySound(SND_MGUN|0x8000);
		if (WaitTicksEvent(6)) {
			GivePoints(BonusScore);	/* Add the final bonus */
			BonusScore=0;
			DrawIScore();
			DrawIBonus();
			break;
		}
	} while (BonusScore);
}

/**********************************

	RollRatio
	Do a Bill-Budgey roll of the ratio.

**********************************/

static void RollRatio(Word x,Word y,Word ratio)
{
	Word i;
	Word NoDelay;

	i = 0;
	NoDelay = 0;
	while (i<ratio) {
		DrawRatio(x,y,i);
		PlaySound(SND_MGUN|0x8000);
		ShowBJ();
		if (WaitTicksEvent(6)) {	
			NoDelay = 1;
			break;
		}
		i+=10;
	}
	DrawRatio(x,y,ratio);

	/* make ding sound */

	if (ratio==100) {
		if (!NoDelay) {
			PlaySound(SND_EXTRA);
			WaitTicks(30);
		}
		BonusScore += 10000;
		DrawIBonus();
		if (!NoDelay) {
			BJBreath(1 * TICRATE);	/* Breath a little */
		}
	}
}

/**********************************

	Let's show 'em how they did!
	
**********************************/

static void LevelCompleted (void)
{
	Word k;

/* setup */

	ParTime = MapListPtr->InfoArray[gamestate.mapon].ParTime;
	BonusScore = 0;		/* Init the bonus */
	
	IntermissionHack = TRUE;	/* Hack to keep score from drawing twice */
	NumberIndex = 47;		/* Hack to draw score using an alternate number set */
	DrawRawFullScreen(rIntermission);
	BlastScreen();
	DrawRawFullScreen(rIntermission);
	BlastScreen();
	DrawRawFullScreen(rIntermission);
	BlastScreen();

	WhichBJ = 0;		/* Init BJ */
	BJTime = ReadTick()-50;		/* Force a redraw */
	BlastScreen();		/* Draw the screen */
	ShowBJ();			/* Draw BJ */
	StartSong(SongListPtr[1]);	/* Play the intermission song */
	SetAPalette(rInterPal);	/* Set the palette */
	DrawIScore();			/* Draw the current score */
	FlushKeys();			/* Flush the keyboard buffer */

	/* First an initial pause */
	
	BJBreath(1 * TICRATE);

	/* Display Par Time, Player's Time, and show bonus if any. */

	if (gamestate.playtime>=(100*60*60UL)) {
		k =(99*60)+59;
	} else {
		k = gamestate.playtime/60;
	}
	DrawTime(TIMEX,TIMEY,k);		/* How much time has elapsed? */
	DrawTime(TIMEX,TIMEY2,ParTime);

	if (k < ParTime) {
		k = (ParTime-k) * 50;		/* 50 points per second */
		BonusScore += k;		/* Add to the bonus */
		DrawIBonus();			/* Draw the bonus */
		PlaySound(SND_EXTRA);
		BJBreath(1 * TICRATE);			/* Breath a little */
	}

/* Show ratios for "terminations", treasure, and secret stuff. */
/* If 100% on all counts, Perfect Bonus! */

	k=0;		/* Not perfect (Yet) */
	RollRatio(RATIOX,RATIOY, gamestate.treasuretotal == 0 ? 100 : (gamestate.treasurecount * 100) / gamestate.treasuretotal);
	if (gamestate.treasurecount == gamestate.treasuretotal) {
		k++;			/* Perfect treasure */
	}
	RollRatio(RATIOX,RATIOY2, gamestate.killtotal == 0 ? 100 : (gamestate.killcount * 100) / gamestate.killtotal);
	if (gamestate.killcount == gamestate.killtotal) {
		k++;			/* Perfect kills */
	}
	RollRatio(RATIOX,RATIOY3, gamestate.secrettotal == 0 ? 100 : (gamestate.secretcount * 100) / gamestate.secrettotal);
	if (gamestate.secretcount == gamestate.secrettotal) {
		k++;			/* Perfect secret */
	}
	if (BonusScore) {	/* Did you get a bonus? */
		RollScore();
		BJBreath(1 * TICRATE);
	}
	if (k==3) {
		WhichBJ = 2;	/* Draw thumbs up for BJ */
		PlaySound(SND_THUMBSUP);
	}
	do {
		ShowBJ();		/* Animate BJ */
	} while (!WaitTicksEvent(1));		/* Wait for a keypress */
	FadeToBlack();		/* Fade away */
	IntermissionHack = FALSE;		/* Release the hack */
	NumberIndex = 36;			/* Restore the index */
}

/**********************************

	Handle the intermission screen
	
**********************************/

void Intermission (void)
{
	FadeToBlack();
	LevelCompleted();			/* Show the data (Init ParTime) */
	gamestate.globaltime += gamestate.playtime;		/* Add in the playtime */
	gamestate.globaltimetotal += ParTime;	/* Get the par */
	gamestate.globalsecret += gamestate.secretcount;	/* Secrets found */
	gamestate.globaltreasure += gamestate.treasurecount;	/* Treasures found */
	gamestate.globalkill += gamestate.killcount;	/* Number killed */
	gamestate.globalsecrettotal += gamestate.secrettotal;	/* Total secrets */
	gamestate.globaltreasuretotal += gamestate.treasuretotal;	/* Total treasures */
	gamestate.globalkilltotal += gamestate.killtotal;	/* Total kills */
	SetupPlayScreen();		/* Reset the game screen */
}

/**********************************

	Okay, let's face it: they won the game.
	
**********************************/

void VictoryIntermission (void)
{
	FadeToBlack();
	LevelCompleted();
}
