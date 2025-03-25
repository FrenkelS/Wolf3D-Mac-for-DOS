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
 *
 *-----------------------------------------------------------------------------*/

#include <conio.h>
#include <dos.h>
#include <ctype.h>

#include "Burger.h"
#include "Wolfdef.h"		/* All the game equates */


extern LongWord LastTick;


#define KEYBOARDINT 9
#define KBDQUESIZE 32
static Byte keyboardqueue[KBDQUESIZE];
static Word kbdtail, kbdhead;
static Boolean isKeyboardIsrSet = FALSE;

#if defined __DJGPP__ 
static _go32_dpmi_seginfo oldkeyboardisr, newkeyboardisr;
#else
static void __interrupt __far (*oldkeyboardisr)(void);
#endif

static void __interrupt __far I_KeyboardISR(void)	
{
	Byte temp;

	// Get the scan code
	keyboardqueue[kbdhead & (KBDQUESIZE - 1)] = inp(0x60);
	kbdhead++;

	// Tell the XT keyboard controller to clear the key
	outp(0x61, (temp = inp(0x61)) | 0x80);
	outp(0x61, temp);

	// acknowledge the interrupt
	outp(0x20, 0x20);
}


void I_InitKeyboard(void)
{
	replaceInterrupt(oldkeyboardisr, newkeyboardisr, KEYBOARDINT, I_KeyboardISR);
	isKeyboardIsrSet = TRUE;
}

void I_ShutdownKeyboard(void)
{
	if (isKeyboardIsrSet)
	{
		restoreInterrupt(KEYBOARDINT, oldkeyboardisr, newkeyboardisr);
	}
}


static Boolean mousepresent = FALSE;

static Boolean I_ResetMouse(void)
{
	union REGS regs;
	regs.w.ax = 0;
	int86(0x33, &regs, &regs);
	return regs.w.ax != 0;
}


void I_InitMouse(void)
{
	if (M_CheckParm("nomouse")) {
		return;
	}

	mousepresent = I_ResetMouse();
}


void I_ShutdownMouse(void)
{
	if (mousepresent) {
		I_ResetMouse();
	}
}


static Boolean I_ReadMouseButton(void)
{
	union REGS regs;
	regs.w.ax = 3;
	int86(0x33, &regs, &regs);
	return regs.w.bx != 0;
}


static Word I_ReadMouseMotionCounter(void)
{
	union REGS regs;
	regs.w.ax = 11;
	int86(0x33, &regs, &regs);
	return regs.w.cx;
}


/**********************************

	Get a key from the keyboard

**********************************/

#define SC_ESCAPE			0x01
#define SC_1				0x02
#define SC_2				0x03
#define SC_3				0x04
#define SC_4				0x05
#define SC_5				0x06
#define SC_6				0x07
#define SC_MINUS			0x0c
#define SC_PLUS				0x0d
#define SC_TAB				0x0f
#define SC_BRACKET_RIGHT	0x1b
#define SC_ENTER			0x1c
#define SC_CTRL				0x1d
#define SC_LSHIFT			0x2a
#define SC_COMMA			0x33
#define SC_PERIOD			0x34
#define SC_RSHIFT			0x36
#define SC_ALT				0x38
#define SC_SPACE			0x39
#define SC_F5				0x3f
#define SC_F10				0x44
#define SC_UPARROW			0x48
#define SC_DOWNARROW		0x50
#define SC_LEFTARROW		0x4b
#define SC_RIGHTARROW		0x4d

#define SC_Q	0x10
#define SC_P	0x19
#define SC_A	0x1e
#define SC_L	0x26
#define SC_Z	0x2c
#define SC_M	0x32

#define EV_KEYUP	0x80
#define EV_KEYDOWN	0x00

static Word GetAKey(void)
{
	while (kbdtail < kbdhead)
	{
		Word event;
		Byte k = keyboardqueue[kbdtail & (KBDQUESIZE - 1)];
		kbdtail++;

		// extended keyboard shift key bullshit
		if ((k & 0x7f) == SC_LSHIFT || (k & 0x7f) == SC_RSHIFT)
		{
			if (keyboardqueue[(kbdtail - 2) & (KBDQUESIZE - 1)] == 0xe0)
				continue;
			k &= 0x80;
			k |= SC_RSHIFT;
		}

		if (k == 0xe0)
			continue;               // special / pause keys
		if (keyboardqueue[(kbdtail - 2) & (KBDQUESIZE - 1)] == 0xe1)
			continue;                               // pause key bullshit

		if (k == 0xc5 && keyboardqueue[(kbdtail - 2) & (KBDQUESIZE - 1)] == 0x9d)
		{
			//ev.type  = ev_keydown;
			//ev.data1 = KEY_PAUSE;
			//D_PostEvent(&ev);
			continue;
		}

		if (k & 0x80)
			event = EV_KEYUP;
		else
			event = EV_KEYDOWN;

		k &= 0x7f;
		switch (k)
		{
			case SC_1:
			case SC_2:
			case SC_3:
			case SC_4:
			case SC_5:
			case SC_6:
			case SC_MINUS:
			case SC_PLUS:
			case SC_ENTER:
			case SC_SPACE:
			case SC_RSHIFT:
			case SC_UPARROW:
			case SC_DOWNARROW:
			case SC_LEFTARROW:
			case SC_RIGHTARROW:
			case SC_TAB:
			case SC_CTRL:
			case SC_ALT:
			case SC_COMMA:
			case SC_PERIOD:
			case SC_BRACKET_RIGHT:
			case SC_F5:
				return k | event;
			case SC_ESCAPE:
			case SC_F10:
				I_Quit();
			default:
				if (SC_Q <= k && k <= SC_P)
				{
					return "qwertyuiop"[k - SC_Q] | event;
				}
				else if (SC_A <= k && k <= SC_L)
				{
					return "asdfghjkl"[k - SC_A] | event;
				}
				else if (SC_Z <= k && k <= SC_M)
				{
					return "zxcvbnm"[k - SC_Z] | event;
				}
				else
					continue;
		}
	}

	return 0;
}

