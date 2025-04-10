#define	MyMenuBar	128				/* application's menu bar */

#define	rAboutAlert	128				/* about alert */
#define	rUserAlert	129				/* error user alert */
#define AskSizeWin	130				/* Ask for screen size window */ 
#define NewGameWin	131				/* Choose difficulty */
#define Not607Win	132				/* Not system 6.0.7 or later */
#define NotColorWin	133				/* No color */
#define Not32QDWin	134				/* Not Quickdraw 32 */
#define SlowWarnWin	135				/* Your machine is slow */
#define SpeedTipsWin 136			/* Hints on speed */
#define ShareWareWin 137			/* Pay us $$$ */
#define GoSlowWin 138				/* You are not in 8 bit color mode */
#define EndGameWin 139				/* You won the shareware version */
#define LowMemWin 140				/* You are dangerously low on memory */

/* kSysEnvironsVersion is passed to SysEnvirons to tell it which version of the
   SysEnvRec we understand. */

#define	kSysEnvironsVersion		1

/* kOSEvent is the event number of the suspend/resume and mouse-moved events sent
   by MultiFinder. Once we determine that an event is an osEvent, we look at the
   high byte of the message sent to determine which kind it is. To differentiate
   suspend and resume events we check the resumeMask bit. */

#define	kOSEvent				app4Evt	/* event used by MultiFinder */
#define	kSuspendResumeMessage	1		/* high byte of suspend/resume event message */
#define	kResumeMask				1		/* bit of message field for resume vs. suspend */
#define	kMouseMovedMessage		0xFA	/* high byte of mouse-moved event message */
#define	kNoEvents				0		/* no events mask */

/* The following constants are used to identify menus and their items. The menu IDs
   have an "m" prefix and the item numbers within each menu have an "i" prefix. */

#define	mApple					128		/* Apple menu */
#define	iAbout					1
#define iSpeedHint				2
#define iShareWare				3

#define	mFile	129		/* File menu */
#define	iNew		1
#define	iOpen	2
#define	iClose	4
#define	iSave	5
#define	iSaveAs	6
#define	iQuit	8

#define	mEdit	130		/* Edit menu */
#define	iUndo	1
#define	iCut		3
#define	iCopy	4
#define	iPaste	5
#define	iClear	6

#define	mOptions	131		/* Game menu */
#define	iSound	1
#define	iMusic	2
#define iScreenSize 3
#define iGovenor 4
#define iMouseControl 5
#define iUseQuickDraw 6

/*	1.01 - kTopLeft - This is for positioning the Disk Initialization dialogs. */

#define kDITop	0x0050
#define kDILeft	0x0070
/* kExtremeNeg and kExtremePos are used to set up wide open rectangles and regions. */

#define kExtremeNeg				-32768
#define kExtremePos				32767 - 1 /* required to address an old region bug */

/* these #defines are used to set enable/disable flags of a menu */

#define AllItems	0b1111111111111111111111111111111	/* 31 flags */
#define NoItems	0b0000000000000000000000000000000
#define MenuItem1	0b0000000000000000000000000000001
#define MenuItem2	0b0000000000000000000000000000010
#define MenuItem3	0b0000000000000000000000000000100
#define MenuItem4	0b0000000000000000000000000001000
#define MenuItem5	0b0000000000000000000000000010000
#define MenuItem6	0b0000000000000000000000000100000
#define MenuItem7	0b0000000000000000000000001000000
#define MenuItem8	0b0000000000000000000000010000000
#define MenuItem9	0b0000000000000000000000100000000
#define MenuItem10	0b0000000000000000000001000000000
#define MenuItem11	0b0000000000000000000010000000000
#define MenuItem12	0b0000000000000000000100000000000

/* Burger resources */

#define rIdLogoPic 128	/* Id Logo for 3do version */
#define rMacPlayPic 129	/* Mac play logo */
#define rMacPlayPal 130
#define rIdLogoPal 131
#define rBlackPal 132
#define rTitlePic 133	/* Title screen picture */
#define rTitlePal 134	/* Title screen palette */
#define MySoundList 135	/* List of sound effects to log */
#define MyDarkData 136	/* 256 byte table to darken walls */
#define MyWallList 137	/* All wall shapes */
#define MyBJFace 138	/* BJ's face for automap */
#define rIntermission 139 /* Intermission background */
#define rInterPal 140
#define rInterPics 141	/* BJ's intermission pictures */
#define rFaceShapes 142	/* All the permanent game shapes */
#define rFace512 143	/* All game sprites */
#define rFace640 144
#define rGamePal 145	/* Game Palette */
#define rMapList 146	/* Map info data */
#define rSongList 147	/* Music list data */
#define rGetPsychPic 148
#define rYummyPic 149
#define rYummyPal 150
#define rFineTangent 151	/* High detail tangent table */
#define rFineSine 152		/* High detail sine table */
#define rScaleAtZ 153		/* High detail scale table */
#define rViewAngleToX 154	/* Angle to X coord */
#define rXToViewAngle 155	/* X to angle */
