#include <string.h>

#include "WolfDef.h"


static Boolean refreshStatusBar;


/**********************************

	Draw a space padded list of numbers 
	for the score
	
**********************************/

static LongWord W_pow10[] = {1,10,100,1000,10000,100000,1000000};
Word NumberIndex = 36;		/* First number in the shape list... */

void SetNumber(LongWord number,Word x,Word y,Word digits)
{
    LongWord val;
    Word count;
    Word empty;
    
    empty = 1;			/* No char's drawn yet */
    while (digits) {	/* Any digits left? */
         count = 0;		/* No value yet */
         val = W_pow10[digits-1];	/* Get the power of 10 */
         while (number >= val) {	/* Any value here? */
             count++;		/* +1 to the count */
             number -= val;		/* Remove the value */
         }
         if (empty && !count && digits!=1) {    /* pad on left with blanks rather than 0 */
              DrawShapeNum(x,y,NumberIndex);
         } else {
              empty = 0;		/* I have drawn... */
              DrawShapeNum(x,y,count+NumberIndex);	/* Draw the digit */
         }
         x+=ScaleX(8);
         digits--;		/* Count down */
    }
}

/**********************************

	Read from the Mac's keyboard/mouse system
	
**********************************/

void IO_CheckInput(void)
{
	ReadSystemJoystick();	/* Set the variable "joystick1" */
	
/* check for auto map */

    if (joystick1 & JOYPAD_START) {
         RunAutoMap();		/* Do the auto map */
    }

/*
** get game control flags from joypad
*/
    memset(buttonstate,0,sizeof(buttonstate));	/* Zap the buttonstates */
    if (joystick1 & JOYPAD_UP)
         buttonstate[bt_north] = 1;
    if (joystick1 & JOYPAD_DN)
         buttonstate[bt_south] = 1;

    if (strafe) {
        if (joystick1 & JOYPAD_LFT)
             buttonstate[bt_left] = 1;
        if (joystick1 & JOYPAD_RGT)
             buttonstate[bt_right] = 1;
    }
    else {
        if (joystick1 & JOYPAD_LFT)
             buttonstate[bt_west] = 1;
        if (joystick1 & JOYPAD_RGT)
             buttonstate[bt_east] = 1;
    }

    if (joystick1 & JOYPAD_TL)
         buttonstate[bt_left] = 1;
    if (joystick1 & JOYPAD_TR)
         buttonstate[bt_right] = 1;
    if ((joystick1 & JOYPAD_B) || mousebutton)
         buttonstate[bt_attack] = 1;
    if (joystick1 & (JOYPAD_Y|JOYPAD_X) )
         buttonstate[bt_run] = 1;
    if (joystick1 & JOYPAD_A)
         buttonstate[bt_use] = 1;
    if (joystick1 & JOYPAD_SELECT) {
         buttonstate[bt_select] = 1;
	}
}

/**********************************

	Draw the floor and castle #
	
**********************************/

void IO_DrawFloor(Word floor)
{
    SetNumber(MapListPtr->InfoArray[floor].ScenarioNum,ScaleX(8), ScaleY(MAXVIEWHEIGHT + 16),1);
    SetNumber(MapListPtr->InfoArray[floor].FloorNum,   ScaleX(32),ScaleY(MAXVIEWHEIGHT + 16),1);
    refreshStatusBar = TRUE;
}

/**********************************

	Draw the score
	
**********************************/

void IO_DrawScore(LongWord score)
{
	if (!IntermissionHack) {			/* Don't draw during intermission! */
    	SetNumber(score,ScaleX(56),ScaleY(MAXVIEWHEIGHT + 16),7);
    	refreshStatusBar = TRUE;
    }
}

/**********************************

	Draw the number of live remaining
	
**********************************/

void IO_DrawLives(Word lives)
{
    
   	if (!IntermissionHack) {			/* Don't draw during intermission! */   	
	    --lives;			/* Adjust for zero start value */
    	if (lives > 9) {
    		lives = 9;		/* Failsafe */
		}
		SetNumber(lives,ScaleX(188),ScaleY(MAXVIEWHEIGHT + 16),1);		/* Draw the lives count */
		refreshStatusBar = TRUE;
	}
}

/**********************************

	Draw the health
	
**********************************/

void IO_DrawHealth(Word health)
{
    SetNumber(health,ScaleX(210),ScaleY(MAXVIEWHEIGHT + 16),3);
    refreshStatusBar = TRUE;
}

/**********************************

	Draw the ammo remaining
	
**********************************/

void IO_DrawAmmo(Word ammo)
{
    SetNumber(ammo,ScaleX(268),ScaleY(MAXVIEWHEIGHT + 16),3);
    refreshStatusBar = TRUE;
}

/**********************************

	Draw the treasure score
	
**********************************/

void IO_DrawTreasure(Word treasure)
{
    SetNumber(treasure,ScaleX(128),ScaleY(MAXVIEWHEIGHT + 16),2);
    refreshStatusBar = TRUE;
}

/**********************************

	Draw the keys held
	
**********************************/

void IO_DrawKeys(Word keys)
{
    if (keys&1) {
         DrawShapeNum(ScaleX(310),ScaleY(MAXVIEWHEIGHT + 4),10);
         refreshStatusBar = TRUE;
    }
    if (keys&2) {
    	DrawShapeNum(ScaleX(310),ScaleY(MAXVIEWHEIGHT + 24),11);
    	refreshStatusBar = TRUE;
    }
}

/**********************************

	Draw the gun in the foreground
	
**********************************/

void IO_AttackShape(Word shape)
{
	DrawXMShapeNum((scaledviewwidth / 2) - 32, viewheight - 64, shape + 12);
}

/**********************************

	Draw the BJ's face
	
**********************************/

void IO_DrawFace(Word face)
{
	DrawShapeNum(ScaleX(160),ScaleY(MAXVIEWHEIGHT + 4),face);		/* Draw the face */
	refreshStatusBar = TRUE;
}

/**********************************

	Redraw the main status bar
	
**********************************/

void IO_DrawStatusBar(void)
{
	DrawShapeNum(ScaleX(0),ScaleY(MAXVIEWHEIGHT),46);
}

/**********************************

	Copy the 3-D screen to display memory
	
**********************************/

void IO_DisplayViewBuffer (void)
{
	BlastView();

	if (refreshStatusBar) {
		refreshStatusBar = FALSE;
		BlastStatusBar();
	}

/* if this is the first frame rendered, upload everything and fade in */
    if (firstframe) { 
		FadeTo(rGamePal);
		firstframe = FALSE;
    }
}