/**********************************

	Wait for a mouse/keyboard event

**********************************/

Word WaitEvent(void)
{
	Word Temp;
	do {
		Temp = WaitTicksEvent(6000);	/* Wait 10 minutes */
	} while (!Temp);	/* No event? */
	return Temp;		/* Return the event code */
}

/**********************************

	Wait for an event or a timeout

**********************************/

Word WaitTicksEvent(Word Time)
{
	LongWord TickMark;
	LongWord NewMark;
	Word RetVal;

	TickMark = ReadTick();	/* Get the initial time mark */
	for (;;) {
		NewMark = ReadTick();		/* Get the new time mark */
		if (Time) {
			if ((NewMark-TickMark)>=Time) {	/* Time up? */
				RetVal = 0;	/* Return timeout */
				break;
			}
		}
		RetVal = GetAKey();
		if (RetVal && !(RetVal & EV_KEYUP)) {
			break;
		}
	}
	LastTick = NewMark;
	return RetVal;
}

/**********************************

	Flush out the keyboard buffer

**********************************/

void FlushKeys(void)
{
	while (GetAKey()) {}
	joystick1 = 0;
}

/**********************************

	Read from DOS's keyboard/mouse system
	
**********************************/

#define NR_OF_CHEATS 9

static char *CheatPtr[NR_OF_CHEATS] = {		/* Cheat strings */
	"XUSCNIELPPA",	
	"IDDQD",
	"BURGER",
	"WOWZERS",
	"LEDOUX",
	"SEGER",
	"MCCALL",
	"APPLEIIGS",
	"RATE"
};
static Word Cheat;			/* Which cheat is active */
static Word CheatIndex;	/* Index to the cheat string */

static Word setblocks = 10;

void ReadSystemJoystick(void)
{
	Word i;
	Word scancode;
	Word event;
	Word Index;

	/* Switch weapons like in DOOM! */
	
	while ((i = GetAKey())) {			/* Was a key hit? */
		scancode = i;
		event = i & EV_KEYUP;
		if (event == EV_KEYDOWN) {
			i = toupper(i);	/* Force UPPER case */
			if (CheatIndex) {		/* Cheat in progress */
				if (CheatPtr[Cheat][CheatIndex]==(char)i) {		/* Match the current string? */
					++CheatIndex;				/* Next char */
					if (!CheatPtr[Cheat][CheatIndex]) {	/* End of the string? */
						PlaySound(SND_BONUS);		/* I got a bonus! */
						switch (Cheat) {		/* Execute the cheat */
						case 1: // IDDQD
							gamestate.godmode^=TRUE;			/* I am invincible! */
							break;
						case 5: // SEGER
							GiveKey(0);
							GiveKey(1);		/* Award the keys */
							break;
						case 6: // MCCALL
							playstate=EX_WARPED;		/* Force a jump to the next level */
							nextmap = gamestate.mapon+1;	/* Next level */
							if (MapListPtr->MaxMap<=nextmap) {	/* Too high? */
								nextmap = 0;			/* Reset to zero */
							}
							break;
						case 7: // APPLEIIGS
							ShowPush ^= TRUE;
							break;
						case 8: // RATE
							ShowFps ^= TRUE;
							if (!ShowFps)
								IO_DrawTreasure(gamestate.treasure);
							break;
						case 0: // XUSCNIELPPA
						case 4: // LEDOUX
							GiveKey(0);		/* Award the keys */
							GiveKey(1);
							gamestate.godmode = TRUE;			/* I am a god */
						case 2: // BURGER
							gamestate.machinegun = TRUE;
							gamestate.chaingun = TRUE;
							gamestate.flamethrower = TRUE;
							gamestate.missile = TRUE;
							GiveAmmo(gamestate.maxammo);
							GiveGas(99);
							GiveMissile(99);
							break;
						case 3: // WOWZERS
							gamestate.maxammo = 999;
							GiveAmmo(999);
							break;
						}
					}
				} else {
					CheatIndex = 0;
					goto TryFirst;
				}
			} else {
TryFirst:
				Index = 0;				/* Init the scan routine */
				do {
					if (CheatPtr[Index][0] == (char)i) {
						Cheat = Index;		/* This is my current cheat I am scanning */
						CheatIndex = 1;		/* Index to the second char */
						break;				/* Exit */
					}
				} while (++Index<NR_OF_CHEATS);		/* All words scanned? */
			}
		}

		if (event == EV_KEYDOWN) {
			switch (scancode) {		/* Use the SCAN code to make sure I hit the right key! */
			case SC_1 :		/* 1 */
				gamestate.pendingweapon = WP_KNIFE;
				break;
			case SC_2 : 	/* 2 */
				if (gamestate.ammo) {
					gamestate.pendingweapon = WP_PISTOL;
				}
				break;
			case SC_3 :		/* 3 */
				if (gamestate.ammo && gamestate.machinegun) {
					gamestate.pendingweapon = WP_MACHINEGUN;
				}
				break;
			case SC_4 :		/* 4 */
				if (gamestate.ammo && gamestate.chaingun) {
					gamestate.pendingweapon = WP_CHAINGUN;
				}
				break;
			case SC_5 :		/* 5 */
				if (gamestate.gas && gamestate.flamethrower) {
					gamestate.pendingweapon = WP_FLAMETHROWER;
				}
				break;	
			case SC_6 :		/* 6 */
				if (gamestate.missiles && gamestate.missile) {
					gamestate.pendingweapon = WP_MISSILE;
				}
				break;
			case SC_MINUS:
				if (playstate == EX_STILLPLAYING) {
					if (setblocks > 4) {
						setblocks--;
						I_SetViewSize(setblocks);
					}
				}
				break;
			case SC_PLUS:
				if (playstate == EX_STILLPLAYING) {
					if (setblocks < 10) {
						setblocks++;
						I_SetViewSize(setblocks);
					}
				}
				break;
			case SC_F5:
				if (playstate == EX_STILLPLAYING) {
					if (detailshift == 2) {
						detailshift = 0;
						viewwidth <<= 2;
					} else {
						detailshift++;
						viewwidth >>= 1;
					}
					StartupRendering();
				}
				break;
			}
		}

		switch (scancode) {
		case SC_ENTER:
		case SC_SPACE:
		case 'e':
			joystick1 |= JOYPAD_A;
			break;
		case SC_ENTER | EV_KEYUP:
		case SC_SPACE | EV_KEYUP:
		case 'e'      | EV_KEYUP:
			joystick1 &= ~JOYPAD_A;
			break;

		case SC_UPARROW:
		case 'w':
			joystick1 |= JOYPAD_UP;
			break;
		case SC_UPARROW | EV_KEYUP:
		case 'w'        | EV_KEYUP:
			joystick1 &= ~JOYPAD_UP;
			break;

		case SC_DOWNARROW:
		case 's':
			joystick1 |= JOYPAD_DN;
			break;
		case SC_DOWNARROW | EV_KEYUP:
		case 's'          | EV_KEYUP:
			joystick1 &= ~JOYPAD_DN;
			break;

		case SC_LEFTARROW:
			joystick1 |= JOYPAD_LFT;
			break;
		case SC_LEFTARROW | EV_KEYUP:
			joystick1 &= ~JOYPAD_LFT;
			break;

		case SC_RIGHTARROW:
			joystick1 |= JOYPAD_RGT;
			break;
		case SC_RIGHTARROW | EV_KEYUP:
			joystick1 &= ~JOYPAD_RGT;
			break;

		case SC_CTRL:
			joystick1 |= JOYPAD_B;
			break;
		case SC_CTRL | EV_KEYUP:
			joystick1 &= ~JOYPAD_B;
			break;

		case SC_ALT:
			strafe = TRUE;
			break;
		case SC_ALT | EV_KEYUP:
			strafe = FALSE;
			break;

		case SC_COMMA:
		case 'a':
			joystick1 |= JOYPAD_TL;
			break;
		case SC_COMMA | EV_KEYUP:
		case 'a'      | EV_KEYUP:
			joystick1 &= ~JOYPAD_TL;
			break;

		case SC_PERIOD:
		case 'd':
			joystick1 |= JOYPAD_TR;
			break;
		case SC_PERIOD | EV_KEYUP:
		case 'd'       | EV_KEYUP:
			joystick1 &= ~JOYPAD_TR;
			break;

		case SC_LSHIFT:
		case SC_RSHIFT:
			joystick1 |= JOYPAD_X;
			break;
		case SC_LSHIFT | EV_KEYUP:
		case SC_RSHIFT | EV_KEYUP:
			joystick1 &= ~JOYPAD_X;
			break;

		case SC_BRACKET_RIGHT:
			joystick1 |= JOYPAD_SELECT;
			break;
		case SC_BRACKET_RIGHT | EV_KEYUP:
			joystick1 &= ~JOYPAD_SELECT;
			break;

		case SC_TAB:
			joystick1 |= JOYPAD_START; 
			break;
		case SC_TAB | EV_KEYUP:
			joystick1 &= ~JOYPAD_START; 
			break;
		}
	}

	if (mousepresent) {
		mousebutton = I_ReadMouseButton();
		mouseturn   = -1 * I_ReadMouseMotionCounter();
	}
}
